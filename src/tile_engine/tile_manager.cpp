#include "tile_manager.h"

#include <QDebug>
#include <QUrl>
#include <QThread>

#include "map_defs.h"
#include "tile_google_provider.h"
#include "tile_amap_provider.h"
#include "tile_openstreet_provider.h"
#include "tile_geovisearth_provider.h"


namespace map {


TileManager::TileManager(QObject *parent) :
    QObject(parent),
    tileProvider_(std::make_shared<TileGoogleProvider>()),
    tileDownloader_(std::make_shared<TileDownloader>(tileProvider_, maxConcurrentDownloads_)),
    tileDB_(std::make_shared<TileDB>(tileProvider_)),
    tileSet_(std::make_shared<TileSet>(tileProvider_, tileDB_, tileDownloader_, maxTilesCapacity_, minTilesCapacity_)),
    lastZoomLevel_(-1)
{
    auto connType = Qt::AutoConnection;
    // tileDownloader_ -> tileSet_
    QObject::connect(tileDownloader_.get(), &TileDownloader::tileDownloaded,  tileSet_.get(), &TileSet::onTileDownloaded,      connType);
    QObject::connect(tileDownloader_.get(), &TileDownloader::downloadStopped, tileSet_.get(), &TileSet::onTileDownloadStopped, connType);
    QObject::connect(tileDownloader_.get(), &TileDownloader::downloadFailed,  tileSet_.get(), &TileSet::onTileDownloadFailed,  connType);

    QThread* dbThread = new QThread();
    tileDB_->moveToThread(dbThread);
    dbThread->setObjectName("MapDBThread" + QString::number(currentMap_));


    // tileDB_ <-> tileSet_
    QObject::connect(tileDB_.get(),  &TileDB::tileLoaded,           tileSet_.get(), &TileSet::onTileLoaded,        connType);
    QObject::connect(tileDB_.get(),  &TileDB::tileLoadFailed,       tileSet_.get(), &TileSet::onTileLoadFailed,    connType);
    QObject::connect(tileDB_.get(),  &TileDB::tileLoadStopped,      tileSet_.get(), &TileSet::onTileLoadStopped,   connType);
    QObject::connect(tileSet_.get(), &TileSet::dbLoadTiles,         tileDB_.get(),  &TileDB::loadTiles,            connType);
    QObject::connect(tileSet_.get(), &TileSet::dbStopAndClearTasks, tileDB_.get(),  &TileDB::stopAndClearRequests, connType);
    QObject::connect(tileSet_.get(), &TileSet::dbStopLoadingTile,   tileDB_.get(),  &TileDB::stopLoading,          connType);
    QObject::connect(tileSet_.get(), &TileSet::dbSaveTile,          tileDB_.get(),  &TileDB::saveTile,             connType);
    QObject::connect(tileDB_.get(),  &TileDB::tileSaved,            tileSet_.get(), &TileSet::onTileSaved,         connType);

    QObject::connect(dbThread, &QThread::started,  tileDB_.get(), &TileDB::init,         connType);
    QObject::connect(dbThread, &QThread::finished, tileDB_.get(), &QObject::deleteLater, connType);
    QObject::connect(dbThread, &QThread::finished, dbThread,      &QThread::deleteLater, connType);

    dbThread->start();
}

void TileManager::onTileProcessed()
{
    if(isScreenSaveMode_) {
        // qDebug() << "dbReq_size: " << tileSet_->dbReq_size() <<"  " << tileSet_->dwReq_size();
        if(tileSet_->dbReqIsEmpty() && tileSet_->dwReqIsEmpty()) {
            isScreenSaveMode_ = false;
            emit targetTilesLoaded();
        }
    }

}

TileManager::~TileManager()
{

}

std::shared_ptr<TileSet> TileManager::getTileSetPtr() const
{
    return tileSet_;
}

MapSourceType TileManager::getCurrentMapType() const
{
    return currentMap_;
}

void TileManager::getRectRequest(QVector<LLA> request, bool isPerspective, LLARef viewLlaRef, bool screenSaveMode)
{
    isScreenSaveMode_ = screenSaveMode;
    if(screenSaveMode) {
       QObject::connect(tileDB_.get(), &TileDB::tileLoaded, this, &TileManager::onTileProcessed);
       QObject::connect(tileDownloader_.get(), &TileDownloader::tileDownloaded,  this, &TileManager::onTileProcessed);
    }

    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();
    int zoomLevel = -1;

    double minLat = std::numeric_limits<double>::max();
    double maxLat = std::numeric_limits<double>::lowest();
    double minLon = std::numeric_limits<double>::max();
    double maxLon = std::numeric_limits<double>::lowest();

    ZoomState zoomState = ZoomState::kUndefined;

    // dimensions
    for (auto& itm : request) {
        LLA lla(itm.latitude, itm.longitude, 0.0f);
        auto tileIndx = tileProvider_.get()->llaToTileIndex(lla, tileProvider_.get()->heightToTileZ(itm.altitude));

        minX = std::min(minX, tileIndx.x_);
        maxX = std::max(maxX, tileIndx.x_);
        minY = std::min(minY, tileIndx.y_);
        maxY = std::max(maxY, tileIndx.y_);

        if (itm.latitude  > maxLat) maxLat = itm.latitude;
        if (itm.latitude  < minLat) minLat = itm.latitude;
        if (itm.longitude > maxLon) maxLon = itm.longitude;
        if (itm.longitude < minLon) minLon = itm.longitude;

        if (zoomLevel == -1) {
            zoomLevel = tileIndx.z_;
            if (zoomLevel != lastZoomLevel_) {
                zoomState = lastZoomLevel_ > zoomLevel ? ZoomState::kOut : ZoomState::kIn;
                // qDebug() << "zoom level changed to:" << zoomLevel << "isPerspective" << isPerspective << "zoomState" << static_cast<int>(zoomState);
                lastZoomLevel_ = zoomLevel;
                emit zoomLevelChanged(zoomLevel);
            }
            else {
                zoomState = ZoomState::kUnchanged;
            }
        }
    }

    double lonEdge = 180.0;
    if (maxLon > lonEdge)
        maxLon = lonEdge;
    if (maxLon < -lonEdge)
        maxLon = -lonEdge;
    if (minLon > lonEdge)
        minLon = lonEdge;
    if (minLon < -lonEdge)
        minLon = -lonEdge;
    if (qFuzzyCompare(minLon, maxLon)) {
        return;
    }

    // 对于高德地图，扩大请求范围，补偿火星坐标偏移
    if (currentMap_ == amapMapSource) {
        double expandFactor = 0.02; // 大约2公里左右的缓冲区
        minLon -= expandFactor;
        maxLon += expandFactor;
        minLat -= expandFactor;
        maxLat += expandFactor;
    }

    auto [lonStartTile, lonEndTile, boundaryTile] = tileProvider_.get()->lonToTileXWithWrapAndBoundary(minLon, maxLon, zoomLevel);

    uint64_t reqSize = 0;
    QSet<TileIndex> indxRequest;

    if (boundaryTile == -1)
    {
        if(currentMap_ == amapMapSource) {
            int extraTilesX = 1;
            int extraTilesY = 2;
            int expandedLonStart = lonStartTile - extraTilesX;
            int expandedLonEnd   = lonEndTile + extraTilesX;
            int expandedMinY     = minY - extraTilesY;
            int expandedMaxY     = maxY + extraTilesY;

            // 确保瓦片索引有效
            int numTiles = 1 << zoomLevel;
            expandedLonStart = std::max(0, expandedLonStart);
            expandedLonEnd   = std::min(expandedLonEnd, numTiles - 1);
            expandedMinY     = std::max(0, expandedMinY);
            expandedMaxY     = std::min(expandedMaxY, numTiles - 1);

            reqSize = (expandedLonEnd - expandedLonStart + 1) * (expandedMaxY - expandedMinY + 1);
            if (reqSize < minTilesCapacity_) {
                for (int x = expandedLonStart; x <= expandedLonEnd; ++x) {
                    for (int y = expandedMinY; y <= expandedMaxY; ++y) {
                        TileIndex tileIndx(x, y, zoomLevel, tileProvider_->getProviderId());
                        indxRequest.insert(tileIndx);
                    }
                }
            }
        }
        else {
            reqSize = (lonEndTile - lonStartTile + 1) * (maxY - minY + 1);
            if (reqSize < minTilesCapacity_) {
                for (int x = lonStartTile; x <= lonEndTile; ++x) {
                    for (int y = minY; y <= maxY; ++y) {
                        TileIndex tileIndx(x, y, zoomLevel, tileProvider_->getProviderId());
                        indxRequest.insert(tileIndx);
                    }
                }
            }
        }
    }

    if (!indxRequest.isEmpty()) {
        tileSet_->onNewRequest(indxRequest, zoomState, viewLlaRef, isPerspective, minLon, maxLon);
    }
}

void TileManager::getLlaRef(LLARef viewLlaRef)
{
    tileSet_->onNewLlaRef(viewLlaRef);
}

void TileManager::switchMapSource(MapSourceType sourceType)
{
    if(currentMap_ == sourceType) {
        return;
    }

    tileDownloader_->stopAndClearRequests();
    tileDB_->stopAndClearRequests();

    QThread::msleep(500);

    currentMap_ = sourceType;
    switch (sourceType) {
        case googleMapSource:
            tileProvider_ = std::make_shared<TileGoogleProvider>();
            break;
        case amapMapSource:
            tileProvider_ = std::make_shared<TileAmapProvider>();
            break;
        case openStreetMapSource:
            tileProvider_ = std::make_shared<TileOpenStreetProvider>();
            break;
        case geovisEarthSource:
            tileProvider_ = std::make_shared<TileGeovisEarthProvider>();
            break;
        default:
            break;
    }

    tileDownloader_ = std::make_shared<TileDownloader>(tileProvider_, maxConcurrentDownloads_);
    tileDB_  = std::make_shared<TileDB>(tileProvider_);
    tileSet_ = std::make_shared<TileSet>(tileProvider_, tileDB_, tileDownloader_, maxTilesCapacity_, minTilesCapacity_);
    lastZoomLevel_ = -1;


    auto downloaderConnType = Qt::AutoConnection;
    QObject::connect(tileDownloader_.get(), &TileDownloader::tileDownloaded,  tileSet_.get(), &TileSet::onTileDownloaded,      downloaderConnType);
    QObject::connect(tileDownloader_.get(), &TileDownloader::downloadStopped, tileSet_.get(), &TileSet::onTileDownloadStopped, downloaderConnType);
    QObject::connect(tileDownloader_.get(), &TileDownloader::downloadFailed,  tileSet_.get(), &TileSet::onTileDownloadFailed,  downloaderConnType);

    QThread* dbThread = new QThread();
    tileDB_->moveToThread(dbThread);
    dbThread->setObjectName("MapDBThread" + QString::number(currentMap_));


    auto dbConnType = Qt::AutoConnection;
    // tileDB_ <-> tileSet_
    QObject::connect(tileDB_.get(),  &TileDB::tileLoaded,           tileSet_.get(), &TileSet::onTileLoaded,        dbConnType);
    QObject::connect(tileDB_.get(),  &TileDB::tileLoadFailed,       tileSet_.get(), &TileSet::onTileLoadFailed,    dbConnType);
    QObject::connect(tileDB_.get(),  &TileDB::tileLoadStopped,      tileSet_.get(), &TileSet::onTileLoadStopped,   dbConnType);
    QObject::connect(tileSet_.get(), &TileSet::dbLoadTiles,         tileDB_.get(),  &TileDB::loadTiles,            dbConnType);
    QObject::connect(tileSet_.get(), &TileSet::dbStopAndClearTasks, tileDB_.get(),  &TileDB::stopAndClearRequests, dbConnType);
    QObject::connect(tileSet_.get(), &TileSet::dbStopLoadingTile,   tileDB_.get(),  &TileDB::stopLoading,          dbConnType);
    QObject::connect(tileSet_.get(), &TileSet::dbSaveTile,          tileDB_.get(),  &TileDB::saveTile,             dbConnType);
    QObject::connect(tileDB_.get(),  &TileDB::tileSaved,            tileSet_.get(), &TileSet::onTileSaved,         dbConnType);

    QObject::connect(dbThread,  &QThread::started,   tileDB_.get(),   &TileDB::init,         dbConnType);
    QObject::connect(dbThread,  &QThread::finished,  tileDB_.get(),   &QObject::deleteLater, dbConnType);
    QObject::connect(dbThread,  &QThread::finished,  dbThread,        &QThread::deleteLater, dbConnType);

    dbThread->start();

    emit tileSetChanged(tileSet_);

}



} // namespace map
