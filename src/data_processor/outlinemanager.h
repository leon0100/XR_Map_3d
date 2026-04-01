#ifndef OUTLINEMANAGER_H
#define OUTLINEMANAGER_H

#include <QObject>
#include <QVector>
#include <QVector3D>
#include "dataset_defs.h"



class PolygonManager
{
public:
    PolygonManager() = default;
    ~PolygonManager() = default;

    void addPoint(const LLA& point);
    void clear();
    bool isEmpty() const;
    bool isPointInPolygon(const QVector3D& point) const;
    bool isTriangleInPolygon(const QVector3D& A, const QVector3D& B, const QVector3D& C) const;
    QVector<LLA> getPoints() const;

    bool getOutlineMode() const;
    void setOutlineMode(bool isDrawOutline);

    QVector<LLA> getPolygonOutlinePts();


    QVector3D convertLLAToNED(const LLA& lla);
    double calculateDistance(const LLA& lla1, const LLA& lla2);

    double calculateAzimuth(const LLA& lla1, const LLA& lla2);



private:
    bool pointInPolygon(const QVector3D& point) const;

    bool isDrawOutlineMode_ = true;
    QVector<LLA> polygonPoints_;     //保存多边形点
};



#endif // OUTLINEMANAGER_H
