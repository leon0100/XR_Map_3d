#ifndef SCREETSHOT_H
#define SCREETSHOT_H

#include <QObject>
#include <QPoint>
#include <QGuiApplication>
#include <QRectF>
#include <QVector3D>
#include <QCursor>
#include <QtConcurrent>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QGraphicsScene>
#include <QQuickView>

#include "dataset_defs.h"
#include "console.h"
// #include <QtGui/private/qzipreader_p.h>
// #include <QtGui/private/qzipwriter_p.h>



#define FILE_PASSWORD "$Toslon&85359189@Yt"
constexpr double TILE_CONSTANT = 126543000.03392;

enum class ResizeMode { None, Move, Top, Bottom, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };

//KB
typedef enum
{
    theorSize_13 = 5,
    theorSize_14 = 9,
    theorSize_15 = 20,
    theorSize_16 = 40,
    theorSize_17 = 150,
    theorSize_18 = 400,
    theorSize_19 = 750,
    theorSize_20 = 1300
}theoreticalSize;

//ms
typedef enum
{
    downTime_13 = 56,
    downTime_14 = 100,
    downTime_15 = 120,
    downTime_16 = 140,
    downTime_17 = 160,
    downTime_18 = 240,
    downTime_19 = 500,
    downTime_20 = 600
}downloadTime;


struct RowData {
    QString size;
    QString theoreticalSize;
    QString downloadTime;
};


// 截图任务
struct ScreenshotTask
{
    int     mapLevel;
    double  minLat;
    double  maxLat;
    double  minLon;
    double  maxLon;
    double  geoWidth;
    double  geoHeight;
    QString outputPath;

    ScreenshotTask() : mapLevel(0), minLat(0), maxLat(0), minLon(0), maxLon(0),geoWidth(0), geoHeight(0)
    {}

    ScreenshotTask(int level, double minL, double maxL, double minLn, double maxLn,
                   double geoW, double geoH, const QString& path)
        : mapLevel(level), minLat(minL), maxLat(maxL), minLon(minLn), maxLon(maxLn),
            geoWidth(geoW), geoHeight(geoH), outputPath(path)
    {}
};








/*------------------------------------------ScreetShot---------------------------------------------*/

class MapView;
class ScreetShot : public QObject
{
    Q_OBJECT
public:
    explicit ScreetShot(QObject *parent = nullptr);

    // 添加矩形属性
    Q_PROPERTY(QRectF selectionRect READ getSelectionRect WRITE setSelectionRect NOTIFY selectionRectChanged)
    Q_PROPERTY(bool isSelectionRectVisible READ isSelectionRectVisible WRITE setSelectionRectVisible NOTIFY selectionRectVisibleChanged)
    Q_PROPERTY(QString screetWidth  READ getScreetWidth   WRITE setScreetWidth NOTIFY screetWidthChanged)
    Q_PROPERTY(QString screetHeight READ getScreetHeight  WRITE  setScreetHeight NOTIFY screetHeightChanged)
    Q_PROPERTY(bool screetToolBar   READ getScreetToolBar WRITE setScreetToolBar NOTIFY screetToolBarShowChanged)
    Q_PROPERTY(bool isMapLevelChooseVisible READ isMapLevelChooseVisible WRITE setMapLevelChooseVisible NOTIFY mapLevelChooseVisibleChanged)


    QRectF getSelectionRect() const;
    void setSelectionRect(const QRectF rect);

    bool isSelectionRectVisible() const;
    void setSelectionRectVisible(bool visible);

    QString getScreetWidth() const;
    void setScreetWidth(const QString screetWidth);
    QString getScreetHeight() const;
    void setScreetHeight(const QString screetHeight);

    bool getScreetToolBar() const;
    void setScreetToolBar(bool screetToolBarShow);

    float mapLevelToDistance(int level) const;

    void setLLARef(LLARef viewLlaRef);

    bool isMapLevelChooseVisible() const;
    void setMapLevelChooseVisible(bool visible);

    static bool createKmlFile(QString kmlPath,QString imageName,double north,double south,double east,double west);
    static bool createXMAPFile(const QString kmlFilePath, const QString imageFilePath, QString outputXMAPPath);
    static void writeBoundaryFile(const QString& folderPath, double north, double south, double east, double west, quint8 level);
    static void menu_renewMap(QString savePath);


signals:
    void selectionRectChanged();
    void selectionRectVisibleChanged();
    void screetWidthChanged();
    void screetHeightChanged();
    void screetToolBarShowChanged();
    void cancelScreetShot();
    void signalScreetGraphics();
    void mapLevelChooseVisibleChanged();
    void updateTableRowData(int row, const QString& size, const QString& theoreticalSize, const QString& downloadTime);


public:
    void judgeResizeMode(const QRectF rect,const QPoint pos);
    void resizeMode(QRectF& rect,const QPoint pos);
    double getDistance_Haversine(double current_longi, double current_lati, double goal_longi, double goal_lati);
    void switchMapSource(MapSourceType sourceType);
    int getTargetMapLevel();
    QString getTargetDirPath();

