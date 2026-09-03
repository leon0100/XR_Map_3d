#include "compute_worker.h"
#include "data_processor.h"
#include "dataset.h"
#include "surface_mesh.h"
#include <QMetaType>
#include <QDebug>


ComputeWorker::ComputeWorker(DataProcessor* ownerDp, Dataset* dataset, QObject* parent)
    : QObject(parent),
      dataProcessor_(ownerDp),
      dataset_(dataset),
      surfaceMesh_(defaultTileSidePixelSize, defaultTileHeightMatrixRatio, defaultTileResolution),
      surface_(ownerDp),
      isobaths_(ownerDp),
      mosaic_(ownerDp)
{
    qRegisterMetaType<WorkBundle>("WorkBundle");

    surface_ .setSurfaceMeshPtr(&surfaceMesh_);
    isobaths_.setSurfaceMeshPtr(&surfaceMesh_);
    mosaic_  .setSurfaceMeshPtr(&surfaceMesh_);

    mosaic_.setDatasetPtr(dataset_);
}

ComputeWorker::~ComputeWorker() = default;

void ComputeWorker::clearAll()
{
    surface_.clear();
    mosaic_.clear();
    isobaths_.clear();

    surfaceMesh_.clear();
}

void ComputeWorker::clearSurface()
{
    surface_.clear();
}

void ComputeWorker::clearMosaic()
{
    mosaic_.clear();
}

void ComputeWorker::clearIsobaths()
{
    isobaths_.clear();
}

inline bool ComputeWorker::isCanceled() const noexcept
{
    return dataProcessor_ && dataProcessor_->isCancelRequested();
}

void ComputeWorker::setDatasetPtr(Dataset* ds)
{
    dataset_ = ds;
    // bottom_.setDatasetPtr(ds);
    mosaic_.setDatasetPtr(ds);
}

void ComputeWorker::setBottomTrackPtr(BottomTrack* bt)
{
    // qDebug() << "ComputeWorker::setBottomTrackPtr...........";
    surface_.setBottomTrackPtr(bt);
}

void ComputeWorker::setSurfaceThemeId(int id)
{
    surface_.setThemeId(id);
    surface_.rebuildColorIntervals(); //颜色区间重构
}

void ComputeWorker::setSurfaceEdgeLimit(float v)
{
    surface_.setEdgeLimit(v);
}

void ComputeWorker::setSurfaceIsobathsLevelCnt(int cnt)
{
    surface_.setSurfaceLevelCnt(cnt);
    isobaths_.setIsobathsLevelCnt(cnt);
}

void ComputeWorker::setMosaicChannels(const ChannelId& ch1, uint8_t sub1, const ChannelId& ch2, uint8_t sub2)
{
    clearAll();

    mosaic_.setChannels(ch1, sub1, ch2, sub2);
}

void ComputeWorker::setMosaicTheme(int id)
{
    mosaic_.setColorTableThemeById(id);
}

void ComputeWorker::setMosaicLAngleOffset(float val)
{
    mosaic_.setLAngleOffset(val);
}

void ComputeWorker::setMosaicRAngleOffset(float val)
{
    mosaic_.setRAngleOffset(val);
}

void ComputeWorker::setMosaicLevels(float lo, float hi)
{
    mosaic_.setColorTableLevels(lo, hi);
}

void ComputeWorker::setMosaicLowLevel(float v)
{
    mosaic_.setColorTableLowLevel(v);
}

void ComputeWorker::setMosaicHighLevel(float v)
{
    mosaic_.setColorTableHighLevel(v);
}

void ComputeWorker::setMosaicTileResolution(float res)
{
    if (res <= 0.f) {
        return;
    }

    surfaceMesh_.reinit(defaultTileSidePixelSize, defaultTileHeightMatrixRatio, res);
    surface_.setTileResolution(res);
    mosaic_.setTileResolution(res);
}

void ComputeWorker::setMinZ(float v)
{
    isobaths_.setMinZ(v);
}

void ComputeWorker::setMaxZ(float v)
{
    // qDebug() << "ComputeWorker::setMaxZ.................";
    isobaths_.setMaxZ(v);
}

void ComputeWorker::adaptSurfaceResolution()
{
    if (surfaceMesh_.getIsInited() || !dataset_) {
        return;
    }

    const float minX = dataset_->minX_;
    const float maxX = dataset_->maxX_;
    const float minY = dataset_->minY_;
    const float maxY = dataset_->maxY_;
    const float width  = (maxX - minX) + 200.f;
    const float height = (maxY - minY) + 200.f;
    if (width  < 0.0f || height < 0.0f) {
        return;
    }

    const int   kMaxSurfaceTiles = 4096;
    const float kMaxResolution   = 3.2f;

    const float curRes = surfaceMesh_.getTileResolution();
    float res = curRes;
    const int tileSidePixelSize = surfaceMesh_.getTileSidePixelSize();
    while (res < kMaxResolution) {
        const float tileSide = tileSidePixelSize * res;
        const double tiles = (width / tileSide) * (height / tileSide);
        if (tiles <= static_cast<double>(kMaxSurfaceTiles)) {
            break;
        }
        res *= 2.0f;
    }

    if (res != curRes) {
        qDebug() << "adaptSurfaceResolution:" << curRes << "->" << res;
        setMosaicTileResolution(res);
    }
}

void ComputeWorker::processBundle(const WorkBundle& wb)
{
    // qDebug() << "ComputeWorker::processBundle";
    // wb.surfaceVec:在底部轨迹数据数组中的索引
    if (!wb.surfaceVec.isEmpty() && !isCanceled()) {
        adaptSurfaceResolution();
        surface_.onUpdatedBottomTrackData(wb.surfaceVec); //生成高度场，不负责等值线的绘制，但是却为等值线提供高度场网格
        surface_.rebuildColorIntervals();
        auto colorIntervals = surface_.getColorIntervals();
        isobaths_.setColorsFromSurfaceProcessor(colorIntervals);
    }

    // if (!wb.mosaicVec.isEmpty() && !isCanceled()) {
    //     mosaic_.updateDataWrapper(wb.mosaicVec);
    // }

    if (wb.doIsobaths && !isCanceled()) {
        isobaths_.fullRebuildLinesLabels(); //只计算等值线....... 但它完全依赖于SurfaceProcessor生成的高度场网格。
    }

    // emit jobFinished();
}
