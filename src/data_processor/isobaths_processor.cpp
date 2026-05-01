#include "isobaths_processor.h"

#include <unordered_map>
#include <QtMath>
#include "data_processor.h"
#include "math_defs.h"


static int findOrAddVertex(const QVector3D& vertice, HeightType heightType, std::unordered_map<VKey, int>& dict,
                           std::vector<QVector3D>& vertPool, std::vector<HeightType>& vertMark)
{
    const double SCALE = 100.0;
    VKey key{ int64_t(std::llround(vertice.x()*SCALE)), int64_t(std::llround(vertice.y()*SCALE)) };

    if (auto it = dict.find(key); it != dict.end()) {
        return it->second;
    }

    int idx = int(vertPool.size());
    dict.emplace(key, idx);
    vertPool.push_back(vertice);
    vertMark.push_back(heightType);
    return idx;
}

IsobathsProcessor::IsobathsProcessor(DataProcessor* dataProcessorPtr):
    dataProcessor_(dataProcessorPtr),surfaceMeshPtr_(nullptr),
    minZ_(std::numeric_limits<float>::max()), maxZ_(std::numeric_limits<float>::lowest()),
    lineStepSize_(3.0f),labelStepSize_(100.f)
{
    qRegisterMetaType<QVector<IsobathUtils::LabelParameters>>("QVector<IsobathUtils::LabelParameters>");
    qRegisterMetaType<QVector<IsobathUtils::ColoredIsobathsSeg>>("QVector<IsobathUtils::ColoredIsobathsSeg>");
}

void IsobathsProcessor::clear()
{
    lineSegments_.clear();
    coloredLineSegments_.clear();
    labels_.clear();
    colorIntervals_.clear();
    vertPool_.clear();
    vertMark_.clear();
    tris_.clear();
}

void IsobathsProcessor::setSurfaceMeshPtr(SurfaceMesh* surfaceMeshPtr)
{
    surfaceMeshPtr_ = surfaceMeshPtr;
}

void IsobathsProcessor::onUpdatedBottomTrackData()
{
    // qDebug() << "IsobathsProcessor::onUpdatedBottomTrackData...";
    fullRebuildLinesLabels();
}

void IsobathsProcessor::setColorsFromSurfaceProcessor(const QVector<IsobathUtils::ColorInterval>& colorIntervals)
{
    colorIntervals_ = colorIntervals;
}

QVector3D IsobathsProcessor::getColorForDepth(float depth) const
{
    if (colorIntervals_.isEmpty()) {
        return QVector3D(0.0f, 0.0f, 0.0f);
    }

    // 找到最接近且小于等于当前深度的颜色区间
    QVector3D color = colorIntervals_.last().color;
    for (int i = colorIntervals_.size() - 1; i >= 0; --i) {
        const auto& interval = colorIntervals_[i];
        if (interval.depth <= depth) {
            color = interval.color;
            break;
        }
    }
    return color;
}

void IsobathsProcessor::setMinZ(float v)
{
    minZ_ = v;
}

void IsobathsProcessor::setMaxZ(float v)
{
    maxZ_ = v;
}

void IsobathsProcessor::setLineStepSize(float v)
{
    lineStepSize_ = v;
}

void IsobathsProcessor::setLabelStepSize(float v)
{
    labelStepSize_ = v;
}

float IsobathsProcessor::getLineStepSize() const
{
    return lineStepSize_;
}

float IsobathsProcessor::getLabelStepSize() const
{
    return labelStepSize_;
}


//等值线边-平面求交函数
void IsobathsProcessor::edgeIntersection(const QVector3D& a,const QVector3D& b, float L, QVector<QVector3D>& out) const
{
    const float zA = a.z();
    const float zB = b.z();
    const float zShift = 0.03f;

    QVector3D A = a;
    A.setZ(zA + zShift);
    QVector3D B = b;
    B.setZ(zB + zShift);

    if (fabsf(zA - L) < kmath::fltEps && fabsf(zB - L) < kmath::fltEps) {
        out << A << B;
        return;
    }

    if ((zA - L) * (zB - L) > 0.f) {
        return;
    }

    if (fabsf(zA - L) < kmath::fltEps) {
        out << A;
        return;
    }

    if (fabsf(zB - L) < kmath::fltEps) {
        out << B;
        return;
    }

    float t = (L - zA) / (zB - zA);
    if (qIsFinite(t)) {
        out << (A + t * (B - A));
    }
}

