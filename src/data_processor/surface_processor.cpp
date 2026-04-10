#include "surface_processor.h"

#include <cmath>
#include <QDebug>
#include "isobaths_processor.h"

SurfaceProcessor::SurfaceProcessor(DataProcessor* parent) :
    dataProcessor_(parent),
    bottomTrackPtr_(nullptr),
    surfaceMeshPtr_(nullptr),
    origin_(0.0f, 0.0f),
    tileResolution_(defaultTileResolution),
    minZ_(std::numeric_limits<float>::max()),
    maxZ_(std::numeric_limits<float>::lowest()),
    edgeLimit_(20.0f),
    surfaceStepSize_(1.0f),
    tileSidePixelSize_(defaultTileSidePixelSize),
    tileHeightMatrixRatio_(defaultTileHeightMatrixRatio),
    themeId_(0),
    cellPx_(1),
    extraWidth_(0),
    originSet_ (false)
{
}

SurfaceProcessor::~SurfaceProcessor()
{
}

void SurfaceProcessor::clear()
{
    delaunayProc_ = delaunay::Delaunay();
    lastMatParams_ = kmath::MatrixParams();
    pointToTris_.clear();
    cellPoints_.clear();
    cellPointsInTri_.clear();
    origin_ = QPointF(0.0f, 0.0f);
    minZ_ = std::numeric_limits<float>::max();
    maxZ_ = std::numeric_limits<float>::lowest();
    originSet_ = false;
}

void SurfaceProcessor::setBottomTrackPtr(BottomTrack *bottomTrackPtr)
{
    bottomTrackPtr_ = bottomTrackPtr;
}

void SurfaceProcessor::setSurfaceMeshPtr(SurfaceMesh *surfaceMeshPtr)
{
    surfaceMeshPtr_ = surfaceMeshPtr;
}

