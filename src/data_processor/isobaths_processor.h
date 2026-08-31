#pragma once

#include <QVector>
#include <QVector3D>
#include "isobaths_defs.h"
#include "surface_mesh.h"

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
    void  setIsobathsLevelCnt(int cnt);
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
    int isobathsLevelCnt_ = 8; //等高线层数
    float labelStepSize_ = 100.0; //间隔100米标签
};