//等深线（Isobaths）创建（重建）
// void IsobathsProcessor::fullRebuildLinesLabels()
// {
//     // qDebug() << "等深线重绘......." << maxZ_ << "  " << minZ_;
//     if (!surfaceMeshPtr_ || (minZ_ >= maxZ_ - kmath::fltEps)) {
//         return;
//     }

//     lineSegments_.clear();
//     labels_.clear();

//     // 唯一顶点, 三角形列表
//     vertPool_.clear();
//     vertMark_.clear();
//     tris_.clear();
//     std::unordered_map<VKey,int> vDict;
//     vDict.reserve(1 << 20);
//     for (auto* tile: surfaceMeshPtr_->getTilesCRef()) {
//         const auto& V = tile->getHeightVerticesCRef();
//         const auto& M = tile->getHeightMarkVerticesRef();
//         const int   N = qRound(std::sqrt(V.size()));

//         if (N < 2) {
//             continue;
//         }

//         if (canceled()) {
//             return;
//         }

//         for (int y = 0; y < N - 1; ++y) {
//             for (int x = 0; x < N - 1; ++x) {
//                 const int tl = y * N + x;
//                 const int tr = tl + 1;
//                 const int bl = (y + 1) * N + x;
//                 const int br = bl + 1;

//                 int v0 = findOrAddVertex(V[tl], M[tl], vDict, vertPool_, vertMark_);
//                 int v1 = findOrAddVertex(V[bl], M[bl], vDict, vertPool_, vertMark_);
//                 int v2 = findOrAddVertex(V[tr], M[tr], vDict, vertPool_, vertMark_);
//                 int v3 = findOrAddVertex(V[br], M[br], vDict, vertPool_, vertMark_);

//                 tris_.push_back({v0, v1, v2});
//                 tris_.push_back({v2, v1, v3});
//             }
//         }
//     }
//     // qDebug() << "tris_................" << tris_.size();

//     if (vertPool_.empty()) {
//         return;
//     }

//     const int levelCnt = static_cast<int>((maxZ_ - minZ_) / lineStepSize_) + 1; //等深线层数

//     QHash<int, IsobathsSegVec> segsByLvl;

//     for (const TrIndxs& t : tris_) { // 三角形的交点
//         const QVector3D  A   = vertPool_[t.a];
//         const QVector3D  B   = vertPool_[t.b];
//         const QVector3D  C   = vertPool_[t.c];
//         const HeightType mA  = vertMark_[t.a];
//         const HeightType mB  = vertMark_[t.b];
//         const HeightType mC  = vertMark_[t.c];

//         if (canceled()) {
//             return;
//         }

//         if (mA == HeightType::kUndefined || mB == HeightType::kUndefined || mC == HeightType::kUndefined) {
//             continue;
//         }

//         for (int lvl = 0; lvl < levelCnt; ++lvl) {
//             const float L = minZ_ + lvl * lineStepSize_;
//             QVector<QVector3D> ip;
//             edgeIntersection(A, B, L, ip);
//             edgeIntersection(B, C, L, ip);
//             edgeIntersection(C, A, L, ip);

//             if (ip.size() == 2) {
//                 segsByLvl[lvl] << canonSeg(ip[0], ip[1]);
//             }
//             else if (ip.size() == 3) {
//                 if (!fuzzyEq(ip[0], ip[1])) {
//                     segsByLvl[lvl] << canonSeg(ip[0], ip[1]);
//                 }
//                 if (!fuzzyEq(ip[1], ip[2])) {
//                     segsByLvl[lvl] << canonSeg(ip[1], ip[2]);
//                 }
//             }
//         }
//     }

