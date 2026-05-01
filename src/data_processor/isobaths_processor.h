#pragma once

#include <QVector>
#include <QVector3D>
#include "isobaths_defs.h"
#include "surface_mesh.h"
#include "dataset_defs.h"

using namespace IsobathUtils;


class DataProcessor;
class IsobathsProcessor
{
public:
    explicit IsobathsProcessor(DataProcessor* dataProcessorPtr);
    ~IsobathsProcessor() = default;

    void clear();
    void setSurfaceMeshPtr(SurfaceMesh* surfaceMeshPtr);
    void onUpdatedBottomTrackData();

    void  setMinZ(float v);
    void  setMaxZ(float v);
    void  setLineStepSize(float v);
    void  setLabelStepSize(float v);
    float getLineStepSize()  const;
    float getLabelStepSize() const;
    void  setColorsFromSurfaceProcessor(const QVector<IsobathUtils::ColorInterval>& colorIntervals);
    QVector3D getColorForDepth(float depth) const;


private:
    void fullRebuildLinesLabels();

    void buildPolylines(const IsobathsSegVec& segs, IsobathsPolylines& p) const;
    void edgeIntersection(const QVector3D& u,const QVector3D& v,float L, QVector<QVector3D>& out) const;
    void filterNearbyLabels(const QVector<LabelParameters>& in, QVector<LabelParameters>& out) const;

    void smoothPolyline(QList<QVector3D>& poly) const;

    bool canceled() const noexcept;

private:
    DataProcessor* dataProcessor_;
    SurfaceMesh* surfaceMeshPtr_;
    std::vector<QVector3D> vertPool_;
    std::vector<HeightType> vertMark_;
    std::vector<TrIndxs> tris_;
    QVector<QVector3D> lineSegments_;
    QVector<IsobathUtils::ColoredIsobathsSeg> coloredLineSegments_;
    QVector<LabelParameters> labels_;
    QVector<IsobathUtils::ColorInterval> colorIntervals_;
    float minZ_;
    float maxZ_;
    float lineStepSize_; //相邻两条等高线之间的高度差
    float labelStepSize_;



    // 在 IsobathsProcessor 类中添加：
private:
    // 新增：多边形裁剪相关
    bool isPointInPolygon(const QVector3D& point, const QVector<North_East_Down>& polygon) const;
    QVector<QVector3D> clipSegmentToPolygon(const QVector3D& start, const QVector3D& end,
                                            const QVector<North_East_Down>& polygon) const;
    QVector<QVector3D> clipPolylineToPolygon(const QVector<QVector3D>& polyline,
                                             const QVector<North_East_Down>& polygon) const;
    bool lineSegmentIntersection(const QVector3D& p1, const QVector3D& p2,
                                                    const QVector3D& p3, const QVector3D& p4,
                                                    QVector3D& intersection) const;
    QVector3D findIntersectionWithPolygonEdge(const QVector3D& start, const QVector3D& end,
                                              const QVector<North_East_Down>& polygon) const;

};
