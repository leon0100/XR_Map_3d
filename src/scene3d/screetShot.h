#ifndef SCREETSHOT_H
#define SCREETSHOT_H

#include <QObject>
#include <QPoint>
#include <QWidget>
#include <QGuiApplication>
#include <QRectF>
#include <QVector3D>
#include <QCursor>
#include <memory>
#include <QtConcurrent>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QGraphicsScene>


// #include <QtGui/private/qzipreader_p.h>
// #include <QtGui/private/qzipwriter_p.h>



enum class ResizeMode { None, Move, Top, Bottom, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };

#define _180_PI (57.2957795131f)
#define _PI_180 (0.01745329252f)
#define EARTH_RADIUS 6378137   // 赤道半径
#define POINT_REPEATITIVE 361.0f


#ifndef PI
#define PI (3.1415926535898)
#endif


#define  MAP_TIlE_SIZE   (256)








// 截图任务结构体
struct ScreenshotTask
{
    int     row;
    int     col;
    int     mapLevel;
    double  minLat;
    double  maxLat;
    double  minLon;
    double  maxLon;
    QString outputPath;

    ScreenshotTask() : row(0), col(0), mapLevel(0), minLat(0), maxLat(0), minLon(0), maxLon(0) {}

    ScreenshotTask(int r, int c, int level, double minL, double maxL, double minLn, double maxLn, const QString& path)
        : row(r), col(c), mapLevel(level),
        minLat(minL), maxLat(maxL), minLon(minLn), maxLon(maxLn), outputPath(path) {}
};








/*------------------------------------------ScreetShot---------------------------------------------*/
class MapView;
class ScreetShot : public QWidget
{
    Q_OBJECT
public:
    explicit ScreetShot(QWidget *parent = nullptr);

    // 添加矩形属性
    Q_PROPERTY(QRectF selectionRect READ getSelectionRect WRITE setSelectionRect NOTIFY selectionRectChanged)
    Q_PROPERTY(bool isSelectionRectVisible READ isSelectionRectVisible WRITE setSelectionRectVisible NOTIFY selectionRectVisibleChanged)
    Q_PROPERTY(QString screetWidth  READ getScreetWidth   WRITE setScreetWidth NOTIFY screetWidthChanged)
    Q_PROPERTY(QString screetHeight READ getScreetHeight  WRITE  setScreetHeight NOTIFY screetHeightChanged)
    Q_PROPERTY(bool screetToolBar   READ getScreetToolBar WRITE setScreetToolBar NOTIFY screetToolBarShowChanged)

    QRectF getSelectionRect() const;
    void setSelectionRect(const QRectF& rect);

    bool isSelectionRectVisible() const;
    void setSelectionRectVisible(bool visible);

    QString getScreetWidth() const;
    void setScreetWidth(const QString screetWidth);
    QString getScreetHeight() const;
    void setScreetHeight(const QString screetHeight);

    bool getScreetToolBar() const;
    void setScreetToolBar(bool screetToolBarShow);


public:
    QRectF shotRect_;
    bool isSelectionRectVisible_ = false;
    QString screetWidth_, screetHeight_;


signals:
    void selectionRectChanged();
    void selectionRectVisibleChanged();
    void screetWidthChanged();
    void screetHeightChanged();
    void screetToolBarShowChanged();
    void cancelScreetShot();

    void signalScreetGraphics();


public:
    void setCurrentMapLevel(int mapLevel);
    void setMapView(const std::shared_ptr<MapView>& mapView);
    void judgeResizeMode(const QRectF rect,const QPoint pos);
    void resizeMode(QRectF& rect,const QPoint pos);



    qreal clip(qreal n, qreal min, qreal max);
    qreal clipLon(qreal lon);   // 裁剪经度范围
    qreal clipLat(qreal lat);   // 裁剪纬度范围
    uint  mapSize(int level);   // 根据地图级别计算世界地图总宽高(以像素为单位)
    QPoint latLongToPixelXY(qreal lon, qreal lat, int level);               // 经纬度转像素 XY坐标
    void pixelXYToLatLong(QPoint pos, int level, qreal& lon, qreal& lat);   // 像素坐标转WGS-84墨卡托坐标
    void pixelXYToLatLong(QPointF pos, int level, qreal& lon, qreal& lat);
    double getDistance_Haversine(double current_longi, double current_lati, double goal_longi, double goal_lati);
    bool createKmlFile(QString kmlPath,QString imageName,double north,double south,double east,double west);
    bool createXMAPFile(const QString kmlFilePath, const QString imageFilePath, QString &outputXMAPPath);


    Q_INVOKABLE void setToArrowCursor();
    Q_INVOKABLE void setCancelShot();
    Q_INVOKABLE void saveScreetShot();

private slots:
    void slot_downloadScreenFinished();

private:

    QString getLengthChEn(double distance,int decimalPlaces = 2);
    void getTitle(QRect rect, int level);
    void getUrl();
    // void httpGetScreen(ImageInfo info);










public:
    bool isScreenMode_ = false;      // 截图模式
    bool m_moveView = false;         // 鼠标移动地图

    bool showHistoryScreen_ = false; // 显示历史截图

    bool firstScreenDown_ = false;
    QPointF startPos_,endPos_;       // 矩形场景坐标

    ResizeMode resizeMode_;

    int currMapLevel_ = 0;
    bool dragging_ = false;

    double topLeftLong_, topLeftLati_, topRightLong_, topRightLati_, bottomRightLong_, bottomRightLati_;
    bool screetToolBarShow_ = false;

    // std::weak_ptr<MapView> mapView_;
    // QRect targetRect_;
    // QFuture<void> m_future; //异步计算的结果
    // QString m_url = GOOGlE_MAP_COM;
    // QSet<quint64> m_exist;        // 已经存在的瓦片地图编号
    // QVector<ImageInfo> m_infos;   // 需要下载的瓦片地图信息
    // int m_completedDownloads_ = 0,m_screenTileCnt_ = 0;
    // bool interruptTile_ = false,isScreenNetError_ = false;
    // QHash<quint16, GraphItemGroup*> m_itemGroup;   // 瓦片图元组
    // bool screenUserCancel_ = false;
    double topWidth_,rightHeight_;
    // int pixel300m_;
    // QGraphicsScene* m_scene = nullptr;
    // int generatedRectCount_ = 0;
    // QMutex imgMutex_;   // 多线程写入保护
    // int tileStartX_ = 0;
    // int tileStartY_ = 0;




    bool screenshotPending_ = false;
    QString screenshotPath_;
    QMutex screenshotMutex_;


    //现在采用FBO分块离屏渲染的方式截图




};

#endif // SCREETSHOT_H
