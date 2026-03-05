#include "screetShot.h"


#include <QDebug>

#include <QDateTime>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>

#include "map_view.h"
#include "tile_google_provider.h"


/*----------------------------------------------ScreetShot---------------------------------------------*/
ScreetShot::ScreetShot(QWidget *parent) : QWidget{parent}
{
    this->hide();
}

QRectF ScreetShot::getSelectionRect() const
{
    return shotRect_;
}
void ScreetShot::setSelectionRect(const QRectF& rect)
{
    shotRect_ = rect;

    topWidth_ = getDistance_Haversine(topLeftLong_, topLeftLati_, topRightLong_, topRightLati_);
    rightHeight_ = getDistance_Haversine(topRightLong_, topRightLati_, bottomRightLong_, bottomRightLati_);

    QString topWidthStr = getLengthChEn(topWidth_);
    QString rightHeightStr = getLengthChEn(rightHeight_);

    setScreetWidth(topWidthStr);
    setScreetHeight(rightHeightStr);

    setScreetToolBar(false);

    emit selectionRectChanged();
}

bool ScreetShot::isSelectionRectVisible() const
{
    return isSelectionRectVisible_;
}
void ScreetShot::setSelectionRectVisible(bool visible)
{
    isSelectionRectVisible_ = visible;
    emit selectionRectVisibleChanged();
}

QString ScreetShot::getScreetWidth() const
{
    return screetWidth_;
}
void ScreetShot::setScreetWidth(const QString screetWidth)
{
    screetWidth_ = screetWidth;
    emit screetWidthChanged();
}
QString ScreetShot::getScreetHeight() const
{
    return screetHeight_;
}
void ScreetShot::setScreetHeight(const QString screetHeight)
{
    screetHeight_ = screetHeight;
    emit screetHeightChanged();
}

bool ScreetShot::getScreetToolBar() const
{
    return screetToolBarShow_;
}
void ScreetShot::setScreetToolBar(bool screetToolBarShow)
{
    screetToolBarShow_ = screetToolBarShow;
    emit screetToolBarShowChanged();
}

void ScreetShot::setMapView(const std::shared_ptr<MapView>& mapView)
{
    // mapView_ = mapView;
}


void ScreetShot::judgeResizeMode(const QRectF rect,const QPoint pos)
{
    const qreal margin = 15.0;
    bool onLeft   =  qAbs(pos.x() - rect.left())   <= margin;
    bool onRight  =  qAbs(pos.x() - rect.right())  <= margin;
    bool onTop    =  qAbs(pos.y() - rect.top())    <= margin;
    bool onBottom =  qAbs(pos.y() - rect.bottom()) <= margin;

    if (onLeft && onTop) {
        resizeMode_ = ResizeMode::TopLeft;
        QGuiApplication::setOverrideCursor(Qt::SizeFDiagCursor);
    }
    else if (onRight && onTop) {
        resizeMode_ = ResizeMode::TopRight;
        QGuiApplication::setOverrideCursor(Qt::SizeBDiagCursor);
    }
    else if (onLeft && onBottom) {
        resizeMode_ = ResizeMode::BottomLeft;
        QGuiApplication::setOverrideCursor(Qt::SizeBDiagCursor);
    }
    else if (onRight && onBottom) {
        resizeMode_ = ResizeMode::BottomRight;
        QGuiApplication::setOverrideCursor(Qt::SizeFDiagCursor);

    }
    else if (onLeft) {
        resizeMode_ = ResizeMode::Left;
        QGuiApplication::setOverrideCursor(Qt::SizeHorCursor);
    }
    else if (onRight) {
        resizeMode_ = ResizeMode::Right;
        QGuiApplication::setOverrideCursor(Qt::SizeHorCursor);
    }
    else if (onTop) {
        resizeMode_ = ResizeMode::Top;
        QGuiApplication::setOverrideCursor(Qt::SizeVerCursor);
    }
    else if (onBottom) {
        resizeMode_ = ResizeMode::Bottom;
        QGuiApplication::setOverrideCursor(Qt::SizeVerCursor);
    }
    else if(rect.contains(pos)){
        resizeMode_ = ResizeMode::Move;
        QGuiApplication::setOverrideCursor(Qt::SizeAllCursor);
    }
    else {
        resizeMode_ = ResizeMode::None;
        QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
    }

}

