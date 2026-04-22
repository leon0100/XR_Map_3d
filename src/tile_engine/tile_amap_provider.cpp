#include "tile_amap_provider.h"

namespace map {

constexpr double AMAP_TILE_CONSTANT = 126543000.03392;
const int AMAP_PROVIDER_ID = 2;

TileAmapProvider::TileAmapProvider() : TileProvider(AMAP_PROVIDER_ID)
{

}

int32_t TileAmapProvider::heightToTileZ(const float height) const
{
    double z = std::log2(AMAP_TILE_CONSTANT / height);
    z = std::max(0.0, std::min(z, 21.0));
    return static_cast<int>(z);
}

int32_t TileAmapProvider::lonToTileX(const double lon, const int z) const
{
    if (z < 0 || z > 21) {
        qWarning() << "Invalid zoom level:" << z;
        return -1;
    }

    // double mgLon, mgLat;
    // // 【修改点】在这里将输入的 WGS-84 转换为 GCJ-02 (Mars)
    // Wgs2Mars(lon, 0, mgLon, mgLat);

    double normalizedLon = std::fmod(lon + 180.0, 360.0);
    return static_cast<int32_t>(floor(normalizedLon / 360.0 * pow(2.0, z)));
}

std::tuple<int32_t, int32_t, int32_t> TileAmapProvider::lonToTileXWithWrapAndBoundary(const double lonStart, const double lonEnd, const int z) const
{
    if (z < 0 || z > 21) {
        qWarning() << "Invalid zoom level:" << z;
        return {-1, -1, -1};
    }

    int32_t numTiles = static_cast<int32_t>(std::pow(2.0, z));

    int32_t tileStart = lonToTileX(lonStart, z);
    int32_t tileEnd = lonToTileX(lonEnd - 1e-9, z);

    bool crossesBoundary = (lonStart < -180.0 || lonEnd > 180.0 || lonStart > lonEnd);

    if (crossesBoundary) {
        int32_t boundaryTile = numTiles - 1;
        return {tileStart, tileEnd, boundaryTile};
    }
    else {
        return {tileStart, tileEnd, -1};
    }
}

int32_t TileAmapProvider::latToTileY(const double lat, const int z) const
{
    if (z < 0 || z > 21) {
        qWarning() << "Invalid zoom level:" << z;
        return -1;
    }

    // double mgLon, mgLat;
    // // 【修改点】在这里将输入的 WGS-84 转换为 GCJ-02 (Mars)
    // Wgs2Mars(0, lat, mgLon, mgLat);

    double maxLat = 85.05112878;
    double clampedLat = std::max(-maxLat, std::min(maxLat, lat));
    double sinLat = sin(clampedLat * M_PI / 180.0);
    int32_t numTiles = 1 << z;
    double y = (0.5 - std::log((1 + sinLat) / (1 - sinLat)) / (4 * M_PI)) * numTiles;
    int32_t tileY = static_cast<int32_t>(floor(y));
    tileY = std::max(0, std::min(tileY, numTiles - 1));
    return tileY;
}


double TileAmapProvider::transformLat(double x, double y)
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * PI) + 20.0 * sin(2.0 * x * PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * PI) + 40.0 * sin(y / 3.0 * PI)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * PI) + 320 * sin(y * PI / 30.0)) * 2.0 / 3.0;
    return ret;
}

