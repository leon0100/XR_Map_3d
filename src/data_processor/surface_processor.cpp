#include "surface_processor.h"

#include <cmath>
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
    minX_ = std::numeric_limits<float>::max();
    maxX_ = std::numeric_limits<float>::lowest();
    minY_ = std::numeric_limits<float>::max();
    maxY_ = std::numeric_limits<float>::lowest();
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


    // 添加边界框的四个顶点（顺时针方向）
    QVector3D lastPt = bTrData.last();
    minX_ = std::min(minX_, lastPt.x());
    maxX_ = std::max(maxX_, lastPt.x());
    minY_ = std::min(minY_, lastPt.y());
    maxY_ = std::max(maxY_, lastPt.y());
    QVector<QVector3D> boundary;
    boundary.append(QVector3D(minY_, minX_, 0));
    boundary.append(QVector3D(maxY_, minX_, 0));
    boundary.append(QVector3D(maxY_, maxX_, 0));
    boundary.append(QVector3D(minY_, maxX_, 0));
    boundary.append(QVector3D(minY_, minX_, 0));


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

    smoothDuringTriangulation();
    extractBoundaryVertices();  // 新增

    const int triCount = static_cast<int>(tr.size());
    if (!triCount) {
        return;
    }

    float lastMinZ = minZ_;
    float lastMaxZ = maxZ_;
    for (int triIdx : std::as_const(updsTrIndx)) {  // 网格内三角形的追踪
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
    for (SurfaceTile* tile : std::as_const(changedTiles)) {
        // smoothTileHeights(tile);  // 对高度场进行平滑处理，减少噪声
        clipHeightFieldToPolygon();
        tile->updateHeightIndices();
        tile->setIsUpdated(false);
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
    // QVector<QVector3D> boundary = extractAlphaShapeBoundary();
    // dataProcessor_->setAutoBounadry(boundary); //获取自动边界
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

QVector<IsobathUtils::ColorInterval> SurfaceProcessor::getColorIntervals()
{
    return colorIntervals_;
}

void SurfaceProcessor::writeTriangleToMesh(const QVector3D &A, const QVector3D &B, const QVector3D &C, QSet<SurfaceTile*> &updatedTiles)
{
    if (!surfaceMeshPtr_ || !surfaceMeshPtr_->getIsInited()) {
        return;
    }

    const int stepPix     = surfaceMeshPtr_->getStepSizeHeightMatrix();
    const int tileSidePix = tileSidePixelSize_;
    const int hvSide      = tileHeightMatrixRatio_ + 1;
    const int tilesY      = surfaceMeshPtr_->getNumHeightTiles();
    const int meshW       = surfaceMeshPtr_->getPixelWidth();
    const int meshH       = surfaceMeshPtr_->getPixelHeight();

    QVector3D Ap = surfaceMeshPtr_->convertPhToPixCoords(A);
    QVector3D Bp = surfaceMeshPtr_->convertPhToPixCoords(B);
    QVector3D Cp = surfaceMeshPtr_->convertPhToPixCoords(C);

    int minPx = std::floor(std::min({ Ap.x(), Bp.x(), Cp.x() })) - stepPix; // 描述带有 stepPix 余量的矩形（顶点位于边界上）
    int maxPx = std::ceil (std::max({ Ap.x(), Bp.x(), Cp.x() })) + stepPix;
    int minPy = std::floor(std::min({ Ap.y(), Bp.y(), Cp.y() })) - stepPix;
    int maxPy = std::ceil (std::max({ Ap.y(), Bp.y(), Cp.y() })) + stepPix;
    minPx = std::clamp((minPx / stepPix) * stepPix, 0, meshW - 1); // 将外接矩形按网格进行偏移（步长 stepPix 整数倍对齐）
    maxPx = std::clamp((maxPx / stepPix) * stepPix, 0, meshW - 1);
    minPy = std::clamp((minPy / stepPix) * stepPix, 0, meshH - 1);
    maxPy = std::clamp((maxPy / stepPix) * stepPix, 0, meshH - 1);

    const float denom = kmath::twiceArea(Ap, Bp, Cp);
    if (qFuzzyIsNull(denom)) { // 退化三角形
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

    const int hvSide   = tileHeightMatrixRatio_ + 1;
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
    if (!dataProcessor_ || !dataProcessor_->datasetPtr_) {
        return true;
    }

    // 使用射线法判断点是否在多边形内
    const QVector<North_East_Down>& polygonOutlineNed = dataProcessor_->datasetPtr_->getPolygonOutlineNED();
    if(polygonOutlineNed.isEmpty()) {
        return true;
    }
    bool inside = false;
    int n = polygonOutlineNed.size();
    for(int i = 0, j = (n-1); i < n; j = i++) {
        const float xi = polygonOutlineNed.at(i).n;
        const float yi = polygonOutlineNed.at(i).e;
        const float xj = polygonOutlineNed.at(j).n;
        const float yj = polygonOutlineNed.at(j).e;
        const bool intersect = ((yi > point.y()) != (yj > point.y())) &&
                            (point.x() < (xj - xi) * (point.y() - yi) / (yj - yi + 1e-12f) + xi);
        if (intersect) {
            inside = !inside;
        }
    }

    return inside;
}


// void SurfaceProcessor::smoothTileHeights(SurfaceTile* tile)
// {
//     int hvSide = tileHeightMatrixRatio_ + 1;
//     if (!tile || !tile->getIsInited()) {
//         return;
//     }

//     auto& vertices = tile->getHeightVerticesRef();
//     auto& marks    = tile->getHeightMarkVerticesRef();

//     // 创建平滑后的高度副本
//     QVector<float> smoothedZ(vertices.size());
//     for (int i = 0; i < vertices.size(); ++i) {
//         smoothedZ[i] = vertices[i].z();
//     }

//     // 3x3高斯核平滑，但只对非边界点进行处理
//     const float kernel[3][3] = {
//         {0.0625f, 0.125f, 0.0625f},
//         {0.125f,  0.25f,  0.125f },
//         {0.0625f, 0.125f, 0.0625f}
//     };

//     for (int y = 1; y < hvSide-1; ++y) {
//         for (int x = 1; x < hvSide-1; ++x) {
//             int idx = y * hvSide + x;

//             // 只平滑有效的三角剖分点
//             if (marks[idx] != HeightType::kTriangulation) {
//                 continue;
//             }

//             float newZ = 0.0f;
//             float weightSum = 0.0f;

//             for (int ky = -1; ky <= 1; ++ky) {
//                 for (int kx = -1; kx <= 1; ++kx) {
//                     int nIdx = (y + ky) * hvSide + (x + kx);
//                     float z = vertices[nIdx].z();

//                     // 只考虑有效的邻居点
//                     if (!qFuzzyIsNull(z)) {
//                         float w = kernel[ky + 1][kx + 1];
//                         newZ += w * z;
//                         weightSum += w;
//                     }
//                 }
//             }

//             if (weightSum > 0.0f) {
//                 smoothedZ[idx] = newZ / weightSum;
//             }
//         }
//     }

//     // 应用平滑后的高度
//     for (int i = 0; i < vertices.size(); ++i) {
//         if (!qFuzzyIsNull(vertices[i].z())) {
//             vertices[i].setZ(smoothedZ[i]);
//         }
//     }
// }




void SurfaceProcessor::smoothDuringTriangulation()
{
    auto& triangles = delaunayProc_.getTriangles();
    auto& points = delaunayProc_.getPointsRef();

    if (triangles.empty()) {
        qDebug() << "smoothDuringTriangulation: triangles empty";
        return;
    }

    qDebug() << QString("smoothDuringTriangulation: processing, triangles: %1, points: %2")
                    .arg(triangles.size()).arg(points.size());

    std::vector<double> originalZ(points.size());
    for (size_t i = 0; i < points.size(); ++i) {
        originalZ[i] = points[i].z;
    }

    std::vector<int> boundaryVertices = findBoundaryVertices(triangles);
    qDebug() << QString("Found %1 boundary vertices").arg(boundaryVertices.size());

    smoothBoundaryVerticesEx(triangles, points, originalZ, boundaryVertices);
    smoothInnerVerticesEx(triangles, points, originalZ, boundaryVertices);

    qDebug() << "smoothDuringTriangulation: done";
}
std::vector<int> SurfaceProcessor::findBoundaryVertices(std::vector<delaunay::Triangle>& triangles)
{
    std::map<std::pair<int, int>, int> edgeCount;
    std::unordered_set<int> boundaryVerticesSet;

    // 统计每条边出现的次数
    for (auto& tri : triangles) {
        if (tri.is_bad || tri.a < 4 || tri.b < 4 || tri.c < 4) {
            continue;
        }

        // 确保顶点索引有序
        int a = std::min(tri.a, tri.b);
        int b = std::max(tri.a, tri.b);
        edgeCount[{a, b}]++;

        a = std::min(tri.b, tri.c);
        b = std::max(tri.b, tri.c);
        edgeCount[{a, b}]++;

        a = std::min(tri.c, tri.a);
        b = std::max(tri.c, tri.a);
        edgeCount[{a, b}]++;
    }

    // 找出只出现一次的边（边界边）上的顶点
    for (const auto& pair : edgeCount) {
        if (pair.second == 1) {
            boundaryVerticesSet.insert(pair.first.first);
            boundaryVerticesSet.insert(pair.first.second);
        }
    }

    return std::vector<int>(boundaryVerticesSet.begin(), boundaryVerticesSet.end());
}


void SurfaceProcessor::smoothBoundaryVerticesEx(
    std::vector<delaunay::Triangle>& triangles,
    std::vector<delaunay::Point>& points,
    const std::vector<double>& originalZ,
    const std::vector<int>& boundaryVertices)
{
    // 找出边界边并连接成链
    std::map<std::pair<int, int>, int> edgeCount;
    for (auto& tri : triangles) {
        if (tri.is_bad || tri.a < 4 || tri.b < 4 || tri.c < 4) continue;
        int a = std::min(tri.a, tri.b);
        int b = std::max(tri.a, tri.b);
        edgeCount[{a, b}]++;
        a = std::min(tri.b, tri.c);
        b = std::max(tri.b, tri.c);
        edgeCount[{a, b}]++;
        a = std::min(tri.c, tri.a);
        b = std::max(tri.c, tri.a);
        edgeCount[{a, b}]++;
    }

    std::vector<std::pair<int, int>> boundaryEdges;
    for (const auto& pair : edgeCount) {
        if (pair.second == 1) {
            boundaryEdges.push_back(pair.first);
        }
    }

    // 构建边界链
    std::vector<std::vector<int>> chains = buildBoundaryChains(boundaryEdges, points);

    // 对每条链进行曲线拟合
    for (auto& chain : chains) {
        if (chain.size() < 3) continue;

        // 提取坐标
        std::vector<double> xs, ys, zs;
        for (int idx : chain) {
            xs.push_back(points[idx].x);
            ys.push_back(points[idx].y);
            zs.push_back(originalZ[idx]);
        }

        // ========== 使用简单的线性拟合替代 Eigen ==========
        // 拟合平面: z = ax + by + c

        int n = xs.size();
        double sumX = 0, sumY = 0, sumZ = 0;
        double sumXX = 0, sumYY = 0, sumXY = 0;
        double sumXZ = 0, sumYZ = 0;

        for (int i = 0; i < n; i++) {
            sumX += xs[i];
            sumY += ys[i];
            sumZ += zs[i];
            sumXX += xs[i] * xs[i];
            sumYY += ys[i] * ys[i];
            sumXY += xs[i] * ys[i];
            sumXZ += xs[i] * zs[i];
            sumYZ += ys[i] * zs[i];
        }

        // 解线性方程组
        double denom = n * (sumXX + sumYY) - (sumX * sumX + sumY * sumY);

        if (std::abs(denom) < 1e-10) continue;  // 避免除零

        // 计算系数
        double a = (n * sumXZ - sumX * sumZ) / denom;
        double b = (n * sumYZ - sumY * sumZ) / denom;
        double c = (sumZ - a * sumX - b * sumY) / n;

        // 使用拟合结果更新边界顶点高度
        for (int i = 0; i < chain.size(); i++) {
            int idx = chain[i];
            double x = xs[i];
            double y = ys[i];
            double fittedZ = a * x + b * y + c;

            // 平滑过渡到拟合值
            points[idx].z = originalZ[idx] * 0.4 + fittedZ * 0.6;
        }
    }
}

std::vector<std::vector<int>> SurfaceProcessor::buildBoundaryChains(
    const std::vector<std::pair<int, int>>& boundaryEdges,
    const std::vector<delaunay::Point>& points)
{
    std::vector<std::vector<int>> chains;

    if (boundaryEdges.empty()) {
        return chains;
    }

    // 构建邻接表
    std::unordered_map<int, std::vector<int>> adjacency;
    for (const auto& edge : boundaryEdges) {
        adjacency[edge.first].push_back(edge.second);
        adjacency[edge.second].push_back(edge.first);
    }

    // 找出链的起点（度数为1的顶点）
    std::vector<int> chainStarts;
    for (const auto& pair : adjacency) {
        if (pair.second.size() == 1) {
            chainStarts.push_back(pair.first);
        }
    }

    // 如果没有度数为1的顶点（环形边界），使用第一个顶点作为起点
    if (chainStarts.empty() && !adjacency.empty()) {
        chainStarts.push_back(adjacency.begin()->first);
    }

    // 标记已访问的顶点
    std::unordered_set<int> visited;

    // 构建每条链
    for (int start : chainStarts) {
        if (visited.count(start)) continue;

        std::vector<int> chain;
        int current = start;
        int prev = -1;

        while (current != -1 && !visited.count(current)) {
            visited.insert(current);
            chain.push_back(current);

            // 找到下一个顶点
            int next = -1;
            for (int neighbor : adjacency[current]) {
                if (neighbor != prev) {
                    next = neighbor;
                    break;
                }
            }

            prev = current;
            current = next;
        }

        // 如果形成环，闭合链
        if (!chain.empty() && chain.size() > 2 && adjacency[chain.back()].size() > 1) {
            // 检查是否形成环
            for (int neighbor : adjacency[chain.back()]) {
                if (neighbor == chain.front()) {
                    // 是环，但不需要闭合
                    break;
                }
            }
        }

        chains.push_back(chain);
    }

    qDebug() << QString("构建了 %1 条边界链").arg(chains.size());
    return chains;
}


void SurfaceProcessor::smoothInnerVerticesEx(
    std::vector<delaunay::Triangle>& triangles,
    std::vector<delaunay::Point>& points,
    const std::vector<double>& originalZ,
    const std::vector<int>& boundaryVertices)
{
    std::unordered_set<int> boundarySet(boundaryVertices.begin(), boundaryVertices.end());

    std::unordered_map<int, std::vector<int>> vertexToTriangles;
    for (size_t i = 0; i < triangles.size(); ++i) {
        auto& tri = triangles[i];
        if (tri.is_bad || tri.a < 4 || tri.b < 4 || tri.c < 4) {
            continue;
        }
        vertexToTriangles[tri.a].push_back(i);
        vertexToTriangles[tri.b].push_back(i);
        vertexToTriangles[tri.c].push_back(i);
    }

    for (const auto& pair : vertexToTriangles) {
        int vertexIdx = pair.first;

        // 跳过边界顶点（已处理）
        if (boundarySet.count(vertexIdx)) {
            continue;
        }

        const auto& triIndices = pair.second;
        std::set<int> neighbors;
        for (int triIdx : triIndices) {
            auto& tri = triangles[triIdx];
            neighbors.insert(tri.a);
            neighbors.insert(tri.b);
            neighbors.insert(tri.c);
        }
        neighbors.erase(vertexIdx);

        double sumZ = 0.0;
        double weightSum = 0.0;
        for (int nIdx : neighbors) {
            if (nIdx < 0 || nIdx >= static_cast<int>(originalZ.size())) {
                continue;
            }
            double dist = calculateDistance(points[vertexIdx], points[nIdx]);
            double weight = (dist < 0.001) ? 1.0 : (1.0 / dist);
            sumZ += originalZ[nIdx] * weight;
            weightSum += weight;
        }

        if (weightSum > 0.0) {
            double smoothedZ = sumZ / weightSum;
            double delta = std::abs(smoothedZ - originalZ[vertexIdx]);

            if (delta < 2.0) {
                points[vertexIdx].z = smoothedZ;
            }
        }
    }
}

// 添加边到映射表
void SurfaceProcessor::addEdgeToMap(std::map<std::pair<int, int>, std::vector<int>>& edgeMap,
                                         int a, int b, int triIdx)
{
    if (a < b) {
        edgeMap[{a, b}].push_back(triIdx);
    } else {
        edgeMap[{b, a}].push_back(triIdx);
    }
}

// 处理边上的顶点
void SurfaceProcessor::smoothEdgeVertices(
    int v1, int v2,
    bool isBoundaryEdge,
    const std::vector<int>& triIndices,
    std::vector<delaunay::Triangle>& triangles,
    std::vector<delaunay::Point>& points,
    const std::vector<double>& originalZ)
{
    // 收集这条边相关的所有顶点
    std::set<int> relatedVertices;
    relatedVertices.insert(v1);
    relatedVertices.insert(v2);

    for (int triIdx : triIndices) {
        auto& tri = triangles[triIdx];
        relatedVertices.insert(tri.a);
        relatedVertices.insert(tri.b);
        relatedVertices.insert(tri.c);
    }

    // 计算平均高度
    double sumZ = 0.0;
    int count = 0;

    for (int idx : relatedVertices) {
        if (idx >= 0 && idx < static_cast<int>(originalZ.size())) {
            sumZ += originalZ[idx];
            count++;
        }
    }

    if (count >= 2) {
        double avgZ = sumZ / count;

        // 根据是否为边界边应用不同的平滑强度
        double weight = isBoundaryEdge ? 0.7 : 0.4;

        // 平滑两个端点
        if (v1 >= 0 && v1 < static_cast<int>(points.size())) {
            double newZ = originalZ[v1] * (1 - weight) + avgZ * weight;
            double delta = std::abs(newZ - originalZ[v1]);

            // 限制最大变化量
            if (delta < 2.0) {
                points[v1].z = newZ;
            }
        }

        if (v2 >= 0 && v2 < static_cast<int>(points.size())) {
            double newZ = originalZ[v2] * (1 - weight) + avgZ * weight;
            double delta = std::abs(newZ - originalZ[v2]);

            if (delta < 2.0) {
                points[v2].z = newZ;
            }
        }
    }
}

// 对内部顶点进行额外平滑
void SurfaceProcessor::smoothInnerVertices(
    std::vector<delaunay::Triangle>& triangles,
    std::vector<delaunay::Point>& points,
    const std::vector<double>& originalZ)
{
    // 创建顶点到三角形的映射
    std::unordered_map<int, std::vector<int>> vertexToTriangles;

    for (size_t i = 0; i < triangles.size(); ++i) {
        auto& tri = triangles[i];
        if (tri.is_bad || tri.a < 4 || tri.b < 4 || tri.c < 4) {
            continue;
        }

        vertexToTriangles[tri.a].push_back(i);
        vertexToTriangles[tri.b].push_back(i);
        vertexToTriangles[tri.c].push_back(i);
    }

    // 对每个顶点进行平滑
    for (const auto& pair : vertexToTriangles) {
        int vertexIdx = pair.first;
        const auto& triIndices = pair.second;

        // 跳过边界顶点（已经在边处理中处理过）
        if (isBoundaryVertex(vertexIdx, triIndices, triangles)) {
            continue;
        }

        // 内部顶点使用更强的平滑
        double sumZ = 0.0;
        double weightSum = 0.0;

        std::set<int> visited;
        visited.insert(vertexIdx);

        for (int triIdx : triIndices) {
            auto& tri = triangles[triIdx];

            size_t vertices[3] = {tri.a, tri.b, tri.c};
            for (int j = 0; j < 3; ++j) {
                int neighborIdx = vertices[j];

                if (visited.count(neighborIdx)) {
                    continue;
                }

                visited.insert(neighborIdx);

                // 距离加权
                double dist = calculateDistance(points[vertexIdx], points[neighborIdx]);
                double weight = (dist < 0.001) ? 1.0 : (1.0 / dist);

                sumZ += originalZ[neighborIdx] * weight;
                weightSum += weight;
            }
        }

        if (weightSum > 0.0) {
            double smoothedZ = sumZ / weightSum;
            double delta = std::abs(smoothedZ - originalZ[vertexIdx]);

            // 内部顶点可以有更大的变化
            if (delta < 2.0) {
                points[vertexIdx].z = smoothedZ;
            }
        }
    }
}

// 计算两点之间的距离
double SurfaceProcessor::calculateDistance(const delaunay::Point& p1, const delaunay::Point& p2)
{
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;

    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// 判断顶点是否为边界顶点
bool SurfaceProcessor::isBoundaryVertex(
    int vertexIdx,
    const std::vector<int>& triIndices,
    std::vector<delaunay::Triangle>& triangles)
{
    std::set<std::pair<int, int>> edges;

    for (int triIdx : triIndices) {
        auto& tri = triangles[triIdx];
        addEdge(edges, tri.a, tri.b);
        addEdge(edges, tri.b, tri.c);
        addEdge(edges, tri.c, tri.a);
    }

    for (const auto& edge : edges) {
        int count = countEdgeOccurrences(edge.first, edge.second, triangles);
        if (count == 1) {
            return true;
        }
    }

    return false;
}

// 添加边到集合
void SurfaceProcessor::addEdge(std::set<std::pair<int, int>>& edges, int a, int b)
{
    if (a < b) {
        edges.insert({a, b});
    } else {
        edges.insert({b, a});
    }
}

// 统计边出现次数
int SurfaceProcessor::countEdgeOccurrences(
    int a, int b,
    std::vector<delaunay::Triangle>& triangles)
{
    int count = 0;
    int minIdx = std::min(a, b);
    int maxIdx = std::max(a, b);

    for (auto& tri : triangles) {
        if (tri.is_bad) continue;

        int t0 = std::min(std::min(tri.a, tri.b), tri.c);
        int t2 = std::max(std::max(tri.a, tri.b), tri.c);

        if (minIdx < t0 || maxIdx > t2) {
            continue;
        }

        bool hasA = (tri.a == a || tri.b == a || tri.c == a);
        bool hasB = (tri.a == b || tri.b == b || tri.c == b);

        if (hasA && hasB) {
            count++;
        }
    }

    return count;
}


// 在 surface_processor.cpp 中实现边界顶点提取
// surface_processor.cpp
void SurfaceProcessor::extractBoundaryVertices()
{
    boundaryVertexIndices_.clear();

    const auto& triangles = delaunayProc_.getTriangles();

    if (triangles.empty()) {
        qDebug() << "extractBoundaryVertices: triangles empty";
        return;
    }

    // 统计每条边出现的次数
    std::map<std::pair<int, int>, int> edgeCount;
    for (const auto& tri : triangles) {
        if (tri.is_bad || tri.a < 4 || tri.b < 4 || tri.c < 4) {
            continue;
        }

        int a = std::min(tri.a, tri.b);
        int b = std::max(tri.a, tri.b);
        edgeCount[{a, b}]++;

        a = std::min(tri.b, tri.c);
        b = std::max(tri.b, tri.c);
        edgeCount[{a, b}]++;

        a = std::min(tri.c, tri.a);
        b = std::max(tri.c, tri.a);
        edgeCount[{a, b}]++;
    }

    // 找出边界顶点索引
    std::unordered_set<int> boundaryVertexSet;
    for (const auto& pair : edgeCount) {
        if (pair.second == 1) {  // 只出现一次的边是边界边
            boundaryVertexSet.insert(pair.first.first);
            boundaryVertexSet.insert(pair.first.second);
        }
    }

    boundaryVertexIndices_.assign(boundaryVertexSet.begin(), boundaryVertexSet.end());

    qDebug() << QString("extractBoundaryVertices: found %1 boundary vertices").arg(boundaryVertexIndices_.size());

    // 将边界顶点传递给 SurfaceView
    if (surfaceView_) {
        const auto& pts = delaunayProc_.getPoints();
        QVector<QVector3D> boundaryVertices;

        for (int idx : boundaryVertexIndices_) {
            if (idx >= 4 && idx < static_cast<int>(pts.size())) {
                boundaryVertices.append(QVector3D(static_cast<float>(pts[idx].x),
                    static_cast<float>(pts[idx].y), static_cast<float>(pts[idx].z)));
            }
        }
        qDebug() << "2222222222222222222222";
        // 通过 DataProcessor 发送
        QMetaObject::invokeMethod(dataProcessor_, "postSurfaceBoundaryVertices", Qt::QueuedConnection,
                                  Q_ARG(QVector<QVector3D>, boundaryVertices));
    }
}















void SurfaceProcessor::clipHeightFieldToPolygon()
{
    if (!dataProcessor_ || !dataProcessor_->datasetPtr_) {
        return;
    }

    const QVector<North_East_Down>& polygonNed = dataProcessor_->datasetPtr_->getPolygonOutlineNED();
    if (polygonNed.isEmpty()) {
        return;
    }

    // qDebug() << "Clipping height field to polygon boundary, polygon size:" << polygonNed.size();

    const auto& tilesRef = surfaceMeshPtr_->getTilesCRef();
    const int stepPix = surfaceMeshPtr_->getStepSizeHeightMatrix();
    const int tileSidePix = surfaceMeshPtr_->getTileSidePixelSize();
    const int hvSide = tileSidePix / stepPix + 1;

    // 遍历所有瓦片
    for (auto* tile : tilesRef) {
        if (!tile || !tile->getIsInited()) {
            continue;
        }

        auto& vertices = tile->getHeightVerticesRef();
        auto& marks = tile->getHeightMarkVerticesRef();


        // 遍历瓦片内的所有顶点
        for (int y = 0; y < hvSide; ++y) {
            for (int x = 0; x < hvSide; ++x) {
                int idx = y * hvSide + x;
                QVector3D& vertex = vertices[idx];

                // 计算该顶点相对于瓦片原点的偏移
                // 注意：顶点数组中的坐标可能已经是物理坐标，直接使用即可
                float worldX = vertex.x();
                float worldY = vertex.y();

                // 如果顶点在多边形外，设置为无效高度
                if (!isPointInPolygon(QVector3D(worldX, worldY, 0))) {
                    vertex.setZ(std::numeric_limits<float>::quiet_NaN());
                    marks[idx] = HeightType::kUndefined;
                }
            }
        }

        tile->setIsUpdated(true);

    }

}








/*--------------------自动绘制多边形--------------------------*/
// // Edge（无向边，自动排序）
struct Edge2 {
    int a, b;

    Edge2(int p1, int p2) {
        if (p1 < p2) { a = p1; b = p2; }
        else { a = p2; b = p1; }
    }

    bool operator==(const Edge2& other) const {
        return a == other.a && b == other.b;
    }
    // 添加 operator< 用于 QMap
    bool operator<(const Edge2& other) const {
        if (a != other.a) return a < other.a;
        return b < other.b;
    }
};
inline uint qHash(const Edge2& e, uint seed = 0) {
    return qHash(e.a, seed) ^ qHash(e.b, seed);
}

// 外接圆半径计算
double SurfaceProcessor::circumradius(const QPointF& A, const QPointF& B, const QPointF& C)
{
    double a = QLineF(B, C).length();
    double b = QLineF(A, C).length();
    double c = QLineF(A, B).length();

    double s = (a + b + c) * 0.5;
    double area = std::sqrt(std::max(s * (s - a) * (s - b) * (s - c), 0.0));

    if (area < 1e-12) return 1e12;

    return (a * b * c) / (4.0 * area);
}

// 多边形面积（判断方向）
double SurfaceProcessor::polygonArea(const QPolygonF& poly)
{
    double area = 0;
    for (int i = 0; i < poly.size(); ++i)
    {
        const QPointF& p1 = poly[i];
        const QPointF& p2 = poly[(i + 1) % poly.size()];
        area += (p1.x() * p2.y() - p2.x() * p1.y());
    }
    return area * 0.5;
}


// 自动估计 alpha 值
double SurfaceProcessor::estimateAlpha(const std::vector<delaunay::Triangle>& triangles, const std::vector<delaunay::Point>& points)
{
    if (triangles.empty()) return 1.0;

    double totalRadius = 0.0;
    double maxRadius = 0.0;
    int count = 0;

    QVector<QPointF> qpoints;
    qpoints.reserve(points.size());
    for (const auto& pt : points) {
        qpoints.append(QPointF(pt.x, pt.y));
    }

    for (const auto& t : triangles) {
        if (t.is_bad) continue;
        if (t.a >= points.size() || t.b >= points.size() || t.c >= points.size()) {
            continue;
        }

        QPointF A = qpoints[t.a];
        QPointF B = qpoints[t.b];
        QPointF C = qpoints[t.c];

        double r = circumradius(A, B, C);
        totalRadius += r;
        if (r > maxRadius) maxRadius = r;
        count++;
    }

    if (count == 0) return 1.0;

    double avgRadius = totalRadius / count;
    return (avgRadius + maxRadius) / 2.0;
}


// 计算叉积 (o -> a) x (o -> b)
double SurfaceProcessor::cross(const Point3D<double>& o, const Point3D<double>& a, const Point3D<double>& b)
{
    return (a.x() - o.x()) * (b.y() - o.y()) - (a.y() - o.y()) * (b.x() - o.x());
}
// 凸包算法：Andrew 单调链算法
std::vector<Point3D<double>> SurfaceProcessor::convexHull(std::vector<Point3D<double>> points)
{
    // 1. 按 x 坐标排序，x 相同按 y 坐标排序
    std::sort(points.begin(), points.end(), [](const Point3D<double>& a, const Point3D<double>& b) {
        if (a.x() != b.x()) return a.x() < b.x();
        return a.y() < b.y();
    });

    int n = points.size();

    // 2. 构建下凸包
    std::vector<Point3D<double>> lower;
    for (int i = 0; i < n; i++) {
        while (lower.size() >= 2 && cross(lower[lower.size()-2], lower.back(), points[i]) <= 0) {
            lower.pop_back();
        }
        lower.push_back(points[i]);
    }

    // 3. 构建上凸包
    std::vector<Point3D<double>> upper;
    for (int i = n - 1; i >= 0; i--) {
        while (upper.size() >= 2 && cross(upper[upper.size()-2], upper.back(), points[i]) <= 0) {
            upper.pop_back();
        }
        upper.push_back(points[i]);
    }

    // 4. 合并上下凸包
    lower.pop_back();
    upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());

    return lower;
}

QVector<QVector3D> SurfaceProcessor::extractAlphaShapeBoundary(double alpha)
{
    QVector<QVector3D> boundaryPoints;

    // 获取三角网数据
    const auto& triangles = delaunayProc_.getTriangles();
    const auto& points = delaunayProc_.getPoints();
    if (triangles.empty() || points.empty()) {
        return boundaryPoints;
    }

    QVector<QPointF> qpoints;
    qpoints.reserve(points.size());
    for (const auto& pt : points) {
        qpoints.append(QPointF(pt.y, pt.x));
    }

    // 如果没有指定 alpha，自动计算
    double effectiveAlpha = alpha;
    if (effectiveAlpha <= 0.0) {
        effectiveAlpha = estimateAlpha(triangles, points);
    }

    QMap<Edge2, int> edgeCount;

    // Alpha过滤
    for (const auto& t : triangles)
    {
        if (t.is_bad) continue;

        if (t.a >= points.size() || t.b >= points.size() || t.c >= points.size()) {
            continue;
        }

        QPointF A = qpoints[t.a];
        QPointF B = qpoints[t.b];
        QPointF C = qpoints[t.c];

        double r = circumradius(A, B, C);

        if (r > effectiveAlpha) continue;

        Edge2 e1(t.a, t.b);
        Edge2 e2(t.b, t.c);
        Edge2 e3(t.c, t.a);

        edgeCount[e1]++;
        edgeCount[e2]++;
        edgeCount[e3]++;
    }

    // 取边界边（只出现1次）
    QMultiMap<int, int> adj;

    for (auto it = edgeCount.begin(); it != edgeCount.end(); ++it)
    {
        if (it.value() == 1)
        {
            adj.insert(it.key().a, it.key().b);
            adj.insert(it.key().b, it.key().a);
        }
    }

    // 多轮廓提取
    QSet<int> visited;
    QVector<QPolygonF> result;

    for (int start : adj.keys())
    {
        if (visited.contains(start)) continue;

        QPolygonF poly;

        int current = start;
        int prev = -1;

        while (true) {
            poly << qpoints[current];
            visited.insert(current);

            auto neighbors = adj.values(current);

            int next = -1;
            for (int n : neighbors) {
                if (n != prev) {
                    next = n;
                    break;
                }
            }

            if (next == -1 || next == start)
                break;

            prev = current;
            current = next;
        }

        if (poly.size() > 2)  result.push_back(poly);
    }

    // 统一方向（外环CCW，内环CW）
    for (auto& poly : result)
    {
        if (polygonArea(poly) < 0) {
            std::reverse(poly.begin(), poly.end());
        }
    }

    // 提取最大的轮廓（假设是外环）
    if (result.isEmpty()) {
        // 回退到凸包
        std::vector<Point3D<double>> allPoints;
        allPoints.reserve(points.size());
        for (const auto& pt : points) {
            allPoints.emplace_back(pt.x, pt.y, pt.z);
        }
        auto polygonPoints = convexHull(allPoints);
        for (const auto& pt : polygonPoints) {
            boundaryPoints.append(QVector3D(pt.x(), pt.y(), pt.z()));
        }
    }
    else {
        // 选择最大的轮廓
        QPolygonF maxPoly = result[0];
        double maxArea = std::abs(polygonArea(maxPoly));
        for (int i = 1; i < result.size(); ++i) {
            double area = std::abs(polygonArea(result[i]));
            if (area > maxArea) {
                maxArea = area;
                maxPoly = result[i];
            }
        }

        for (const auto& pt : maxPoly) {
            boundaryPoints.append(QVector3D(pt.x(), pt.y(), 0));
        }
    }

    return boundaryPoints;
}
