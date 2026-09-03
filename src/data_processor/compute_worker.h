#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QSet>
#include <QUuid>
#include "surface_processor.h"
#include "isobaths_processor.h"
#include "mosaic_processor.h"


struct WorkBundle {
    QVector<QPair<char,int>> surfaceVec;
    QVector<int>             mosaicVec;
    bool                     doIsobaths{false};
};
Q_DECLARE_METATYPE(WorkBundle)

class Dataset;
class DataProcessor;
class SurfaceMesh;
class ComputeWorker : public QObject
{
    Q_OBJECT

public:
    explicit ComputeWorker(DataProcessor* ownerDp, Dataset* dataset, QObject* parent = nullptr);
    ~ComputeWorker();

public slots:
    void clearAll();
    void clearSurface();
    void clearMosaic();
    void clearIsobaths();

    void setDatasetPtr(Dataset* ds);
    void setBottomTrackPtr(BottomTrack* bt);
    void setSurfaceThemeId(int id);
    void setSurfaceEdgeLimit(float v);
    void setSurfaceIsobathsLevelCnt(int cnt);
    void setMosaicChannels(const ChannelId& ch1, uint8_t sub1, const ChannelId& ch2, uint8_t sub2);
    void setMosaicTheme(int id);
    void setMosaicLAngleOffset(float val);
    void setMosaicRAngleOffset(float val);
    void setMosaicLevels(float lo, float hi);
    void setMosaicLowLevel(float v);
    void setMosaicHighLevel(float v);
    void setMosaicTileResolution(float res);
    void setMinZ(float v);
    void setMaxZ(float v);

    void processBundle(const WorkBundle& wb);

signals:
    void jobFinished(); //用于 dataProcessor（正常，取消）


private:
    inline bool isCanceled() const noexcept;
    void adaptSurfaceResolution();

private:
    DataProcessor*       dataProcessor_;
    Dataset*             dataset_;
    SurfaceMesh          surfaceMesh_;

    SurfaceProcessor     surface_;
    IsobathsProcessor    isobaths_;
    MosaicProcessor      mosaic_;
};