double TileAmapProvider::transformLon(double x, double y)
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * PI) + 20.0 * sin(2.0 * x * PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * PI) + 40.0 * sin(x / 3.0 * PI)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * PI) + 300.0 * sin(x / 30.0 * PI)) * 2.0 / 3.0;
    return ret;
}
void TileAmapProvider::Mars2Wgs(double mgs_lng, double mgs_lat, double &wgs_lng, double &wgs_lat)
{
    if (mgs_lng < 72.004 || mgs_lng > 137.8347 || mgs_lat < 0.8293 || mgs_lat > 55.8271) {
        wgs_lng = mgs_lng;
        wgs_lat = mgs_lat;
        return;
    }

    double dlat = transformLat(mgs_lng - 105.0, mgs_lat - 35.0);
    double dlng = transformLon(mgs_lng - 105.0, mgs_lat - 35.0);
    double radlat = mgs_lat / 180.0 * PI;
    double magic = sin(radlat);
    magic = 1 - EE * magic * magic;
    double sqrtmagic = sqrt(magic);
    dlat = (dlat * 180.0) / ((SEMI_MAJOR_AXIS * (1 - EE)) / (magic * sqrtmagic) * PI);
    dlng = (dlng * 180.0) / (SEMI_MAJOR_AXIS / sqrtmagic * cos(radlat) * PI);
    double mglat = mgs_lat + dlat;
    double mglng = mgs_lng + dlng;

    wgs_lng = mgs_lng * 2 - mglng;
    wgs_lat = mgs_lat * 2 - mglat;
}

void TileAmapProvider::Wgs2Mars(double wgLon, double wgLat, double &mgLon,double &mgLat)
{
    if (wgLon < 72.004 || wgLon > 137.8347 || wgLat < 0.8293 || wgLat > 55.8271) {
        mgLon = wgLon;
        mgLat = wgLat;
        return;
    }

    double dLat = transformLat(wgLon - 105.0, wgLat - 35.0);
    double dLon = transformLon(wgLon - 105.0, wgLat - 35.0);
    double radLat = wgLat / 180.0 * PI;
    double magic = sin(radLat);
    magic = 1 - EE * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((SEMI_MAJOR_AXIS * (1 - EE)) / (magic * sqrtMagic) * PI);
    dLon = (dLon * 180.0) / (SEMI_MAJOR_AXIS / sqrtMagic * cos(radLat) * PI);
    mgLat = wgLat + dLat;
    mgLon = wgLon + dLon;
}


map::TileInfo TileAmapProvider::indexToTileInfo(map::TileIndex tileIndx, map::TilePosition pos) const
{
    TileInfo info;
    GeoBounds bounds;

    double numTiles = pow(2.0, tileIndx.z_);
    double originalWest  = (tileIndx.x_ / numTiles) * 360.0 - 180.0;
    double originalEast  = ((tileIndx.x_ + 1) / numTiles) * 360.0 - 180.0;
    double originalNorth = atan(sinh(M_PI * (1 - 2.0 * tileIndx.y_ / numTiles))) * (180.0 / M_PI);
    double originalSouth = atan(sinh(M_PI * (1 - 2.0 * (tileIndx.y_ + 1) / numTiles))) * (180.0 / M_PI);


    // 对于西侧和北侧边界，直接转换
    // 对于东侧和南侧边界，使用相邻瓦片的西侧和北侧边界值
    // 这样可以确保瓦片之间无缝拼接
    double wgsWest, wgsNorth;
    Mars2Wgs(originalWest, originalNorth, wgsWest, wgsNorth);
    bounds.west = wgsWest;
    bounds.north = wgsNorth;
    Mars2Wgs(originalEast, originalNorth, bounds.east, wgsNorth);
    Mars2Wgs(originalWest, originalSouth, wgsWest, bounds.south);

    if (pos == TilePosition::kOnLeft) {
        bounds.west += 360.0;
        bounds.east += 360.0;
    }
    else if (pos == TilePosition::kOnRight) {
        bounds.west -= 360.0;
        bounds.east -= 360.0;
    }

    double lat_center = (bounds.north + bounds.south) / 2.0;
    double resolution = (156543.03392804062 * cos(lat_center * M_PI / 180.0)) / pow(2.0, tileIndx.z_);
    double tileSizePixels = 256.0;
    info.tileSizeMeters = resolution * tileSizePixels;

    info.bounds = bounds;

    return info;
}


QString TileAmapProvider::createURL(const map::TileIndex& tileIndx) const
{
    return QString("http://wprd04.is.autonavi.com/appmaptile?style=6&x=%1&y=%2&z=%3")
        .arg(tileIndx.x_).arg(tileIndx.y_).arg(tileIndx.z_);
}


}