void SurfaceProcessor::onUpdatedBottomTrackData(const QVector<QPair<char, int>> &indxs)
{
    if (indxs.empty()) {
        return;
    }

    // Delaunay processing
    QVector<QVector3D> bTrData;
    {
        QReadLocker rl(&lock_);
        bTrData = bottomTrackPtr_ ? bottomTrackPtr_->cdata() : QVector<QVector3D>();
    }
    if (bTrData.empty()) {
        return;
    }


    //头一次都是初始化时的数据
    auto& tr = delaunayProc_.getTriangles();
    auto& pt = delaunayProc_.getPoints();
    // qDebug() << "tr.size()... " << tr.size() << "    pt.size()...." << pt.size();

    const auto registerTriangle = [&](int triIdx) {
        const auto& t = tr[triIdx];
        pointToTris_[t.a] << triIdx;
        pointToTris_[t.b] << triIdx;
        pointToTris_[t.c] << triIdx;
    };

    QSet<int> updsTrIndx;
    bool beenManualChanged = false;

    static QSet<SurfaceTile*> changedTiles;
    changedTiles.clear();

    // --- 添加 / 更新中心点（按网格单元划分） ---
    auto processOneCenter = [&](const QVector3D& pnt) -> void {
        if (!originSet_) {
            origin_    = {pnt.x(), pnt.y()};
            originSet_ = true;
        }

        const int ix = qRound((pnt.x() - origin_.x()) / cellPx_);
        const int iy = qRound((pnt.y() - origin_.y()) / cellPx_);
        const QPair<int,int> cid(ix, iy);

        if (auto it = cellPointsInTri_.find(cid); it != cellPointsInTri_.end()) {  // 更新中心点的 Z坐标
            const int pIdx = it.value();
            auto& dp = delaunayProc_.getPointsRef()[pIdx];
            if (!qFuzzyCompare(static_cast<float>(dp.z), pnt.z())) {
                dp.z = pnt.z();
                beenManualChanged = true;
                const auto& relatedTris = pointToTris_.value(pIdx);
                for (int triIdx : relatedTris) {
                    updsTrIndx.insert(triIdx);
                }
            }
        }
        else { // 添加新点
            const auto res = delaunayProc_.addPoint({pnt.x(), pnt.y(), pnt.z()});
            for (int triIdx : res.newTriIdx) {
                registerTriangle(triIdx); // 注册加入新三角形
                updsTrIndx.insert(triIdx);
            }
            cellPointsInTri_[cid] = res.pointIdx;
        }
    };

    for (const auto& itm : indxs) { // 添加至三角剖分
        if (canceled()) {
            return;
        }

        if (itm.second < 0 || itm.second >= bTrData.size()) {
            continue;
        }

        const QVector3D& point = bTrData[itm.second];
        if (!qIsFinite(point.z())) {
            continue;
        }
        if(!isPointInPolygon(point)) {
            continue;
        }
        processOneCenter(point);
    }

    const int triCount = static_cast<int>(tr.size());
    if (!triCount) {
        return;
    }

    float lastMinZ = minZ_;
    float lastMaxZ = maxZ_;
    for (int triIdx : std::as_const(updsTrIndx)) { // 网格内三角形的追踪
        if (canceled()) {
            return;
        }

        if (triIdx < 0 || triIdx >= triCount) {
            continue;
        }

        const auto& t = tr[triIdx];
        const bool inWork = !(t.a < 4 || t.b < 4 || t.c < 4 || t.is_bad || t.longest_edge_dist > edgeLimit_);
        if (!inWork) {
            continue;
        }

        QVector<QVector3D> pts(3, QVector3D(0.0f, 0.0f, 0.0f));
        pts[0] = kmath::fvec(pt[t.a]);
        pts[1] = kmath::fvec(pt[t.b]);
        pts[2] = kmath::fvec(pt[t.c]);

        kmath::MatrixParams actualMatParams(lastMatParams_);
        kmath::MatrixParams newMatrixParams = kmath::getMatrixParams(pts);
        if (newMatrixParams.isValid()) {
            concatenateMatrixParameters(actualMatParams, newMatrixParams);
            if (surfaceMeshPtr_->concatenate(actualMatParams)) {
               lastMatParams_ = actualMatParams;
            }
        }

        writeTriangleToMesh(pts[0], pts[1], pts[2], changedTiles);

        minZ_ = std::min(static_cast<double>(minZ_), std::min({ pt[t.a].z, pt[t.b].z, pt[t.c].z }));
        maxZ_ = std::max(static_cast<double>(maxZ_), std::max({ pt[t.a].z, pt[t.b].z, pt[t.c].z }));
    }

    propagateBorderHeights(changedTiles);
    const int stepPix  = surfaceMeshPtr_->getStepSizeHeightMatrix();
    const int hvSide   = surfaceMeshPtr_->getTileSidePixelSize() / stepPix + 1;
    for (SurfaceTile* t : std::as_const(changedTiles)) {
        smoothTileHeights(t, hvSide);   // 对高度场进行平滑处理，减少噪声
        t->updateHeightIndices();
        t->setIsUpdated(false);
    }

    if (beenManualChanged) {
        float currMin = std::numeric_limits<float>::max();
        float currMax = std::numeric_limits<float>::lowest();
        for (const auto& t : tr) {
            const bool inWork = !(t.a < 4 || t.b < 4 || t.c < 4 || t.is_bad || t.longest_edge_dist > edgeLimit_);
            if (!inWork) {
                continue;
            }
            currMin = std::fmin(currMin, std::min({ pt[t.a].z, pt[t.b].z, pt[t.c].z }));
            currMax = std::fmax(currMax, std::max({ pt[t.a].z, pt[t.b].z, pt[t.c].z }));
        }
        if (currMin != std::numeric_limits<float>::max()) {
            minZ_ = currMin;
        }
        if (currMax != std::numeric_limits<float>::lowest()) {
            maxZ_ = currMax;
        }
    }

    const bool zChanged = !qFuzzyCompare(1.0+minZ_, 1.0+lastMinZ) || !qFuzzyCompare(1.0+maxZ_, 1.0+lastMaxZ);
    if (zChanged) {
        QMetaObject::invokeMethod(dataProcessor_, "postMinZ", Qt::QueuedConnection, Q_ARG(float, minZ_));
        QMetaObject::invokeMethod(dataProcessor_, "postMaxZ", Qt::QueuedConnection, Q_ARG(float, maxZ_));
    }

    TileMap res;
    res.reserve(changedTiles.size());
    for (auto it = changedTiles.cbegin(); it != changedTiles.cend(); ++it) {
        res.insert((*it)->getUuid(), (*(*it)));
    }

    QMetaObject::invokeMethod(dataProcessor_, "postSurfaceTiles", Qt::QueuedConnection, Q_ARG(TileMap, res), Q_ARG(bool, false));
}


void SurfaceProcessor::setTileResolution(float tileResolution)
{
    tileResolution_ = tileResolution;
}

