#ifndef TILE_AMAP_PROVIDER_H
#define TILE_AMAP_PROVIDER_H


#include "tile_provider.h"
#include "map_defs.h"


namespace map {

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

    static double transformLat(double x, double y);
    static double transformLon(double x, double y);
    static void   Mars2Wgs(double lng, double lat, double &wgs_lng, double &wgs_lat);
    static void   Wgs2Mars(double wgLat, double wgLon,double &mgLat,double &mgLon);
};





}

#endif // TILE_AMAP_PROVIDER_H