void ScreetShot::setToArrowCursor()
{
    QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
}

void ScreetShot::setCancelShot()
{
    qDebug() << "ScreetShot::setCancelShot().........";
    isScreenMode_ = false;
    screetToolBarShow_ = false;
    setSelectionRectVisible(false);
    emit cancelScreetShot();

}

void ScreetShot::getTitle(QRect rect, int level)
{
    // QPoint tl = Bing::pixelXYToTileXY(rect.topLeft());
    // QPoint br = Bing::pixelXYToTileXY(rect.bottomRight());

    // quint64 value = 0;
    // ImageInfo info;
    // info.z = level;

    // int max = qPow(2, level);   // 最大瓦片编号
    // for (int x = tl.x(); x <= br.x(); x++)
    // {
    //     if (x < 0)   continue;
    //     if (x >= max)  break;
    //     info.x = x;
    //     for (int y = tl.y(); y <= br.y(); y++)
    //     {
    //         if (y < 0)   continue;
    //         if (y >= max)  break;
    //         value = ((quint64) level << 48) + (x << 24) + y;
    //         if (!m_exist.contains(value))
    //         {
    //             info.y = y;
    //             m_infos.append(info);
    //         }
    //     }
    // }
}

void ScreetShot::getUrl()
{
    // for (int i = 0; i < m_infos.count(); i++)
    // {
    //     if(m_url == GEOVISEARTH_MAP_CN) {
    //         m_infos[i].url = m_url.arg(m_infos[i].z).arg(m_infos[i].x).arg(m_infos[i].y).arg(API_KEY_GEOVIS);
    //     }else if(m_url == OPENSTREET_MAP_CN){
    //         m_infos[i].url = m_url.arg(m_infos[i].z).arg(m_infos[i].x).arg(m_infos[i].y);
    //     } else {
    //         m_infos[i].url = m_url.arg(m_infos[i].x).arg(m_infos[i].y).arg(m_infos[i].z);
    //     }
    // }
}


// void ScreetShot::httpGetScreen(ImageInfo info)
// {
    // int retryCount = 0;
    // int tileTotalCnt = m_infos.count();
    // // qDebug() << "tileTotalCnt is " << tileTotalCnt;
    // interruptTile_ = false;
    // isScreenNetError_ = false;

    // while (retryCount < 5)
    // {
    //     QNetworkAccessManager manager;
    //     QNetworkRequest request;
    //     request.setUrl(QUrl(info.url));
    //     if(m_url == OPENSTREET_MAP_CN) {
    //         request.setRawHeader("User-Agent", "XR-Map/1.10 (Contact: 2376963887@qq.com)");
    //     }

    //     QSharedPointer<QNetworkReply> reply(manager.get(request));
    //     QEventLoop loop;
    //     QObject::connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    //     QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    //     loop.exec();

    //     if (reply->error() == QNetworkReply::NoError) {
    //         QByteArray buf = reply->readAll();
    //         if (!buf.isEmpty()) {
    //             info.img.loadFromData(buf);
    //             if (!info.img.isNull()) {
    //                 // emit GetInterface::getInterface()->update(info);
    //                 if(!m_itemGroup.contains(info.z))   // 如果图层不存在则添加
    //                 {
    //                     auto* item = new GraphItemGroup();
    //                     m_itemGroup.insert(info.z, item);
    //                     m_scene->addItem(item);
    //                 }

    //                 GraphItemGroup* itemGroup = m_itemGroup.value(info.z);
    //                 if (itemGroup) {
    //                     itemGroup->addImage(info);
    //                 }

    //                 m_screenTileCnt_++;
    //                 // emit updateProgressText(m_screenTileCnt_,tileTotalCnt);
    //                 return;
    //             }
    //         }

    //     } else {
    //         retryCount++;
    //         // qDebug() << "Retry:" << retryCount;
    //     }
    // }

    // if(retryCount >= 5 && !isScreenNetError_) {
    //     interruptTile_ = true;
    //     isScreenNetError_ = true;

    //     // emit showNetworkError();
    // }