void SurfaceProcessor::setEdgeLimit(float val)
{
    edgeLimit_ = val;

    refreshAfterEdgeLimitChange();
}

void SurfaceProcessor::rebuildColorIntervals()
{
    int levelCount = static_cast<int>(((maxZ_ - minZ_) / surfaceStepSize_) + 1);
    if (levelCount <= 0) {
        return;
    }

    colorIntervals_.clear();
    QVector<QVector3D> palette = generateExpandedPalette(levelCount);
    std::reverse(palette.begin(), palette.end());
    colorIntervals_.reserve(levelCount);

    for (int i = 0; i < levelCount; ++i) {
        colorIntervals_.append({ minZ_ + i * surfaceStepSize_, palette[i] });
    }

    QMetaObject::invokeMethod(dataProcessor_, "postSurfaceColorIntervalsSize", Qt::QueuedConnection, Q_ARG(int, static_cast<int>(colorIntervals_.size())));
    QMetaObject::invokeMethod(dataProcessor_, "postSurfaceStepSize", Qt::QueuedConnection, Q_ARG(float, surfaceStepSize_));

    updateTexture();
}

void SurfaceProcessor::setSurfaceStepSize(float val)
{
    surfaceStepSize_ = val;
}

void SurfaceProcessor::setThemeId(int val)
{
    themeId_ = val;
}

void SurfaceProcessor::setExtraWidth(int val)
{
    extraWidth_ = val;
}

float SurfaceProcessor::getEdgeLimit() const
{
    return edgeLimit_;
}

float SurfaceProcessor::getSurfaceStepSize() const
{
    return surfaceStepSize_;
}

int SurfaceProcessor::getThemeId() const
{
    return themeId_;
}

int SurfaceProcessor::getExtraWidth() const
{
    return extraWidth_;
}

void SurfaceProcessor::writeTriangleToMesh(const QVector3D &A, const QVector3D &B, const QVector3D &C, QSet<SurfaceTile*> &updatedTiles)
{
    if (!surfaceMeshPtr_ || !surfaceMeshPtr_->getIsInited()) {
        return;
    }

    const int stepPix     = surfaceMeshPtr_->getStepSizeHeightMatrix();
    const int tileSidePix = surfaceMeshPtr_->getTileSidePixelSize();
    const int hvSide      = tileSidePix / stepPix + 1;
    const int tilesY      = surfaceMeshPtr_->getNumHeightTiles();
    const int meshW       = surfaceMeshPtr_->getPixelWidth();
    const int meshH       = surfaceMeshPtr_->getPixelHeight();

    QVector3D Ap = surfaceMeshPtr_->convertPhToPixCoords(A);
    QVector3D Bp = surfaceMeshPtr_->convertPhToPixCoords(B);
    QVector3D Cp = surfaceMeshPtr_->convertPhToPixCoords(C);

    int minPx = std::floor(std::min({ Ap.x(), Bp.x(), Cp.x() })) - stepPix; // описывающий прямоугольник с запасом stepPix (вершина на границе)
    int maxPx = std::ceil (std::max({ Ap.x(), Bp.x(), Cp.x() })) + stepPix;
    int minPy = std::floor(std::min({ Ap.y(), Bp.y(), Cp.y() })) - stepPix;
    int maxPy = std::ceil (std::max({ Ap.y(), Bp.y(), Cp.y() })) + stepPix;
    minPx = std::clamp((minPx / stepPix) * stepPix, 0, meshW - 1); // сдвигаем описывающий прямоугольник на сетку (кратность stepPix)
    maxPx = std::clamp((maxPx / stepPix) * stepPix, 0, meshW - 1);
    minPy = std::clamp((minPy / stepPix) * stepPix, 0, meshH - 1);
    maxPy = std::clamp((maxPy / stepPix) * stepPix, 0, meshH - 1);

    const float denom = kmath::twiceArea(Ap, Bp, Cp);
    if (qFuzzyIsNull(denom)) { // вырожденный треугольник
        return;
    }

    for (int py = minPy; py <= maxPy; py += stepPix) {
        for (int px = minPx; px <= maxPx; px += stepPix) {
            QVector3D Pp(px, py, 0.f);

            float w0 = kmath::twiceArea(Bp, Cp, Pp) / denom;
            float w1 = kmath::twiceArea(Cp, Ap, Pp) / denom;
            float w2 = 1.f - w0 - w1;

            if (w0 < -kmath::fltEps || w1 < -kmath::fltEps || w2 < -kmath::fltEps) { // вне треугольника
                continue;
            }

            float interpZ = w0 * A.z() + w1 * B.z() + w2 * C.z();

            int tileX = px / tileSidePix;
            int tileY = (tilesY - 1) - py / tileSidePix;
            int locX  = px % tileSidePix;
            int locY  = py % tileSidePix;
            int hvIdx = (locY / stepPix) * hvSide + (locX / stepPix);

            SurfaceTile* tile = surfaceMeshPtr_->getTileMatrixRef()[tileY][tileX];
            if (!tile->getIsInited()) {
                tile->init(tileSidePix, tileHeightMatrixRatio_, tileResolution_);
            }

            tile->getHeightVerticesRef()[hvIdx][2]  = interpZ;
            tile->getHeightMarkVerticesRef()[hvIdx] = HeightType::kTriangulation;
            tile->setIsUpdated(true);
            updatedTiles.insert(tile);
        }
    }
}