//     QHash<int, IsobathsPolylines> polysByLvl; // 多线段
//     QVector<IsobathUtils::ColoredIsobathsSeg> resColoredLines;
//     for (auto it = segsByLvl.begin(); it != segsByLvl.end(); ++it) {
//         buildPolylines(it.value(), polysByLvl[it.key()]);
//     }

//     QVector<QVector3D> resLines;
//     QVector<LabelParameters> resLabels;

//     for (auto it = polysByLvl.begin(); it != polysByLvl.end(); ++it) {
//         const int lvl = it.key();
//         const float depth = minZ_ + lvl * lineStepSize_;
//         const auto& polys = it.value();

//         QVector3D color = getColorForDepth(depth);

//         // 线条
//         for (const auto& p : polys) {
//             for (int i = 0; i + 1 < p.size(); ++i) {
//                 resLines << p[i] << p[i + 1];
//                 resColoredLines << IsobathUtils::ColoredIsobathsSeg(p[i], p[i+1], color);
//             }
//         }

//         // label
//         float distNext = 0.0f;
//         for (const auto& p : polys) {
//             if (canceled()) {
//                 return;
//             }

//             QVector<float> segLen(p.size() - 1);
//             float polyLen = 0.0f;

//             for (int i = 0; i + 1 < p.size(); ++i) {
//                 segLen[i] = (p[i + 1] - p[i]).length();
//                 polyLen += segLen[i];
//             }

//             int cur = 0;
//             float off = 0.0f;

//             while(distNext < (polyLen - kmath::fltEps)) {
//                 while(cur < segLen.size() && (off + segLen[cur]) < (distNext - kmath::fltEps)) {
//                     off += segLen[cur];
//                     ++cur;
//                 }

//                 if (cur >= segLen.size()) {
//                     break;
//                 }

//                 float t = (distNext-off) / segLen[cur];
//                 QVector3D pos = p[cur] + t * (p[cur+1] - p[cur]);
//                 QVector3D dir = (p[cur+1] - p[cur]).normalized();
//                 dir.setZ(0.0f);
//                 resLabels << LabelParameters{ pos, dir, fabsf(depth) };
//                 distNext += labelStepSize_;
//             }

//             distNext -= polyLen;
//         }
//     }

//     if (dataProcessor_ && dataProcessor_->datasetPtr_) {
//         const QVector<North_East_Down>& polygonOutlineNed = dataProcessor_->datasetPtr_->getPolygonOutlineNED();

//         if (!polygonOutlineNed.isEmpty()) {
//             // 获取最外层等高线的高度（最低高度层）
//             float outermostHeight = minZ_;  // 最外层等高线的高度

//             // 外层轮廓的高度设为最外层等高线的1/2
//             float outlineHeight = outermostHeight / 2.0f;

//             // 获取外层轮廓的颜色（使用最外层等高线的颜色）
//             QVector3D outlineColor = getColorForDepth(outermostHeight);

//             // 将轮廓点转换为QVector3D格式
//             QVector<QVector3D> outlinePoints;
//             for (const auto& ned : polygonOutlineNed) {
//                 outlinePoints.append(QVector3D(ned.n, ned.e, outlineHeight));
//             }

//             // 将轮廓点连接成线段
//             for (int i = 0; i < outlinePoints.size(); ++i) {
//                 int next = (i + 1) % outlinePoints.size();
//                 resColoredLines << IsobathUtils::ColoredIsobathsSeg(outlinePoints[i], outlinePoints[next], outlineColor);
//             }

//             qDebug() << "Added user polygon outline as outermost contour with height:" << outlineHeight;
//         }
//     }

//     // qDebug() << "resLines.size() " << resLines.size();
//     filterNearbyLabels(resLabels, labels_);
//     lineSegments_ = std::move(resLines);
//     coloredLineSegments_ = std::move(resColoredLines);