// }

void ScreetShot::slot_downloadScreenFinished()
{
    // qDebug() << "watcher true 所有瓦片图像下载完成...." << m_completedDownloads_ << m_infos.count();
    // if(!interruptTile_) {
    //     // emit signalDrawScreenTrue();
    //     if(screenUserCancel_){
    //         return;
    //     }


    //     GraphItemGroup* itemGroup = nullptr;
    //     if (m_itemGroup.contains(currMapLevel_)) {
    //         itemGroup = m_itemGroup.value(currMapLevel_);
    //         itemGroup->show();
    //     }
    //     else {
    //         itemGroup = new GraphItemGroup();
    //         m_itemGroup.insert(currMapLevel_, itemGroup);
    //         m_scene->addItem(itemGroup);
    //     }

    //     QString baseDir = QCoreApplication::applicationDirPath();
    //     QDir dir(baseDir);
    //     dir.mkpath("screetTest");
    //     dir.cd("screetTest");
    //     if(!dir.exists()) {
    //         if (!dir.mkpath(".")) {
    //         }
    //     }

    //     QString bigFile = baseDir + "mosaic.png";
    //     QImage image(targetRect_.size(),QImage::Format_RGB16);
    //     if (image.isNull()) {
    //         qWarning() << "Failed to create QImage. Aborting rendering.";
    //         return;
    //     }

    //     {
    //         QPainter painter(&image);
    //         m_scene->render(&painter, QRectF(0, 0, targetRect_.width(), targetRect_.height()), targetRect_);
    //     }
    //     image.save(bigFile);
    //     qDebug() << "Big mosaic saved:" << bigFile;

    //     return;


    //     // 划分 targetRect_ 为小正方形
    //     double pixel1m = targetRect_.width() / topWidth_;
    //     // qDebug() << mapLevel << "级下， 1米的像素值: " << pixel1m << " " << targetRect_.height()/rightLen_;
    //     pixel300m_ = pixel1m*300;
    //     int rows = targetRect_.height() / pixel300m_;
    //     int cols = targetRect_.width()  / pixel300m_;

    //     int kmzCount = rows*cols;

    //     // objScreenProgress_->setProperty("infoText", tr("      XMAP Generating..."));
    //     // objScreenProgress_->setProperty("maxValue",kmzCount);
    //     // qmlScreenProgress_->show();
    //     // qmlScreenProgress_->raise();

    //     for (int row = 0; row < rows; ++row)
    //     {
    //         for (int col = 0; col < cols; ++col)
    //         {
    //             if(screenUserCancel_) {
    //                 generatedRectCount_ = kmzCount;
    //                 break;
    //             }
    //             QPointF topLeft(targetRect_.x()+col * pixel300m_, targetRect_.y()+row * pixel300m_);
    //             QPointF bottomRight(targetRect_.x()+(col+1) * pixel300m_, targetRect_.y()+(row+1) * pixel300m_);
    //             QRectF  square(topLeft, bottomRight);

    //             //          qDebug() << "Square:" << square.topLeft() << "->" << square.bottomRight();
    //             double topLeftLon,topLeftLati,btmRightLon,btmRightLati;
    //             Bing::pixelXYToLatLong(square.topLeft().toPoint(),currMapLevel_,topLeftLon,topLeftLati);
    //             Bing::pixelXYToLatLong(square.bottomRight().toPoint(),currMapLevel_,btmRightLon,btmRightLati);
    //             // if(isAmapSource_) {
    //             //     Mars2Wgs(topLeftLon,topLeftLati,&topLeftLon,&topLeftLati);
    //             //     Mars2Wgs(btmRightLon,btmRightLati,&btmRightLon,&btmRightLati);
    //             // }

    //             QString rowStr = QString::number(row + 1);
    //             QString colStr = QString::number(col + 1);
    //             QString kmzDir = baseDir + "/";
    //             QString imagPathName = kmzDir + rowStr + "_" + colStr + ".png";
    //             QString fileName = kmzDir + rowStr + "_" + colStr +".kml";
    //             QString imageName = rowStr + "_" + colStr + ".png";
    //             bool createKMl = createKmlFile(fileName,imageName,topLeftLati, btmRightLati,btmRightLon, topLeftLon);
    //             if(!createKMl) {
    //                 qDebug() << "createKMl failed";
    //                 return;
    //             }

    //             QImage image(square.toRect().size(),QImage::Format_RGB16);
    //             if (image.isNull()) {
    //                 qWarning() << "Failed to create QImage. Aborting rendering.";
    //                 return;
    //             }

    //             {
    //                 QPainter painter(&image);
    //                 m_scene->render(&painter, QRectF(0, 0, square.width(), square.height()), square);
    //             }
    //             image.save(imagPathName);
    //             QString tmpFileName = fileName;
    //             QString fileNameXmap = tmpFileName.replace("kml","xmap");
    //             createXMAPFile(fileName,imagPathName,fileNameXmap);

    //         }
    //     }


    // }
}