QVector<QVector3D> SurfaceProcessor::generateExpandedPalette(int totalColors) const
{
    const auto &palette = colorPalette(themeId_);
    const int paletteSize = palette.size();

    QVector<QVector3D> retVal;

    if (totalColors <= 1 || paletteSize == 0) {
        retVal.append(paletteSize > 0 ? palette.first() : QVector3D(1.0f, 1.0f, 1.0f)); // fallback: white
        return retVal;
    }

    retVal.reserve(totalColors);

    for (int i = 0; i < totalColors; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(totalColors - 1);
        float ft = t * (paletteSize - 1);
        int i0 = static_cast<int>(ft);
        int i1 = std::min(i0 + 1, paletteSize - 1);
        float l = ft - static_cast<float>(i0);
        retVal.append((1.f - l) * palette[i0] + l * palette[i1]);
    }

    return retVal;
}

void SurfaceProcessor::updateTexture() const
{
    int paletteSize = colorIntervals_.size();
    if (paletteSize == 0) {
        return;
    }

    std::vector<uint8_t> textureTask;
    textureTask.resize(paletteSize * 4);
    for (int i = 0; i < paletteSize; ++i) {
        const QVector3D &c = colorIntervals_[i].color;
        textureTask[i * 4 + 0] = static_cast<uint8_t>(qBound(0.f, c.x() * 255.f, 255.f));
        textureTask[i * 4 + 1] = static_cast<uint8_t>(qBound(0.f, c.y() * 255.f, 255.f));
        textureTask[i * 4 + 2] = static_cast<uint8_t>(qBound(0.f, c.z() * 255.f, 255.f));
        textureTask[i * 4 + 3] = 255;
    }

    QMetaObject::invokeMethod(dataProcessor_, "postSurfaceColorTable", Qt::QueuedConnection, Q_ARG(std::vector<uint8_t>, textureTask));
}

