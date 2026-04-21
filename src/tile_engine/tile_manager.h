#pragma once

#include <QObject>
#include <QVector>
#include <QVector3D>
#include <QDateTime>
#include <memory>

#include "tile_set.h"
#include "tile_provider.h"
#include "tile_downloader.h"
#include "tile_db.h"


namespace map {


class TileManager : public QObject
{
    Q_OBJECT
public:
    explicit TileManager(QObject* parent = nullptr);
    ~TileManager();

    std::shared_ptr<TileSet> getTileSetPtr() const;
    MapSourceType getCurrentMapType() const;

signals:
    void zoomLevelChanged(int level);
    void tileSetChanged(std::shared_ptr<TileSet> tileSet);

public slots:
    void getRectRequest(QVector<LLA> request, bool isPerspective, LLARef viewLlaRef, bool moveUp, map::CameraTilt tiltCam);
    void getLlaRef(LLARef viewLlaRef);
    void switchMapSource(MapSourceType sourceType);


private:
    std::shared_ptr<TileProvider> tileProvider_;
    std::shared_ptr<TileDownloader> tileDownloader_;
    std::shared_ptr<TileDB> tileDB_;
    std::shared_ptr<TileSet> tileSet_;
    int lastZoomLevel_;
    MapSourceType currentMap_ = googleMapSource;

    static constexpr int maxTilesCapacity_{ 800 };
    static constexpr int minTilesCapacity_{ 400 };
    static constexpr int maxConcurrentDownloads_{ 10 };






    double transformLat(double x, double y);
    double transformLon(double x, double y);
    void   Mars2Wgs(double lng, double lat, double &wgs_lng, double &wgs_lat);
    void   Wgs2Mars(double wgLat, double wgLon,double &mgLat,double &mgLon);
};


} // namespace map

