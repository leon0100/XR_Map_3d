#include "tile_amap_provider.h"

namespace map {


const double PI = 3.1415926535897932384626;
const double a = 6378245.0;
const double ee = 0.0066934216229659423;


TileAmapProvider::TileAmapProvider() :
    TileProvider(AMAP_PROVIDER_ID)
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

    double maxLat = 85.05112878;
    double clampedLat = std::max(-maxLat, std::min(maxLat, lat));
    double sinLat = sin(clampedLat * M_PI / 180.0);
    int32_t numTiles = 1 << z;
    double y = (0.5 - std::log((1 + sinLat) / (1 - sinLat)) / (4 * M_PI)) * numTiles;
    int32_t tileY = static_cast<int32_t>(floor(y));
    tileY = std::max(0, std::min(tileY, numTiles - 1));
    return tileY;
}

map::TileInfo TileAmapProvider::indexToTileInfo(map::TileIndex tileIndx, map::TilePosition pos) const
{
    TileInfo info;
    GeoBounds bounds;

    double numTiles = pow(2.0, tileIndx.z_);

    bounds.west = (tileIndx.x_ / numTiles) * 360.0 - 180.0;
    bounds.east = ((tileIndx.x_ + 1) / numTiles) * 360.0 - 180.0;
    double lat_rad_north = atan(sinh(M_PI * (1 - 2.0 * tileIndx.y_ / numTiles)));
    bounds.north = lat_rad_north * (180.0 / M_PI);
    double lat_rad_south = atan(sinh(M_PI * (1 - 2.0 * (tileIndx.y_ + 1) / numTiles)));
    bounds.south = lat_rad_south * (180.0 / M_PI);

    if (pos == TilePosition::kOnLeft) {
        bounds.west += 360.0;
        bounds.east += 360.0;
    } else if (pos == TilePosition::kOnRight) {
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

int TileAmapProvider::generateNum(int x, int y) const
{
    return (x + y) % 4;
}

void TileAmapProvider::generateWords(int x, int y, QString& sec1, QString& sec2) const
{
    int setLen = ((x * 3) + y) % 8;
    sec2 = secAmapWord.left(setLen);
    if (y >= 10000 && y < 100000) {
        sec1 = QStringLiteral("&s=");
    }
}

QString TileAmapProvider::createURL(const map::TileIndex& tileIndx) const
{
    qDebug() <<"TileAmapProvider::createURL...........";
    QString str1, str2;
    generateWords(tileIndx.x_, tileIndx.y_, str1, str2);
    return QString("http://wprd04.is.autonavi.com/appmaptile?style=6&x=%1&y=%2&z=%3").arg(tileIndx.x_).arg(tileIndx.y_).arg(tileIndx.z_);
}


bool TileAmapProvider::outOfChina(double lon, double lat)
{
    if (lon < 72.004 || lon > 137.8347) return true;
    if (lat < 0.8293 || lat > 55.8271) return true;
    return false;
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

LLA TileAmapProvider::wgs84ToGcj02(const LLA& wgs84)
{
    if (outOfChina(wgs84.longitude, wgs84.latitude)) {
        return wgs84;
    }

    double dLat = transformLat(wgs84.longitude - 105.0, wgs84.latitude - 35.0);
    double dLon = transformLon(wgs84.longitude - 105.0, wgs84.latitude - 35.0);
    double radLat = wgs84.latitude / 180.0 * PI;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * PI);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * PI);

    LLA gcj02;
    gcj02.longitude = wgs84.longitude + dLon;
    gcj02.latitude = wgs84.latitude + dLat;
    gcj02.altitude = wgs84.altitude;

    return gcj02;
}

LLA TileAmapProvider::gcj02ToWgs84(const LLA& gcj02)
{
    if (outOfChina(gcj02.longitude, gcj02.latitude)) {
        return gcj02;
    }

    double dLat = transformLat(gcj02.longitude - 105.0, gcj02.latitude - 35.0);
    double dLon = transformLon(gcj02.longitude - 105.0, gcj02.latitude - 35.0);
    double radLat = gcj02.latitude / 180.0 * PI;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * PI);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * PI);

    LLA wgs84;
    wgs84.longitude = gcj02.longitude - dLon;
    wgs84.latitude = gcj02.latitude - dLat;
    wgs84.latitude = gcj02.latitude;

    return wgs84;
}


map::TileIndex TileAmapProvider::llaToTileIndex(LLA lla, int z)
{
    qDebug() << "TileAmapProvider::llaToTileIndex............";
    // 将 GCJ-02 坐标转换为 WGS-84 坐标
    LLA wgs84 = gcj02ToWgs84(lla);

    // 使用转换后的 WGS-84 坐标计算瓦片索引
    return map::TileIndex(lonToTileX(wgs84.longitude, z), latToTileY(wgs84.latitude, z), z, providerId_);

}


}