void SurfaceProcessor::propagateBorderHeights(QSet<SurfaceTile*>& changedTiles)
{
    if (!surfaceMeshPtr_ || !surfaceMeshPtr_->getIsInited()) {
        return;
    }

    const int stepPix  = surfaceMeshPtr_->getStepSizeHeightMatrix();
    const int hvSide   = surfaceMeshPtr_->getTileSidePixelSize() / stepPix + 1;
    const int tilesY   = surfaceMeshPtr_->getNumHeightTiles();
    const int tilesX   = surfaceMeshPtr_->getNumWidthTiles();

    auto& matrix = surfaceMeshPtr_->getTileMatrixRef();

    auto copyRow = [&](SurfaceTile* src, SurfaceTile* dst, int rowFrom, int rowTo) {
        auto& vSrc = src->getHeightVerticesRef();
        auto& vDst = dst->getHeightVerticesRef();
        auto& mDst = dst->getHeightMarkVerticesRef();
        for (int k = 0; k < hvSide; ++k) {
            int iFrom = rowFrom * hvSide + k;
            int iTo   = rowTo   * hvSide + k;
            if (!qFuzzyIsNull(vSrc[iFrom].z())) {
                float srcZ = vSrc[iFrom].z();
                float dstZ = vDst[iTo].z();
                if(!qFuzzyIsNull(dstZ)) {
                    vDst[iTo][2] = 0.7f * srcZ + 0.3f * dstZ;
                } else {
                    vDst[iTo][2] = srcZ;
                }
                mDst[iTo] = HeightType::kExrtapolation;
            }
        }
    };

    auto copyCol = [&](SurfaceTile* src, SurfaceTile* dst, int colFrom, int colTo) {
        auto& vSrc = src->getHeightVerticesRef();
        auto& vDst = dst->getHeightVerticesRef();
        auto& mDst = dst->getHeightMarkVerticesRef();
        for (int k = 0; k < hvSide; ++k) {
            int iFrom = k * hvSide + colFrom;
            int iTo   = k * hvSide + colTo;
            if (!qFuzzyIsNull(vSrc[iFrom].z())) {
                float srcZ = vSrc[iFrom].z();
                float dstZ = vDst[iTo].z();
                if(!qFuzzyIsNull(dstZ)) {
                    vDst[iTo][2] = 0.7f * srcZ + 0.3f * dstZ;
                } else {
                    vDst[iTo][2] = srcZ;
                }
                mDst[iTo] = HeightType::kExrtapolation;
            }
        }
    };

    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            SurfaceTile* t = matrix[ty][tx];
            if (!t->getIsUpdated()) {
                continue;
            }

            if (ty + 1 < tilesY) { // 向上，将第 0 行移动到顶部瓦片的最后一行
                SurfaceTile* top = matrix[ty + 1][tx];
                if (!top->getIsInited()) {
                    top->init(tileSidePixelSize_, tileHeightMatrixRatio_, tileResolution_);
                }
                copyRow(t, top, 0, hvSide - 1);
                top->setIsUpdated(true);
                changedTiles.insert(top);
            }

            if (tx > 0) { // 向左，将第 0 列移到左侧瓦片的最右列
                SurfaceTile* left = matrix[ty][tx - 1];
                if (!left->getIsInited()) {
                    left->init(tileSidePixelSize_, tileHeightMatrixRatio_, tileResolution_);
                }
                copyCol(t, left, 0, hvSide - 1);
                left->setIsUpdated(true);
                changedTiles.insert(left);
            }

            if (ty + 1 < tilesY && tx > 0) { // 对角节点
                SurfaceTile* topLeft = matrix[ty + 1][tx - 1];
                if (!topLeft->getIsInited()) {
                    topLeft->init(tileSidePixelSize_, tileHeightMatrixRatio_, tileResolution_);
                }
                auto& vSrc = t->getHeightVerticesRef();
                auto& vDst = topLeft->getHeightVerticesRef();
                auto& mDst = topLeft->getHeightMarkVerticesRef();

                const int srcTL = 0;
                const int dstBR = hvSide * hvSide - 1;
                if (!qFuzzyIsNull(vSrc[srcTL].z())) {
                    vDst[dstBR][2] = vSrc[srcTL][2];
                    mDst[dstBR]    = HeightType::kExrtapolation;
                    topLeft->setIsUpdated(true);
                    changedTiles.insert(topLeft);
                }
            }
        }
    }

}


void SurfaceProcessor::refreshAfterEdgeLimitChange()
{
    if (!surfaceMeshPtr_ || !surfaceMeshPtr_->getIsInited()) {
        return;
    }

    surfaceMeshPtr_->clearHeightData(HeightType::kTriangulation);

    const auto& tr = delaunayProc_.getTriangles();
    const auto& pt = delaunayProc_.getPoints();
    QSet<SurfaceTile*> changedTiles;

    for (std::size_t i = 0; i < tr.size(); ++i) {
        const auto& t = tr[i];
        if (t.is_bad || t.a < 4 || t.b < 4 || t.c < 4) {
            continue;
        }
        if (t.longest_edge_dist > edgeLimit_) {
            continue;
        }

        QVector3D A = kmath::fvec(pt[t.a]);
        QVector3D B = kmath::fvec(pt[t.b]);
        QVector3D C = kmath::fvec(pt[t.c]);
        writeTriangleToMesh(A,B,C, changedTiles);
    }

    propagateBorderHeights(changedTiles);

    for (SurfaceTile* t : std::as_const(changedTiles)) {
        t->updateHeightIndices();
    }

    float lastMinZ = minZ_;
    float lastMaxZ = maxZ_;

    float currMin = std::numeric_limits<float>::max();
    float currMax = std::numeric_limits<float>::lowest();
    for (auto t : tr) {
        bool inWork = !(t.a < 4 || t.b < 4 || t.c < 4 || t.is_bad || t.longest_edge_dist > edgeLimit_);
        if (!inWork) {
            continue;
        }
        currMin = std::fmin(currMin, std::min({ pt[t.a].z, pt[t.b].z , pt[t.c].z }));
        currMax = std::fmax(currMax, std::max({ pt[t.a].z, pt[t.b].z , pt[t.c].z }));
    }
    if (currMin != std::numeric_limits<float>::max()) {
        minZ_ = currMin;
    }
    if (currMax != std::numeric_limits<float>::lowest()) {
        maxZ_ = currMax;
    }

    const bool zChanged = !qFuzzyCompare(1.0 + minZ_, 1.0 + lastMinZ) || !qFuzzyCompare(1.0 + maxZ_, 1.0 + lastMaxZ);
    if (zChanged) {
        QMetaObject::invokeMethod(dataProcessor_, "postMinZ", Qt::QueuedConnection, Q_ARG(float, minZ_));
        QMetaObject::invokeMethod(dataProcessor_, "postMaxZ", Qt::QueuedConnection, Q_ARG(float, maxZ_));
    }

    // to SurfaceView 所有瓦片
    const auto& tilesRef = surfaceMeshPtr_->getTilesCRef();
    TileMap res;
    res.reserve(tilesRef.size());
    for (auto it = tilesRef.cbegin(); it != tilesRef.cend(); ++it) {
        res.insert((*it)->getUuid(), (*(*it)));
    }

    QMetaObject::invokeMethod(dataProcessor_, "postSurfaceTiles", Qt::QueuedConnection, Q_ARG(TileMap, res), Q_ARG(bool, false));
}