void ScreetShot::saveScreetShot()
{
    emit signalScreetGraphics();




    return;
    /*-暂时弃用XR-Map中的截图方式-*/

//     auto mapView = mapView_.lock();
//     if (!mapView) {
//         return;
//     }
//     double minLat = std::min({topLeftLati_, topRightLati_, bottomRightLati_});
//     double maxLat = std::max({topLeftLati_, topRightLati_, bottomRightLati_});
//     double minLon = std::min({topLeftLong_, topRightLong_, bottomRightLong_});
//     double maxLon = std::max({topLeftLong_, topRightLong_, bottomRightLong_});

//     map::TileGoogleProvider provider;
//     auto [lonStartTile, lonEndTile, boundaryTile] = provider.lonToTileXWithWrapAndBoundary(minLon, maxLon, currMapLevel_);
//     int yStart = provider.latToTileY(maxLat, currMapLevel_);
//     int yEnd   = provider.latToTileY(minLat, currMapLevel_);
//     int minY   = std::min(yStart, yEnd);
//     int maxY   = std::max(yStart, yEnd);

//     QVector<int> xIndices;
//     if (boundaryTile == -1) {
//         for (int x = lonStartTile; x <= lonEndTile; ++x) {
//             xIndices.append(x);
//         }
//     }
//     else {
//         for (int x = 0; x <= lonEndTile; ++x) {
//             xIndices.append(x);
//         }
//         for (int x = lonStartTile; x <= boundaryTile; ++x) {
//             xIndices.append(x);
//         }
//     }


//     qDebug() << "currMapLevel_ = " << currMapLevel_;
//     qDebug() << "Lat =" << minLat << "~" << maxLat << "   Lon =" << minLon << "~" << maxLon;

//     qDebug() << "lonStartTile =" << lonStartTile << "lonEndTile =" << lonEndTile << "boundaryTile =" << boundaryTile;
//     qDebug() << "yStart =" << yStart << "yEnd =" << yEnd;
//     qDebug() << "minY =" << minY << "maxY =" << maxY;

//     map::TileIndex firstTileIndx(xIndices[0], minY, currMapLevel_, provider.getProviderId());
//     auto firstTileInfo = provider.indexToTileInfo(firstTileIndx);
//     map::TileIndex lastTileIndx(xIndices.last(), maxY, currMapLevel_, provider.getProviderId());
//     auto lastTileInfo  = provider.indexToTileInfo(lastTileIndx);
//     double tileMinLat = lastTileInfo.bounds.south;
//     double tileMaxLat = firstTileInfo.bounds.north;
//     double tileMinLon = firstTileInfo.bounds.west;
//     double tileMaxLon = lastTileInfo.bounds.east;
//     qDebug() << "Tile coverage Lat: [" << tileMinLat << "," << tileMaxLat << "]";
//     qDebug() << "Tile coverage Lon: [" << tileMinLon << "," << tileMaxLon << "]";


// /*--------------------------------------------------------------------------------*/
//     int w = int(qPow(2, currMapLevel_)*256);
//     targetRect_ = QRect(0, 0, w, w);
//     QPoint topLeft = Bing::latLongToPixelXY(minLon, maxLat, currMapLevel_);
//     QPoint bottomRight = Bing::latLongToPixelXY(maxLon, minLat, currMapLevel_);
//     qDebug() << "minLon: " << minLon << "  " << maxLat << "  " << maxLon << "   " << minLat;
//     targetRect_.setTopLeft(topLeft);
//     targetRect_.setBottomRight(bottomRight);

//     if(m_scene == nullptr) {
//         m_scene = new QGraphicsScene();
//         // this->setScene
//     }
//     m_scene->setSceneRect(targetRect_);
//     if(targetRect_.isEmpty() || currMapLevel_ > 22 || currMapLevel_ < 0)  return;

//     if (m_future.isRunning()) {
//         m_future.cancel();
//     }
//     m_infos.clear();
//     getTitle(targetRect_, currMapLevel_);
//     getUrl();

//     isScreenNetError_ = false;
//     m_screenTileCnt_ = 0;

//     // 创建 QFutureWatcher 来监控并发任务
//     QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
//     connect(watcher, &QFutureWatcher<void>::finished, this, &ScreetShot::slot_downloadScreenFinished);
//     auto f = [this](ImageInfo& info) {
//         this->httpGetScreen(info);
//     };
//     m_future = QtConcurrent::map(m_infos,f);
//     watcher->setFuture(m_future);
}