//     QMetaObject::invokeMethod(dataProcessor_, "postIsobathsLineSegments", Qt::QueuedConnection, Q_ARG(QVector<QVector3D>, lineSegments_));
//     QMetaObject::invokeMethod(dataProcessor_, "postIsobathsColoredLineSegments",
//             Qt::QueuedConnection, Q_ARG(QVector<IsobathUtils::ColoredIsobathsSeg>, coloredLineSegments_));
//     QMetaObject::invokeMethod(dataProcessor_, "postIsobathsLabels", Qt::QueuedConnection, Q_ARG(QVector<IsobathUtils::LabelParameters>, labels_));
// }

void IsobathsProcessor::fullRebuildLinesLabels()
{
    if (!surfaceMeshPtr_ || (minZ_ >= maxZ_ - kmath::fltEps)) {
        return;
    }

    lineSegments_.clear();
    labels_.clear();

    // 唯一顶点, 三角形列表
    vertPool_.clear();
    vertMark_.clear();
    tris_.clear();
    std::unordered_map<VKey,int> vDict;
    vDict.reserve(1 << 20);
    for (auto* tile: surfaceMeshPtr_->getTilesCRef()) {
        const auto& V = tile->getHeightVerticesCRef();
        const auto& M = tile->getHeightMarkVerticesRef();
        const int   N = qRound(std::sqrt(V.size()));

        if (N < 2) {
            continue;
        }

        if (canceled()) {
            return;
        }

        for (int y = 0; y < N - 1; ++y) {
            for (int x = 0; x < N - 1; ++x) {
                const int tl = y * N + x;
                const int tr = tl + 1;
                const int bl = (y + 1) * N + x;
                const int br = bl + 1;

                int v0 = findOrAddVertex(V[tl], M[tl], vDict, vertPool_, vertMark_);
                int v1 = findOrAddVertex(V[bl], M[bl], vDict, vertPool_, vertMark_);
                int v2 = findOrAddVertex(V[tr], M[tr], vDict, vertPool_, vertMark_);
                int v3 = findOrAddVertex(V[br], M[br], vDict, vertPool_, vertMark_);

                tris_.push_back({v0, v1, v2});
                tris_.push_back({v2, v1, v3});
            }
        }
    }

    if (vertPool_.empty()) {
        return;
    }

    const int levelCnt = static_cast<int>((maxZ_ - minZ_) / lineStepSize_) + 1;

    QHash<int, IsobathsSegVec> segsByLvl;

    for (const TrIndxs& t : tris_) {
        const QVector3D  A   = vertPool_[t.a];
        const QVector3D  B   = vertPool_[t.b];
        const QVector3D  C   = vertPool_[t.c];
        const HeightType mA  = vertMark_[t.a];
        const HeightType mB  = vertMark_[t.b];
        const HeightType mC  = vertMark_[t.c];

        if (canceled()) {
            return;
        }

        if (mA == HeightType::kUndefined || mB == HeightType::kUndefined || mC == HeightType::kUndefined) {
            continue;
        }

        for (int lvl = 0; lvl < levelCnt; ++lvl) {
            const float L = minZ_ + lvl * lineStepSize_;
            QVector<QVector3D> ip;
            edgeIntersection(A, B, L, ip);
            edgeIntersection(B, C, L, ip);
            edgeIntersection(C, A, L, ip);

            if (ip.size() == 2) {
                segsByLvl[lvl] << canonSeg(ip[0], ip[1]);
            }
            else if (ip.size() == 3) {
                if (!fuzzyEq(ip[0], ip[1])) {
                    segsByLvl[lvl] << canonSeg(ip[0], ip[1]);
                }
                if (!fuzzyEq(ip[1], ip[2])) {
                    segsByLvl[lvl] << canonSeg(ip[1], ip[2]);
                }
            }
        }
    }

    QHash<int, IsobathsPolylines> polysByLvl;
    for (auto it = segsByLvl.begin(); it != segsByLvl.end(); ++it) {
        buildPolylines(it.value(), polysByLvl[it.key()]);
    }

    // ========== 获取用户绘制的多边形轮廓 ==========
    QVector<North_East_Down> polygonOutlineNed;
    bool hasPolygon = false;

    if (dataProcessor_ && dataProcessor_->datasetPtr_) {
        polygonOutlineNed = dataProcessor_->datasetPtr_->getPolygonOutlineNED();
        hasPolygon = !polygonOutlineNed.isEmpty();
    }
    // ============================================

    QVector<QVector3D> resLines;
    QVector<LabelParameters> resLabels;
    QVector<IsobathUtils::ColoredIsobathsSeg> resColoredLines;

    for (auto it = polysByLvl.begin(); it != polysByLvl.end(); ++it) {
        const int lvl = it.key();
        const float depth = minZ_ + lvl * lineStepSize_;
        const auto& polys = it.value();

        QVector3D color = getColorForDepth(depth);

        // 线条
        for (const auto& p : polys) {
            // ========== 裁剪等高线到多边形内 ==========
            QVector<QVector3D> processPoly = p;

            if (hasPolygon) {
                processPoly = clipPolylineToPolygon(p, polygonOutlineNed);
            }
            // =========================================

            for (int i = 0; i + 1 < processPoly.size(); ++i) {
                resLines << processPoly[i] << processPoly[i + 1];
                resColoredLines << IsobathUtils::ColoredIsobathsSeg(processPoly[i], processPoly[i+1], color);
            }
        }

        // label
        float distNext = 0.0f;
        for (const auto& p : polys) {
            if (canceled()) {
                return;
            }

            QVector<float> segLen(p.size() - 1);
            float polyLen = 0.0f;

            for (int i = 0; i + 1 < p.size(); ++i) {
                segLen[i] = (p[i + 1] - p[i]).length();
                polyLen += segLen[i];
            }

            int cur = 0;
            float off = 0.0f;

            while(distNext < (polyLen - kmath::fltEps)) {
                while(cur < segLen.size() && (off + segLen[cur]) < (distNext - kmath::fltEps)) {
                    off += segLen[cur];
                    ++cur;
                }

                if (cur >= segLen.size()) {
                    break;
                }

                float t = (distNext-off) / segLen[cur];
                QVector3D pos = p[cur] + t * (p[cur+1] - p[cur]);
                QVector3D dir = (p[cur+1] - p[cur]).normalized();
                dir.setZ(0.0f);
                resLabels << LabelParameters{ pos, dir, fabsf(depth) };
                distNext += labelStepSize_;
            }

            distNext -= polyLen;
        }
    }

    // ========== 添加用户绘制的多边形轮廓作为最外层等高线 ==========
    if (hasPolygon) {
        float outlineHeight = minZ_ / 2.0f;
        QVector3D outlineColor = getColorForDepth(minZ_);

        QVector<QVector3D> outlinePoints;
        for (const auto& ned : polygonOutlineNed) {
            outlinePoints.append(QVector3D(ned.n, ned.e, outlineHeight));
        }

        for (int i = 0; i < outlinePoints.size(); ++i) {
            int next = (i + 1) % outlinePoints.size();
            resColoredLines << IsobathUtils::ColoredIsobathsSeg(outlinePoints[i], outlinePoints[next], outlineColor);
        }

        qDebug() << "Added user polygon outline as outermost contour with height:" << outlineHeight;
    }
    // ===============================================================

    filterNearbyLabels(resLabels, labels_);
    lineSegments_ = std::move(resLines);
    coloredLineSegments_ = std::move(resColoredLines);

    QMetaObject::invokeMethod(dataProcessor_, "postIsobathsLineSegments", Qt::QueuedConnection, Q_ARG(QVector<QVector3D>, lineSegments_));
    QMetaObject::invokeMethod(dataProcessor_, "postIsobathsColoredLineSegments",
                              Qt::QueuedConnection, Q_ARG(QVector<IsobathUtils::ColoredIsobathsSeg>, coloredLineSegments_));
    QMetaObject::invokeMethod(dataProcessor_, "postIsobathsLabels", Qt::QueuedConnection, Q_ARG(QVector<IsobathUtils::LabelParameters>, labels_));
}

