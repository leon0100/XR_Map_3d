#include "screetShot.h"


#include <QDebug>

#include <QDateTime>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <private/qzipwriter_p.h>
#include <QFileDialog>



#include "map_view.h"


/*----------------------------------------------ScreetShot---------------------------------------------*/
ScreetShot::ScreetShot(QWidget *parent) : QWidget{parent}
{
    this->hide();
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
    qDebug() << "ScreetShot::setCancelShot().........";
    isScreenMode_ = false;
    screetToolBarShow_ = false;
    setSelectionRectVisible(false);
    emit cancelScreetShot();
}


void ScreetShot::saveScreetShot()
{
    // 判断是否需要调整框选区域
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
        GIF->dialogCheck(tr("Confirm to Clear Isobaths?"),[this](bool confirmed, bool reminder) {
            if(confirmed) {
                isReminderChecked_ = reminder;

            }
        }, tr("Don't prompt again!"));
    } else {
        doSaveMapLevelProcess();
    }





    emit signalScreetGraphics();
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
        openloadQia_->close();
        if(!openMapLevelList_) {
            judgeLevelCount_ = 13;
            GIF->dialogInfo(Dialog_OK,tr("Loading failed, please check your network connection or try again!"));
        }
    });
    timeoutTimer->start(7000);
    openloadQia_->show();

    judgeLevelCount_ = (currLevel_ > 13) ? currLevel_ : 13;
    mapLevelChoose_->resetIsExistTileVec();

    judgeCurrentLevelExist(startLon_,startLati_,judgeLevelCount_);
}