    Q_INVOKABLE void setToArrowCursor();
    Q_INVOKABLE void setCancelShot();
    Q_INVOKABLE void saveScreetShot();
    Q_INVOKABLE void setTargetMapLevel(int level);


private:
    QString getLengthChEn(double distance,int decimalPlaces = 2);
    void doSaveMapLevelProcess();
    void judgeCurrentLevelExist(double longitude,double latitude,int level);
    QPoint latLongToTileXY(qreal lon, qreal lat, int level);   // 经纬度转瓦片编号
    qreal clip(qreal n, qreal min, qreal max);
    qreal clipLon(qreal lon);   // 裁剪经度范围
    qreal clipLat(qreal lat);   // 裁剪纬度范围
    void slot_judgeLevelExist();
    void setDataStatistics(double widthLen,double heightLen);
    RowData getTheorSizeDownloadTime(quint64 theorSize,quint64 downTime,quint64 totalSmallSquare);

    static QByteArray readKmlFromKmz(const QString& kmzPath);


public:
    // QQuickView* loadingQuickView_;
    bool isScreenMode_ = false;      // 截图模式
    bool isScreenSaveMode_ = false;  //截图后的保存模式
    bool m_moveView = false;         // 鼠标移动地图
    bool showHistoryScreen_ = false; // 显示历史截图
    bool firstScreenDown_ = false;
    QPointF startPos_,endPos_;       // 矩形场景坐标
    ResizeMode resizeMode_;
    int currMapLevel_ = 0;
    bool dragging_ = false;
    double topLeftLong_, topLeftLati_, topRightLong_, topRightLati_, bottomRightLong_, bottomRightLati_;
    bool screetToolBarShow_ = false;
    QRectF shotRect_;
    bool isSelectionRectVisible_ = false;
    QString screetWidth_, screetHeight_;
    bool isMapLevelChooseVisible_ = false;


private:
    double topWidth_,rightHeight_;
    LLARef viewLlaRef_;
    bool isReminderChecked_ = false;
    QString targetDirPath_;
    bool openMapLevelList_ = false;

    u8 judgeLevelCount_ = 13;

    bool screenshotPending_ = false;
    QMutex screenshotMutex_;

    MapSourceType currentMap_ = googleMapSource;
    QString googleMap = "http://mt2.google.com/vt/lyrs=y&hl=en&x=%1&y=%2&z=%3";
    QString amapMap = "http://wprd04.is.autonavi.com/appmaptile?style=6&x=%1&y=%2&z=%3";
    QVector<ImageInfo> jude_infos;
    int targetMapLevel_;











/*---------------------------------DistMeasure---------------------------------*/
public:
    Q_PROPERTY(bool isDistMeasureVisible READ getDistMeasureVisible WRITE setDistMeasureVisible NOTIFY distMeasureVisibleChanged)
    Q_PROPERTY(QLineF distLine READ getDistLine WRITE setDistLine NOTIFY distLineChanged)
    Q_PROPERTY(QPointF distLineP1 READ getDistLineP1 WRITE setDistLineP1 NOTIFY distLineChanged)
    Q_PROPERTY(QPointF distLineP2 READ getDistLineP2 WRITE setDistLineP2 NOTIFY distLineChanged)



    bool getDistMeasureVisible() const;
    void setDistMeasureVisible(bool visible);

    QLineF getDistLine() const;
    void setDistLine(const QLineF line);

    void setDistLineStart(QPointF start);
    void setDistLineEnd(QPointF end);

    QPointF getDistLineP1() const;
    void setDistLineP1(const QPointF p1);

    QPointF getDistLineP2() const;
    void setDistLineP2(const QPointF p2);





public:
    bool isDistMeasureMode_ = false;
    int isDrawMeasure_ = -1;   //0:开始绘制（起点还没绘）    1：完成起点绘制    2:保持绘制完成的持续状态

signals:
    void distMeasureVisibleChanged();
    void distLineChanged();


private:
    bool isDistMeasureVisible_ = false;
    QLineF distLine_;
    QPointF distLineP1_, distLineP2_;



};

#endif // SCREETSHOT_H