bool SurfaceProcessor::canceled() const noexcept
{
    return dataProcessor_ && dataProcessor_->isCancelRequested();
}

bool SurfaceProcessor::isPointInPolygon(const QVector3D& point) const
{
    // 使用射线法判断点是否在多边形内
    const QVector<North_East_Down>& polygonOutlineNed = dataProcessor_->datasetPtr_->getPolygonOutlineNED();
    if(polygonOutlineNed.isEmpty()) {
        return true;
    }
    bool inside = false;
    int n = polygonOutlineNed.size();
    for(int i = 0, j = (n-1); i < n; j = i++) {
        QVector3D vi = QVector3D(polygonOutlineNed.at(i).n, polygonOutlineNed.at(i).e, 0.0f);
        QVector3D vj = QVector3D(polygonOutlineNed.at(j).n, polygonOutlineNed.at(j).e, 0.0f);
        if (  ( (vi.y() > point.y()) != (vj.y() > point.y()) ) &&
            (point.x() < (vj.x()-vi.x()) * (point.y()-vi.y()) / (vj.y()-vi.y())+vi.x()) ) {
            inside = !inside;
        }
    }
    return inside;
}


void SurfaceProcessor::smoothTileHeights(SurfaceTile* tile, int hvSide)
{
    if (!tile || !tile->getIsInited()) {
        return;
    }

    auto& vertices = tile->getHeightVerticesRef();
    auto& marks = tile->getHeightMarkVerticesRef();

    // 创建平滑后的高度副本
    QVector<float> smoothedZ(vertices.size());
    for (int i = 0; i < vertices.size(); ++i) {
        smoothedZ[i] = vertices[i].z();
    }

    // 3x3高斯核平滑，但只对非边界点进行处理
    const float kernel[3][3] = {
        {0.0625f, 0.125f, 0.0625f},
        {0.125f,  0.25f,  0.125f},
        {0.0625f, 0.125f, 0.0625f}
    };

    for (int y = 1; y < hvSide - 1; ++y) {
        for (int x = 1; x < hvSide - 1; ++x) {
            int idx = y * hvSide + x;

            // 只平滑有效的三角剖分点
            if (marks[idx] != HeightType::kTriangulation) {
                continue;
            }

            float newZ = 0.0f;
            float weightSum = 0.0f;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int nIdx = (y + ky) * hvSide + (x + kx);
                    float z = vertices[nIdx].z();

                    // 只考虑有效的邻居点
                    if (!qFuzzyIsNull(z)) {
                        float w = kernel[ky + 1][kx + 1];
                        newZ += w * z;
                        weightSum += w;
                    }
                }
            }

            if (weightSum > 0.0f) {
                smoothedZ[idx] = newZ / weightSum;
            }
        }
    }

    // 应用平滑后的高度
    for (int i = 0; i < vertices.size(); ++i) {
        if (!qFuzzyIsNull(vertices[i].z())) {
            vertices[i].setZ(smoothedZ[i]);
        }
    }
}
