#ifndef TILE_AMAP_PROVIDER_H
#define TILE_AMAP_PROVIDER_H


#include "tile_provider.h"
#include "map_defs.h"


namespace map {

constexpr double AMAP_TILE_CONSTANT = 126543000.03392;
const int AMAP_PROVIDER_ID = 2;
const int amapSat = 997;
const QString secAmapWord = QStringLiteral("Galileo");




class TileAmapProvider : public TileProvider
{
public:
    TileAmapProvider();

    int32_t heightToTileZ(float height) const override final;  //距离地面高度转换成地图等级z
    int32_t lonToTileX(double lon, int z) const override final;
    std::tuple<int32_t, int32_t, int32_t> lonToTileXWithWrapAndBoundary(const double lonStart, const double lonEnd, const int z) const override final;
    int32_t latToTileY(double lat, int z) const override final;
    map::TileInfo indexToTileInfo(map::TileIndex tileIndx, map::TilePosition pos = map::TilePosition::kFits) const override final;
    QString createURL(const map::TileIndex& tileIndx) const override final;

private:
    int generateNum(int x, int y) const;
    void generateWords(const int x, const int y, QString& sec1, QString& sec2) const;

    // static void   Mars2Wgs( double lng, double lat, double *wgs_lng, double *wgs_lat);
    // static double transformLat(double x, double y);
    // static double transformLon(double x, double y);

};





}

#endif // TILE_AMAP_PROVIDER_H