qreal ScreetShot::clip(qreal n, qreal min, qreal max)
{
    n = qMax(n, min);
    n = qMin(n, max);
    return n;
}

qreal ScreetShot::clipLon(qreal lon)
{
    return clip(lon, -180.0, 180);
}

qreal ScreetShot::clipLat(qreal lat)
{
    return clip(lat, -85.05112878, 85.05112878);
}

uint ScreetShot::mapSize(int level)
{
    uint w = MAP_TIlE_SIZE;
    return (w << level);
}

//将一个点从纬度/经度WGS-84墨卡托坐标(以度为单位)转换为指定细节级别的像素XY坐标。
QPoint ScreetShot::latLongToPixelXY(qreal lon, qreal lat, int level)
{
    lon = clipLon(lon);
    lat = clipLat(lat);

    qreal x = (lon + 180) / 360;
    qreal sinLat = qSin(lat * M_PI / 180);
    qreal y = 0.5 - qLn((1 + sinLat) / (1 - sinLat)) / (4 * M_PI);

    uint size = mapSize(level);
    qreal pixelX = x * size + 0.5;
    pixelX = clip(pixelX, 0, size - 1);
    qreal pixelY = y * size + 0.5;
    pixelY = clip(pixelY, 0, size - 1);

    return QPoint(pixelX, pixelY);
}


//将像素从指定细节级别的像素XY坐标转换为经纬度WGS-84坐标(以度为单位)
void ScreetShot::pixelXYToLatLong(QPoint pos, int level, qreal& lon, qreal& lat)
{
    uint size = mapSize(level);
    qreal x = (clip(pos.x(), 0, size - 1) / size) - 0.5;
    qreal y = 0.5 - (clip(pos.y(), 0, size - 1) / size);
    lon = x * 360;
    lat = 90 - (360 * qAtan(qExp(-y*2*M_PI)) / M_PI);
}
void ScreetShot::pixelXYToLatLong(QPointF pos, int level, qreal& lon, qreal& lat)
{
    uint size = mapSize(level);
    qreal x = (clip(pos.x(), 0, size - 1) / size) - 0.5;
    qreal y = 0.5 - (clip(pos.y(), 0, size - 1) / size);
    lon = x * 360;
    lat = 90 - (360 * qAtan(qExp(-y*2*M_PI)) / M_PI);
}


