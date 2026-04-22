#ifndef TILE_GEOVISEARTH_PROVIDER_H
#define TILE_GEOVISEARTH_PROVIDER_H


#include "tile_provider.h"
#include "map_defs.h"


namespace map {


class TileGeovisEarthProvider : public TileProvider
{
public:
    TileGeovisEarthProvider();

    int32_t heightToTileZ(float height) const override final;  //距离地面高度转换成地图等级z
    int32_t lonToTileX(double lon, int z) const override final;
    std::tuple<int32_t, int32_t, int32_t> lonToTileXWithWrapAndBoundary(const double lonStart, const double lonEnd, const int z) const override final;
    int32_t latToTileY(double lat, int z) const override final;
    map::TileInfo indexToTileInfo(map::TileIndex tileIndx, map::TilePosition pos = map::TilePosition::kFits) const override final;
    QString createURL(const map::TileIndex& tileIndx) const override final;

};



}


#endif // TILE_GEOVISEARTH_PROVIDER_H
