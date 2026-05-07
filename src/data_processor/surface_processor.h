#pragma once

#include <stdint.h>
#include <QHash>
#include <QPair>
#include <QPointF>
#include <QReadWriteLock>
#include <unordered_set>
#include "delaunay.h"
#include "isobaths_defs.h"
#include "surface_tile.h"
#include "bottom_track.h"
#include "point_3d.h"

enum class EdgeDirection { Top, Bottom, Left, Right };


class BottomTrack;
class DataProcessor;
class SurfaceMesh;
class SurfaceTile;
class SurfaceProcessor
{
public:
    explicit SurfaceProcessor(DataProcessor* parent);
    ~SurfaceProcessor();

    void clear();

    void setBottomTrackPtr(BottomTrack* bottomTrackPtr);
    void setSurfaceMeshPtr(SurfaceMesh* surfaceMeshPtr);
    void onUpdatedBottomTrackData(const QVector<QPair<char, int>>& indxs); //基于渲染缓存索引运行的海底轨迹
    void setTileResolution(float tileResolution);
    void setEdgeLimit(float val);
    void rebuildColorIntervals();
    void setSurfaceStepSize(float val);
    void setThemeId(int val);
    void setExtraWidth(int val);
    float getEdgeLimit() const;
    float getSurfaceStepSize() const;
    int getThemeId() const;
    int getExtraWidth() const;
    QVector<IsobathUtils::ColorInterval> getColorIntervals();

private:
    void writeTriangleToMesh(const QVector3D& A, const QVector3D& B, const QVector3D& C, QSet<SurfaceTile*>& updatedTiles);
    QVector<QVector3D> generateExpandedPalette(int totalColors) const;
    void updateTexture() const;
    void propagateBorderHeights(QSet<SurfaceTile*>& changedTiles);
    void refreshAfterEdgeLimitChange();
    bool canceled() const noexcept;
    bool isPointInPolygon(const QVector3D& point) const;
    void smoothTileHeights(SurfaceTile* tile);
    void clipHeightFieldToPolygon();




    void smoothDuringTriangulation();
    void addEdgeToMap(std::map<std::pair<int, int>, std::vector<int>>& edgeMap, int a, int b, int triIdx);
    void smoothEdgeVertices(int v1, int v2, bool isBoundaryEdge,const std::vector<int>& triIndices,
        std::vector<delaunay::Triangle>& triangles,std::vector<delaunay::Point>& points,
        const std::vector<double>& originalZ);
    void smoothInnerVertices( std::vector<delaunay::Triangle>& triangles,std::vector<delaunay::Point>& points,
                        const std::vector<double>& originalZ);
    double calculateDistance(const delaunay::Point& p1, const delaunay::Point& p2);
    bool isBoundaryVertex(int vertexIdx, const std::vector<int>& triIndices,
        std::vector<delaunay::Triangle>& triangles);
    void addEdge(std::set<std::pair<int, int>>& edges, int a, int b);
    int countEdgeOccurrences(
        int a, int b,
        std::vector<delaunay::Triangle>& triangles);
    std::vector<int> findBoundaryVertices(std::vector<delaunay::Triangle>& triangles);
    void smoothBoundaryVerticesEx(
        std::vector<delaunay::Triangle>& triangles,
        std::vector<delaunay::Point>& points,
        const std::vector<double>& originalZ,
        const std::vector<int>& boundaryVertices);
    void smoothInnerVerticesEx(
        std::vector<delaunay::Triangle>& triangles,
        std::vector<delaunay::Point>& points,
        const std::vector<double>& originalZ,
        const std::vector<int>& boundaryVertices);


    std::vector<std::vector<int>> buildBoundaryChains(const std::vector<std::pair<int, int>>& boundaryEdges,
                                     const std::vector<delaunay::Point>& points);


    std::vector<delaunay::Point> boundaryVertices_;  // 存储边界顶点
public:
    const std::vector<delaunay::Point>& getBoundaryVertices() const {
        return boundaryVertices_;
    }
    void extractBoundaryVertices();
    // 获取边界顶点索引（用于调试）
    const std::vector<int>& getBoundaryVertexIndices() const {
        return boundaryVertexIndices_;
    }

    // 设置 SurfaceView 指针
    void setSurfaceView(SurfaceView* view) {
        surfaceView_ = view;
    }

    std::vector<int> boundaryVertexIndices_;  // 存储边界顶点索引
    SurfaceView* surfaceView_ = nullptr;       // 指向 SurfaceView 的指针


    //从三角网提取边界轮廓
    static double circumradius(const QPointF& A, const QPointF& B, const QPointF& C);
    static double polygonArea(const QPolygonF& poly);
    double estimateAlpha(const std::vector<delaunay::Triangle>& triangles, const std::vector<delaunay::Point>& points);
    static double cross(const Point3D<double>& o, const Point3D<double>& a, const Point3D<double>& b);
    static std::vector<Point3D<double>> convexHull(std::vector<Point3D<double>> points);
    QVector<QVector3D> extractAlphaShapeBoundary(double alpha = 0.0);

private:
    DataProcessor* dataProcessor_;
    BottomTrack* bottomTrackPtr_;
    SurfaceMesh* surfaceMeshPtr_;
    delaunay::Delaunay delaunayProc_;
    kmath::MatrixParams lastMatParams_;
    QReadWriteLock lock_;
    QHash<uint64_t, QVector<int>> pointToTris_;
    QHash<QPair<int,int>, QVector3D> cellPoints_; // fir - virt indx, sec - indx in tr
    QHash<QPair<int,int>, int>  cellPointsInTri_;
    QVector<IsobathUtils::ColorInterval> colorIntervals_; // render
    QPointF origin_;
    float tileResolution_;
    float minZ_;
    float maxZ_;
    float edgeLimit_;
    float surfaceStepSize_;
    int tileSidePixelSize_;
    int tileHeightMatrixRatio_;
    int themeId_;
    int cellPx_;
    int extraWidth_;
    bool originSet_;
    QSet<TileKey> visibleTileKeys_;


    // 边界框成员变量
    float minX_ = std::numeric_limits<float>::max();
    float maxX_ = std::numeric_limits<float>::lowest();
    float minY_ = std::numeric_limits<float>::max();
    float maxY_ = std::numeric_limits<float>::lowest();



};
