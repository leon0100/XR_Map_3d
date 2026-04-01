#include "outlinemanager.h"

void PolygonManager::addPoint(const LLA& point)
{
    polygonPoints_.append(point);
}

void PolygonManager::clear()
{
    polygonPoints_.clear();
}

bool PolygonManager::isEmpty() const
{
    return polygonPoints_.size() < 3;
}

QVector<LLA> PolygonManager::getPoints() const
{
    return polygonPoints_;
}

void PolygonManager::setOutlineMode(bool isDrawOutline)
{
    isDrawOutlineMode_ = isDrawOutline;
}

QVector<LLA> PolygonManager::getPolygonOutlinePts()
{
    return polygonPoints_;
}

bool PolygonManager::getOutlineMode() const
{
    return isDrawOutlineMode_;
}

bool PolygonManager::isPointInPolygon(const QVector3D& point) const
{
    if (polygonPoints_.size() < 3) return true; // 空多边形或少于3个点，默认全部区域
    return pointInPolygon(point);
}

bool PolygonManager::isTriangleInPolygon(const QVector3D& A, const QVector3D& B, const QVector3D& C) const
{
    if (polygonPoints_.size() < 3) return true; // 空多边形或少于3个点，默认全部区域

    // 检查三角形的三个顶点是否都在多边形内
    return isPointInPolygon(A) && isPointInPolygon(B) && isPointInPolygon(C);
}

bool PolygonManager::pointInPolygon(const QVector3D& point) const
{
    bool inside = false;
    // int n = polygonPoints_.size();
    // for (int i = 0, j = n - 1; i < n; j = i++) {
    //     if (((polygonPoints_[i].y() > point.y()) != (polygonPoints_[j].y() > point.y())) &&
    //         (point.x() < (polygonPoints_[j].x() - polygonPoints_[i].x()) *
//(polygonPoints_.y() - polygonPoints_[i].y()) / (points_[j].y() - points_[i].y()) + points_[i].x())) {
    //         inside = !inside;
    //     }
    // }
    return inside;
}



QVector3D PolygonManager::convertLLAToNED(const LLA& lla) {
    // 这里需要实现 LLA（经纬度高度）到 NED（北东地）坐标系的转换
    // 具体实现取决于项目的坐标系统设置
    // 以下是一个简化的示例实现：

    // 假设参考点（原点）
    static const LLA originLLA(0.0, 0.0, 0.0);

    // 计算距离和方位角
    double distance = calculateDistance(originLLA, lla);
    double azimuth = calculateAzimuth(originLLA, lla);

    // 转换为 NED 坐标
    double north = distance * cos(azimuth);
    double east = distance * sin(azimuth);
    double down = -lla.altitude; // 高度取负值作为深度

    return QVector3D(north, east, down);
}

double PolygonManager::calculateDistance(const LLA& lla1, const LLA& lla2) {
    // 实现两点之间的距离计算（如 Haversine 公式）
    // 简化示例
    return sqrt(pow(lla1.latitude - lla2.latitude, 2) + pow(lla1.longitude - lla2.longitude, 2)) * 111000; // 粗略估算
}

double PolygonManager::calculateAzimuth(const LLA& lla1, const LLA& lla2) {
    // 实现方位角计算
    // 简化示例
    double dLon = lla2.longitude - lla1.longitude;
    double dLat = lla2.latitude - lla1.latitude;
    return atan2(dLon, dLat);
}