// 判断点是否在多边形内（射线法）
bool IsobathsProcessor::isPointInPolygon(const QVector3D& point, const QVector<North_East_Down>& polygon) const
{
    if (polygon.isEmpty()) {
        return true;
    }

    bool inside = false;
    int n = polygon.size();
    for (int i = 0, j = n - 1; i < n; j = i++) {
        const float xi = polygon.at(i).n;
        const float yi = polygon.at(i).e;
        const float xj = polygon.at(j).n;
        const float yj = polygon.at(j).e;

        const bool intersect = ((yi > point.y()) != (yj > point.y())) &&
                               (point.x() < (xj - xi) * (point.y() - yi) / (yj - yi + 1e-12f) + xi);
        if (intersect) {
            inside = !inside;
        }
    }

    return inside;
}

// 线段与多边形求交，返回裁剪后的线段
QVector<QVector3D> IsobathsProcessor::clipSegmentToPolygon(const QVector3D& start, const QVector3D& end,
                                                           const QVector<North_East_Down>& polygon) const
{
    QVector<QVector3D> result;

    bool startInside = isPointInPolygon(start, polygon);
    bool endInside = isPointInPolygon(end, polygon);

    if (startInside && endInside) {
        // 两者都在多边形内，保留整个线段
        result.append(start);
        result.append(end);
    }
    else if (startInside && !endInside) {
        // 起点在内，终点在外，保留从起点到交点的部分
        QVector3D intersection = findIntersectionWithPolygonEdge(start, end, polygon);
        if (!intersection.isNull()) {
            result.append(start);
            result.append(intersection);
        }
    }
    else if (!startInside && endInside) {
        // 起点在外，终点在内，保留从交点到终点的部分
        QVector3D intersection = findIntersectionWithPolygonEdge(start, end, polygon);
        if (!intersection.isNull()) {
            result.append(intersection);
            result.append(end);
        }
    }
    // 两者都在外，丢弃整个线段

    return result;
}

