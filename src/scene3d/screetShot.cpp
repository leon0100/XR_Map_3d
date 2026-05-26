#include "screetShot.h"


#include <QDebug>

#include <QDateTime>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <private/qzipwriter_p.h>
#include <QFileDialog>


// #include "map_view.h"


/*----------------------------------------------ScreetShot---------------------------------------------*/
ScreetShot::ScreetShot(QObject *parent) : QObject{parent}
{
    loadingQuickView_ = new QQuickView();
    loadingQuickView_->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    loadingQuickView_->setSource(QUrl("qrc:/Bluetooth/loading.qml"));
    loadingQuickView_->setResizeMode(QQuickView::SizeRootObjectToView);
    loadingQuickView_->resize(75, 75);
    QMetaObject::invokeMethod(loadingQuickView_, [this]() { loadingQuickView_->hide(); }, Qt::QueuedConnection);

}

QRectF ScreetShot::getSelectionRect() const
{
    return shotRect_;
}

void ScreetShot::setSelectionRect(const QRectF rect)
{
    shotRect_ = rect;

    // topWidth_    = getDistance_Haversine(topLeftLong_, topLeftLati_, topRightLong_, topRightLati_);
    // rightHeight_ = getDistance_Haversine(topRightLong_, topRightLati_, bottomRightLong_, bottomRightLati_);
    LLA topLeftLla(topLeftLati_, topLeftLong_, 0.0);
    North_East_Down topLeftNed(&topLeftLla, &viewLlaRef_, false);
    LLA bottomRightLla(bottomRightLati_, bottomRightLong_, 0.0);
    North_East_Down bottomRightNed(&bottomRightLla, &viewLlaRef_, false);
    topWidth_    = std::abs(bottomRightNed.e - topLeftNed.e); // 宽度（米）
    rightHeight_ = std::abs(bottomRightNed.n - topLeftNed.n); // 高度（米）

    // qDebug() << "topWidth_:" << topWidth_ << "   rightHeight_:" << rightHeight_;


    QString topWidthStr    = getLengthChEn(topWidth_);
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

void ScreetShot::setLLARef(LLARef viewLlaRef)
{
    viewLlaRef_ = viewLlaRef;
}

float ScreetShot::mapLevelToDistance(int level) const
{
    switch (level)
    {
        case 10:  return 72558.6f;
        case 11:  return 36074.4f;
        case 12:  return 17935.4f;
        case 13:  return 10254.6f;
        case 14:  return 5098.35f;
        case 15:  return 2534.78f;
        case 16:  return 1260.24f;
        case 17:  return 626.56f;
        case 18:  return 311.51f;
        case 19:  return 178.11f;
        case 20:  return 88.55f;
        case 21:  return 58.22f;
        default:  break;
    }

    return 1000.0f;
}

bool ScreetShot::createKmlFile(QString kmlPath,QString imageName,double north,double south,double east,double west)
{
    QFile file(kmlPath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        qDebug() << QString("Cannot write file %1.").arg(file.errorString());
        return false;
    }
    QXmlStreamWriter writer(&file);

    writer.setCodec("UTF-8");
    writer.setAutoFormatting(true);
    writer.writeStartDocument("1.0", true);

    writer.writeStartElement("kml");
    writer.writeAttribute("xmlns", "http://www.opengis.net/kml/2.2");

    writer.writeStartElement("Document");
    writer.writeAttribute("id", "root_doc");

    writer.writeStartElement("GroundOverlay");
    writer.writeTextElement("name", "Shaded Relief");

    writer.writeStartElement("Icon");
    writer.writeTextElement("href", imageName);
    writer.writeEndElement(); // Icon

    writer.writeStartElement("LatLonBox");
    writer.writeTextElement("north", QString::number(north, 'f', 14));
    writer.writeTextElement("south", QString::number(south, 'f', 14));
    writer.writeTextElement("east", QString::number(east, 'f', 14));
    writer.writeTextElement("west", QString::number(west, 'f', 14));
    writer.writeEndElement(); // LatLonBox

    writer.writeEndElement(); // GroundOverlay

    writer.writeEndElement(); // Document
    writer.writeEndElement(); // kml

    writer.writeEndDocument();
    file.close();

    return true;
}

bool ScreetShot::createXMAPFile(const QString kmlFilePath, const QString imageFilePath, QString outputXMAPPath)
{
    QFile kmlFile(kmlFilePath);
    if (!kmlFile.exists()) {
        qDebug() << "KML file does not exist.";
        return false;
    }
    QFile imageFile(imageFilePath);
    if (!imageFile.exists()) {
        qDebug() << "Image file does not exist.";
        return false;
    }

    // 1、打开KML文件并读取内容
    if (!kmlFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open KML file.";
        return false;
    }
    QByteArray kmlData = kmlFile.readAll();
    if(!kmlFile.remove()) {
        qDebug() << "kmlFile remove failed";
        return false;
    }

    // 2、打开图片文件并读取内容
    if (!imageFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open image file.";
        return false;
    }
    QByteArray imageData = imageFile.readAll();
    if(!imageFile.remove()) {
        qDebug() << "Failed to delete image file.";
        return false;
    }

    // 3、创建KMZ文件
    QString imageFileName1 = QFileInfo(imageFilePath).fileName();
    QString kmzPath = outputXMAPPath + ".kmz";
    QZipWriter kmzWriter(kmzPath);
    kmzWriter.addFile("doc.kml", kmlData);
    kmzWriter.addFile(imageFileName1, imageData);
    kmzWriter.close();
    if (kmzWriter.status() != QZipWriter::NoError) {
        qDebug() << "[ERROR] Failed to create KMZ file.";
        return false;
    }

    return true;
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
    isScreenMode_ = false;
    screetToolBarShow_ = false;
    setSelectionRectVisible(false);
    emit cancelScreetShot();
}


void ScreetShot::saveScreetShot()
{
    screetToolBarShow_ = false;
    setSelectionRectVisible(false);
    emit cancelScreetShot();

    const double MIN_SIZE = 900.0;
    if (topWidth_ < MIN_SIZE || rightHeight_ < MIN_SIZE) {
        double screen_1m = shotRect_.width() / topWidth_;
        qDebug() << "screen_1m.........." << screen_1m;
        shotRect_.setWidth(screen_1m * 900);
        shotRect_.setHeight(screen_1m * 900);

        topWidth_    = MIN_SIZE;
        rightHeight_ = MIN_SIZE;

        setScreetWidth("900m");
        setScreetHeight("900m");

        emit selectionRectChanged();

        GIF->dialogInfo(Dialog_OK,tr("The selected range is less than 900m*900m,Automatically resized!"));
        return;
    }


    if(!isReminderChecked_) {
        GIF->dialogCheck2(tr("Please save the folder to path:\"SD:\\XR5XX\\Map\""), [this](bool ok, bool reminder) {
            if(reminder) {
                isReminderChecked_ = reminder;
            }

            if(ok) {
                doSaveMapLevelProcess();
            }

        }, tr("Don't prompt again"));
    }
    else {
        doSaveMapLevelProcess();
    }
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

void ScreetShot::switchMapSource(MapSourceType sourceType)
{
    currentMap_ = sourceType;
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

void ScreetShot::doSaveMapLevelProcess()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timeDir = "XMap" + currentDateTime.toString("yyyy-MM-dd_HH-mm-ss");
#ifdef Q_OS_WIN
    QString fullPath = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save File"), timeDir);
    if (fullPath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(fullPath);
    targetDirPath_ = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName();
#elif defined(Q_OS_ANDROID)
    // 调用 Java 方法打开 SAF 文件选择器
    QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniObject dirName = QAndroidJniObject::fromString(timeDir);
    QAndroidJniObject operationType = QAndroidJniObject::getStaticObjectField(
        "com/nqc/FileQtActivity$OperationType","CREATE_FOLDER", "Lcom/nqc/FileQtActivity$OperationType;");
    QAndroidJniObject::callStaticMethod<void>(
        "com/nqc/FileQtActivity", "openDirectoryFromQt",
        "(Ljava/lang/String;Lcom/nqc/FileQtActivity$OperationType;)V",dirName.object<jstring>(), operationType.object());

#endif
    //    qDebug() << "targetDirPath_ " << targetDirPath_;

    openMapLevelList_ = false;
    QTimer* timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, [this]() {
        QMetaObject::invokeMethod(loadingQuickView_, [this]() { loadingQuickView_->hide(); }, Qt::QueuedConnection);
        if(!openMapLevelList_) {
            judgeLevelCount_ = 13;
            GIF->dialogInfo(Dialog_OK,tr("Loading failed, please check your network connection or try again!"));
        }
    });
    timeoutTimer->start(7000);
    QMetaObject::invokeMethod(loadingQuickView_, [this](){ loadingQuickView_->show(); }, Qt::QueuedConnection);
    judgeLevelCount_ = (currMapLevel_ > 13) ? currMapLevel_ : 13;

    judgeCurrentLevelExist(topLeftLong_, topLeftLati_,judgeLevelCount_);
}

bool ScreetShot::isMapLevelChooseVisible() const
{
    return isMapLevelChooseVisible_;
}

void ScreetShot::setMapLevelChooseVisible(bool visible)
{
    isMapLevelChooseVisible_ = visible;
    emit mapLevelChooseVisibleChanged();
}

void ScreetShot::setTargetMapLevel(int level)
{
    targetMapLevel_ = level;
    emit signalScreetGraphics();
}

int ScreetShot::getTargetMapLevel()
{
    return targetMapLevel_;
}

QString ScreetShot::getTargetDirPath()
{
    return targetDirPath_;
}

void ScreetShot::judgeCurrentLevelExist(double longitude,double latitude,int level)
{
    QString m_url;
    if(currentMap_ == MapSourceType::amapMapSource){
        m_url = amapMap;
    }
    else {
        m_url = googleMap;
    }

    jude_infos.clear();
    for(int z = level; z <= 18; z++) {
        QPoint tilePos = latLongToTileXY(longitude, latitude, z);
        ImageInfo info;
        info.x   = tilePos.x();
        info.y   = tilePos.y();
        info.z   = z;
        info.url = m_url.arg(info.x).arg(info.y).arg(info.z);
        jude_infos.append(info);
    }

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, &ScreetShot::slot_judgeLevelExist);
    auto f = [](ImageInfo& info) {
        QNetworkAccessManager manager;
        while (info.count < 3) {
            QSharedPointer<QNetworkReply> reply(manager.get(QNetworkRequest(QUrl(info.url))));
            QEventLoop loop;
            QObject::connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
            QTimer::singleShot(5000, &loop, &QEventLoop::quit);
            loop.exec();

            if (reply->error() == QNetworkReply::NoError) {
                info.isValidTile = true;
            }

            reply->deleteLater();
            info.count++;
            break;
        }
    };

    QFuture<void> m_future = QtConcurrent::map(jude_infos,f);
    watcher->setFuture(m_future);
}

void ScreetShot::slot_judgeLevelExist()
{
    openMapLevelList_ = true;
    QMetaObject::invokeMethod(loadingQuickView_, [this]() {loadingQuickView_->hide(); }, Qt::QueuedConnection);

    setDataStatistics(topWidth_,rightHeight_);

    setMapLevelChooseVisible(true);
}

void ScreetShot::setDataStatistics(double widthLen,double heightLen)
{
    int rows = widthLen/300;
    int cols = heightLen/300;

    int totalSquareS = rows*cols;

    QString size;
    if(qMin(widthLen,heightLen) > 1000) {
        size = QString::number(widthLen/1000.0,'f',2) + "km * " + QString::number(heightLen/1000.0,'f',2) + "km";
    } else {
        size = QString::number(widthLen) + "m x " + QString::number(heightLen) + "m";
    }

    RowData theorSizeDownTime_13 = getTheorSizeDownloadTime(theorSize_13,downTime_13,totalSquareS);
    emit updateTableRowData(0, size, theorSizeDownTime_13.theoreticalSize, theorSizeDownTime_13.downloadTime);

    RowData theorSizeDownTime_14 = getTheorSizeDownloadTime(theorSize_14,downTime_14,totalSquareS);
    emit updateTableRowData(1,size,theorSizeDownTime_14.theoreticalSize,theorSizeDownTime_14.downloadTime);

    RowData theorSizeDownTime_15 = getTheorSizeDownloadTime(theorSize_15,downTime_15,totalSquareS);
    emit updateTableRowData(2,size,theorSizeDownTime_15.theoreticalSize,theorSizeDownTime_15.downloadTime);

    RowData theorSizeDownTime_16 = getTheorSizeDownloadTime(theorSize_16,downTime_16,totalSquareS);
    emit updateTableRowData(3,size,theorSizeDownTime_16.theoreticalSize,theorSizeDownTime_16.downloadTime);

    RowData theorSizeDownTime_17 = getTheorSizeDownloadTime(theorSize_17,downTime_17,totalSquareS);
    emit updateTableRowData(4,size,theorSizeDownTime_17.theoreticalSize,theorSizeDownTime_17.downloadTime);

    RowData theorSizeDownTime_18 = getTheorSizeDownloadTime(theorSize_18,downTime_18,totalSquareS);
    emit updateTableRowData(5,size,theorSizeDownTime_18.theoreticalSize,theorSizeDownTime_18.downloadTime);

}

RowData ScreetShot::getTheorSizeDownloadTime(quint64 theorSize,quint64 downTime,quint64 totalSmallSquare)
{
    RowData theorSizeDownTime;

    QString theoreticalSize,downloadTime;
    quint64 theoriSize = theorSize*totalSmallSquare;

    if (theoriSize < 1024) {
        theoreticalSize =  QString::number(theoriSize) + "KB";
    }
    else if (theoriSize < 1024 * 1024) {
        theoreticalSize = QString::number(theoriSize / 1024.0, 'f', 1) + "MB";
    }
    else {
        theoreticalSize = QString::number(theoriSize / (1024.0 * 1024), 'f', 2) + "G";
    }

    quint64 loadTime = (downTime*totalSmallSquare)/1000;
    if(loadTime < 1) {
        downloadTime = "1s";
    }
    else if ((loadTime < 60) && (loadTime >= 1)) {
        downloadTime = QString::number(loadTime) + "s";
    }
    else if (loadTime < 3600) {
        quint64 minutes = loadTime / 60;
        quint64 seconds = loadTime % 60;
        downloadTime = QString::number(minutes) + "min" + QString::number(seconds) + "s";
    }
    else {
        quint64 hours = loadTime / 3600;
        quint64 minutes = (loadTime % 3600) / 60;
        downloadTime = QString::number(hours) + "hour" + QString::number(minutes) + "min";
    }

    theorSizeDownTime.theoreticalSize = theoreticalSize;
    theorSizeDownTime.downloadTime = downloadTime;

    return theorSizeDownTime;
}


QPoint ScreetShot::latLongToTileXY(qreal lon, qreal lat, int level)
{
    lon = clip(lon, -180.0, 180);
    lat = clip(lat, -85.05112878, 85.05112878);

    qreal x = (lon + 180) / 360;
    qreal sinLat = qSin(lat * M_PI / 180);
    qreal y = 0.5 - qLn((1 + sinLat) / (1 - sinLat)) / (4 * M_PI);

    uint size = MAP_TIlE_SIZE << level;
    qreal pixelX = x * size + 0.5;
    pixelX = clip(pixelX, 0, size - 1);
    qreal pixelY = y * size + 0.5;
    pixelY = clip(pixelY, 0, size - 1);

    int tileX = pixelX / MAP_TIlE_SIZE;
    int tileY = pixelY / MAP_TIlE_SIZE;
    return QPoint(tileX, tileY);
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