void ScreetShot::resizeMode(QRectF& rect, const QPoint pos)
{
    QPointF delta;

    switch (resizeMode_) {
    case ResizeMode::Top: {
        qreal newTop = pos.y();
        if (newTop > rect.bottom()) {
            rect.setTop(rect.bottom());
            rect.setBottom(newTop);
        } else {
            rect.setTop(newTop);
        }
        break;
    }
    case ResizeMode::Bottom: {
        qreal newBottom = pos.y();
        if (newBottom < rect.top()) {
            rect.setBottom(rect.bottom());
            rect.setTop(newBottom);
        } else {
            rect.setBottom(newBottom);
        }
        break;
    }
    case ResizeMode::Left: {
        qreal newLeft = pos.x();
        if (newLeft > rect.right()) {
            rect.setLeft(rect.left());
            rect.setRight(newLeft);
        } else {
            rect.setLeft(newLeft);
        }
        break;
    }
    case ResizeMode::Right: {
        qreal newRight = pos.x();
        if (newRight < rect.left()) {
            rect.setRight(rect.right());
            rect.setLeft(newRight);
        } else {
            rect.setRight(newRight);
        }
        break;
    }
    case ResizeMode::TopLeft: {
        qreal newTop = pos.y();
        qreal newLeft = pos.x();

        if (newTop > rect.bottom()) {
            qreal oldTop = rect.top();
            rect.setTop(newTop);
            rect.setBottom(oldTop);
        } else {
            rect.setTop(newTop);
        }

        if (newLeft > rect.right()) {
            qreal oldLeft = rect.left();
            rect.setLeft(newLeft);
            rect.setRight(oldLeft);
        } else {
            rect.setLeft(newLeft);
        }
        break;
    }
    case ResizeMode::TopRight: {
        qreal newTop = pos.y();
        qreal newRight = pos.x();

        if (newTop > rect.bottom()) {
            qreal oldTop = rect.top();
            rect.setTop(newTop);
            rect.setBottom(oldTop);
        } else {
            rect.setTop(newTop);
        }

        if (newRight < rect.left()) {
            qreal oldRight = rect.right();
            rect.setRight(newRight);
            rect.setLeft(oldRight);
        } else {
            rect.setRight(newRight);
        }
        break;
    }
    case ResizeMode::BottomLeft: {
        qreal newBottom = pos.y();
        qreal newLeft = pos.x();

        if (newBottom < rect.top()) {
            qreal oldBottom = rect.bottom();
            rect.setBottom(newBottom);
            rect.setTop(oldBottom);
        } else {
            rect.setBottom(newBottom);
        }

        if (newLeft > rect.right()) {
            qreal oldLeft = rect.left();
            rect.setLeft(newLeft);
            rect.setRight(oldLeft);
        } else {
            rect.setLeft(newLeft);
        }
        break;
    }
    case ResizeMode::BottomRight: {
        qreal newBottom = pos.y();
        qreal newRight = pos.x();

        if (newBottom < rect.top()) {
            qreal oldBottom = rect.bottom();
            rect.setBottom(newBottom);
            rect.setTop(oldBottom);
        } else {
            rect.setBottom(newBottom);
        }

        if (newRight < rect.left()) {
            qreal oldRight = rect.right();
            rect.setRight(newRight);
            rect.setLeft(oldRight);
        } else {
            rect.setRight(newRight);
        }
        break;
    }
    case ResizeMode::Move: {
        delta = QPointF(pos) - endPos_;
        endPos_ = QPointF(pos);
        rect.translate(delta);
        break;
    }
    default:
        break;
    }
}