// 找到线段与多边形边的第一个交点
QVector3D IsobathsProcessor::findIntersectionWithPolygonEdge(const QVector3D& start, const QVector3D& end,
                                                             const QVector<North_East_Down>& polygon) const
{
    QVector3D closestIntersection;
    float minDist = std::numeric_limits<float>::max();

    int n = polygon.size();
    for (int i = 0; i < n; ++i) {
        const QVector3D a(polygon.at(i).n, polygon.at(i).e, 0);
        const QVector3D b(polygon.at((i + 1) % n).n, polygon.at((i + 1) % n).e, 0);

        QVector3D intersection;
        if (lineSegmentIntersection(start, end, a, b, intersection)) {
            float dist = (intersection - start).length();
            if (dist < minDist) {
                minDist = dist;
                closestIntersection = intersection;
            }
        }
    }

    if (minDist < std::numeric_limits<float>::max()) {
        return closestIntersection;
    }
    return QVector3D();  // 返回无效值
}

// 线段相交检测
bool IsobathsProcessor::lineSegmentIntersection(const QVector3D& p1, const QVector3D& p2,
                                                const QVector3D& p3, const QVector3D& p4,
                                                QVector3D& intersection) const
{
    const float eps = 1e-6f;

    QVector3D d1 = p2 - p1;
    QVector3D d2 = p4 - p3;
    QVector3D d3 = p1 - p3;

    float cross = d1.x() * d2.y() - d1.y() * d2.x();

    if (qAbs(cross) < eps) {
        return false;  // 平行线
    }

    float t = (d3.x() * d2.y() - d3.y() * d2.x()) / cross;
    float u = (d3.x() * d1.y() - d3.y() * d1.x()) / cross;

    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        intersection = p1 + t * d1;
        return true;
    }

    return false;
}

// 裁剪折线到多边形内
QVector<QVector3D> IsobathsProcessor::clipPolylineToPolygon(const QVector<QVector3D>& polyline,
                                                            const QVector<North_East_Down>& polygon) const
{
    if (polyline.isEmpty() || polygon.isEmpty()) {
        return polyline;
    }

    QVector<QVector3D> result;

    for (int i = 0; i < polyline.size() - 1; ++i) {
        const QVector3D& start = polyline[i];
        const QVector3D& end = polyline[i + 1];

        QVector<QVector3D> clipped = clipSegmentToPolygon(start, end, polygon);
        result.append(clipped);
    }

    return result;
}