double ScreetShot::getDistance_Haversine(double current_longi, double current_lati, double goal_longi, double goal_lati)
{
    double dLat = (goal_lati - current_lati) * _PI_180;
    double dLon = (goal_longi - current_longi) * _PI_180;
    double a = pow(sin(dLat/2), 2) + cos(current_lati*_PI_180) * cos(goal_lati*_PI_180) * pow(sin(dLon/2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    return EARTH_RADIUS * c;
}

QString ScreetShot::getLengthChEn(double distance,int decimalPlaces)
{
    QString distanceStr;
    // bool isMetres = ContourSingleton::getInstance().getGlobalUnits();
    bool isMetres = true;
    if (distance > 1000) {
        double distanceKm = distance / 1000.0;
        distanceStr = isMetres ? (QString::number(distanceKm,'f',decimalPlaces)+" km") :
                          (QString::number(distanceKm*0.621371,'f',decimalPlaces)+" mi");
    } else {
        double distanceFeet = distance * 3.28084;
        distanceStr = isMetres ? (QString::number(distance,'f',0)+" m") : (QString::number(distanceFeet,'f',0)+" ft");
    }

    return distanceStr;
}



bool ScreetShot::createKmlFile(QString kmlPath,QString imageName,double north,double south,double east,double west)
{
    // QFile file(kmlPath);
    // if (!file.open(QFile::WriteOnly | QFile::Text)) {
    //     qDebug() << QString("Cannot write file %1.").arg(file.errorString());
    //     return false;
    // }

    // QXmlStreamWriter writer(&file);
    // writer.setCodec("UTF-8");
    // writer.setAutoFormatting(true);
    // writer.writeStartDocument("1.0", true);

    // writer.writeStartElement("kml");
    // writer.writeAttribute("xmlns", "http://www.opengis.net/kml/2.2");

    // writer.writeStartElement("Document");
    // writer.writeAttribute("id", "root_doc");

    // writer.writeStartElement("GroundOverlay");
    // writer.writeTextElement("name", "Shaded Relief");

    // writer.writeStartElement("Icon");
    // writer.writeTextElement("href", imageName);
    // writer.writeEndElement(); // Icon

    // writer.writeStartElement("LatLonBox");
    // writer.writeTextElement("north", QString::number(north, 'f', 14));
    // writer.writeTextElement("south", QString::number(south, 'f', 14));
    // writer.writeTextElement("east", QString::number(east, 'f', 14));
    // writer.writeTextElement("west", QString::number(west, 'f', 14));
    // writer.writeEndElement(); // LatLonBox

    // writer.writeEndElement(); // GroundOverlay

    // writer.writeEndElement(); // Document
    // writer.writeEndElement(); // kml

    // writer.writeEndDocument();
    // file.close();

    return true;
}



bool ScreetShot::createXMAPFile(const QString kmlFilePath, const QString imageFilePath, QString &outputXMAPPath)
{
    // QFile kmlFile(kmlFilePath);
    // if (!kmlFile.exists()) {
    //     qDebug() << "KML file does not exist.";
    //     return false;
    // }
    // QFile imageFile(imageFilePath);
    // if (!imageFile.exists()) {
    //     qDebug() << "Image file does not exist.";
    //     return false;
    // }

    // // 1、打开KML文件并读取内容
    // if (!kmlFile.open(QIODevice::ReadOnly)) {
    //     qDebug() << "Failed to open KML file.";
    //     return false;
    // }
    // QByteArray kmlData = kmlFile.readAll();
    // if(!kmlFile.remove()) {
    //     qDebug() << "kmlFile remove failed";
    //     return false;
    // }

    // // 2、打开图片文件并读取内容
    // if (!imageFile.open(QIODevice::ReadOnly)) {
    //     qDebug() << "Failed to open image file.";
    //     return false;
    // }
    // QByteArray imageData = imageFile.readAll();
    // //nie:test
    // // if(!imageFile.remove()) {
    // //     qDebug() << "Failed to delete image file.";
    // //     return false;
    // // }

    // //3、创建KMZ文件
    // outputXMAPPath.replace(".xmap",".kmz");
    // QZipWriter kmzWriter(outputXMAPPath);
    // kmzWriter.addFile("doc.kml", kmlData);
    // QString imageFileName1 = QFileInfo(imageFilePath).fileName();
    // kmzWriter.addFile(imageFileName1, imageData);
    // kmzWriter.close();
    // if (kmzWriter.status() != QZipWriter::NoError) {
    //     qDebug() << "Failed to create KMZ file.";
    //     return false;
    // }

    return true;
}