//把杂乱等深线段拼接成最大连续折线
void IsobathsProcessor::buildPolylines(const IsobathsSegVec& segs, IsobathsPolylines& polys) const
{
    constexpr float EPS = 0.05f; // 5 см
    auto eq = [&](const QVector3D& a, const QVector3D& b) {
        return (a - b).lengthSquared() < (EPS * EPS);
    };

    QVector<char> used(segs.size(), 0);

    for (int i = 0; i < segs.size(); ++i) {
        if (used[i]) {
            continue;
        }

        QList<QVector3D> poly{ segs[i].first, segs[i].second };
        used[i] = 1;
        bool again = true;
        while (again) {
            again = false;
            for (int j = 0; j < segs.size(); ++j) {
                if (used[j]) {
                    continue;
                }

                if (eq(poly.back(), segs[j].first)) {
                    poly << segs[j].second;
                    used[j] = 1;
                    again = true;
                }
                else if (eq(poly.back(), segs[j].second)) {
                    poly << segs[j].first;
                    used[j] = 1;
                    again = true;
                }
                else if (eq(poly.front(),segs[j].second)) {
                    poly.prepend(segs[j].first);
                    used[j] = 1;
                    again = true;
                }
                else if (eq(poly.front(),segs[j].first )) {
                    poly.prepend(segs[j].second);
                    used[j] = 1;
                    again = true;
                }
            }
        }

        // qDebug() << "poly.size()............" << poly.size();
        if (poly.size() > 5) {
            smoothPolyline(poly);
            polys << QVector<QVector3D>(poly.begin(), poly.end());
        }
    }
}

// 新增20260328：平滑折线，减少锯齿和碎片
void IsobathsProcessor::smoothPolyline(QList<QVector3D>& poly) const
{
    if (poly.size() < 3) {
        return;
    }

    // 使用简单的移动平均平滑
    QList<QVector3D> smoothed;
    smoothed.append(poly.first()); // 保持起点不变

    for (int i = 1; i < poly.size() - 1; ++i) {
        // 三点加权平均：当前点权重0.5，相邻点各0.25
        QVector3D avg = 0.25f * poly[i-1] + 0.5f * poly[i] + 0.25f * poly[i+1];
        // 保持原始Z值（等高线高度）
        avg.setZ(poly[i].z());
        smoothed.append(avg);
    }

    smoothed.append(poly.last()); // 保持终点不变
    poly = smoothed;
}

//过滤掉彼此距离小于一定阈值的 Label
void IsobathsProcessor::filterNearbyLabels(const QVector<LabelParameters>& in, QVector<LabelParameters>& out) const
{
    const float cell = 20.f;
    const float inv = 1.f / cell;
    const float min2 = cell * cell;

    QHash<QPair<int,int>,QVector<QVector3D>> grid;
    for (const auto& lbl : in) {
        int cx = int(std::floor(lbl.pos.x() * inv));
        int cy = int(std::floor(lbl.pos.y() * inv));
        bool isNear = false;
        for (int dx = -1; dx <= 1 && !isNear; ++dx) {
            for(int dy = -1; dy <= 1 && !isNear; ++dy) {
                auto k = qMakePair(cx + dx, cy + dy);
                if (!grid.contains(k)) {
                    continue;
                }
                const auto& pGr = grid[k];
                for (const auto& p : pGr) {
                    if ((lbl.pos - p).lengthSquared() < min2) {
                        isNear = true;
                        break;
                    }
                }
            }
        }

        if (!isNear){
            out << lbl;
            grid[qMakePair(cx, cy)] << lbl.pos;
        }
    }
}

bool IsobathsProcessor::canceled() const noexcept
{
    return dataProcessor_ && dataProcessor_->isCancelRequested();
}
