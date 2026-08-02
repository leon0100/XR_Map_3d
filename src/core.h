#pragma once

#include <QObject>
#include <QQmlApplicationEngine>
#include <QStandardItemModel>
#include <QQmlContext>
#include <QThread>
#include <QFileDialog>
#include "data_processor.h"
#include "qPlot2D.h"
#include "logger.h"
#include "console.h"
#include "scene3d_view.h"
#include "boat_track_control_menu_controller.h"
#include "navigation_arrow_control_menu_controller.h"
#include "bottom_track_control_menu_controller.h"
#include "isobaths_view_control_menu_controller.h"
#include "mosaic_view_control_menu_controller.h"
#include "image_view_control_menu_controller.h"
#include "map_view_control_menu_controller.h"
#include "usbl_view_control_menu_controller.h"
#include "point_group_control_menu_controller.h"
#include "polygon_group_control_menu_controller.h"
#include "mpc_filter_control_menu_controller.h"
#include "npd_filter_control_menu_controller.h"
#include "scene3d_toolbar_controller.h"
#include "scene3d_control_menu_controller.h"
#include "device_manager_wrapper.h"
#include "link_manager_wrapper.h"
#include "tile_manager.h"
#include "data_horizon.h"
#include "blemanager.h"
#include "udpmanager.h"
#include "serialportmanager.h"
#include "location.h"

class Core : public QObject
{
    Q_OBJECT

public:
    Core();
    ~Core();

    Q_PROPERTY(bool      isGPSAlive                   READ getIsGPSAlive                   NOTIFY isGPSAliveChanged)
    Q_PROPERTY(bool      isFactoryMode                READ isFactoryMode                   CONSTANT)
    Q_PROPERTY(ConsoleListModel* consoleList          READ consoleList                     CONSTANT)
    Q_PROPERTY(QString   filePath                     READ getFilePath                     NOTIFY filePathChanged)
    Q_PROPERTY(bool      isFileOpening                READ getIsFileOpening                NOTIFY sendIsFileOpening)
    Q_PROPERTY(bool      isSeparateReading            READ getIsSeparateReading            CONSTANT)
    Q_PROPERTY(QString   ch1Name                      READ getChannel1Name                 NOTIFY channelListUpdated FINAL)
    Q_PROPERTY(QString   ch2Name                      READ getChannel2Name                 NOTIFY channelListUpdated FINAL)
    Q_PROPERTY(int       currMapLevel                 READ getCurrMapLevel                 NOTIFY currentMapLevelChanged)
    Q_PROPERTY(QObject*  progress   READ progress       WRITE setProgress        NOTIFY progressChanged)
    Q_PROPERTY(bool batchCorrect   READ batchCorrect    WRITE setBatchCorrect    NOTIFY drawBatchCorrectChanged)
    Q_PROPERTY(bool depthCorrect   READ depthCorrect    WRITE setDepthCorrect    NOTIFY drawDepthCorrectChanged)



    void setEngine(QQmlApplicationEngine *engine);
    Console* getConsolePtr();
    Dataset* getDatasetPtr();
    DataProcessor* getDataProcessorPtr() const;
    DeviceManagerWrapper* getDeviceManagerWrapperPtr() const;
    LinkManagerWrapper* getLinkManagerWrapperPtr() const;
    void stopLinkManagerTimer() const;
    void refreshMap(LLA lla);
    void saveCurrentMapState(std::function<void(double lat, double lon)>writer);

    void consoleInfo(QString msg);
    void consoleWarning(QString msg);
    void consoleProto(FrameParser& parser, bool isIn = true);
    void saveLLARefToSettings();
    void removeLinkManagerConnections();

    QHash<QUuid, QString> getLinkNames() const;



public slots:
    void setIsGPSAlive(bool state) { isGPSAlive_ = state; emit isGPSAliveChanged(); }
    bool getIsGPSAlive() const { return isGPSAlive_; };
    void openLogFile(const QString& filePath, bool isAppend = false, bool onCustomEvent = false);
    bool closeLogFile();
    void onFileOpened();
    bool openXTF(const QByteArray& data);
    bool openCSV(QString name, int separatorType, int row = -1, int colTime = -1, bool isUtcTime = true, int colLat = -1, int colLon = -1, int colAltitude = -1, int colNorth = -1, int colEast = -1, int colUp = -1);
    bool openProxy(const QString& address, const int port, bool isTcp);
    bool closeProxy();
    bool upgradeFW(const QString& name, QObject* dev);
    void upgradeChanged(int progressStatus);
    bool exportUSBLToCSV(QString filePath);
    bool exportPlotAsCVS(QString filePath, const ChannelId& channelId, float decimation = 0);
    bool exportPlotAsXTF(QString filePath);
    void setPlotStartLevel(int level);
    void setPlotStopLevel(int level);
    void setTimelinePosition(double position);
    void resetAim();
    void UILoad(QObject* object, const QUrl& url);
    void setMosaicChannels(const QString& firstChStr, const QString& secondChStr);
    bool getIsFileOpening() const;
    bool getIsSeparateReading() const;
    void onChannelsUpdated();
    int  getDataProcessorState() const;
    void initStreamList();
    int  getCurrMapLevel() const;
    void createDatasetConnections();
    void createScene3dConnections();
    void onTileSetChanged(std::shared_ptr<map::TileSet> tileSet);

    QObject* progress() const;
    void setProgress(QObject* dialog);
    bool batchCorrect();
    void setBatchCorrect(bool batchCorrect);
    bool depthCorrect();
    void setDepthCorrect(bool depthCorrect);

    Q_INVOKABLE QString getChannel1Name() const;
    Q_INVOKABLE QString getChannel2Name() const;
    Q_INVOKABLE QVariant getConvertedMousePos(int indx, int mouseX, int mouseY);

    Q_INVOKABLE void setIsAttitudeExpected(bool state);
    Q_INVOKABLE void openFileFromMenu();
    Q_INVOKABLE void clearRouteData();
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE void location(uint8_t type);
    Q_INVOKABLE void setAutoRenderSpan(bool isAuto);
    Q_INVOKABLE void exitApp();
    Q_INVOKABLE void switchMapType(int sourceType);
    Q_INVOKABLE void bathyMetryConfigApply(int soundVelocity, int draftOffset);
    Q_INVOKABLE void setDepthFilterVisible(bool visible, int value);
    Q_INVOKABLE void setKeelOffsetValue(int value);

signals:
    void connectionChanged(bool duplex = false);
    void filePathChanged();
    void sendIsFileOpening();
    void channelListUpdated();
    void isGPSAliveChanged();

    void scrrenModeChanged();
    void currentMapLevelChanged();

    void progressChanged();

    void drawRealtimeContour(bool isRead);

    void drawBatchCorrectChanged();
    void drawDepthCorrectChanged();


private:
    QObject* progress_ = nullptr;
    QMetaObject::Connection oldTileSetConnection_;


private slots:
    void onFileStopsOpening();
    void onFileStopsOpening2(QVector<float>& depthVec, double minZ, double maxZ);
    void onSendMapTextureIdByTileIndx(const map::TileIndex& tileIndx, GLuint textureId); // TODO: maybe store map texture id in mapView
    void onDataProcesstorStateChanged(const DataProcessorType& state);

    void onZoomLevelChanged(int level);

    void slot_RealtimeDrawContourBle(QVector<float>& depthVec, double minZ, double maxZ, bool isRead);
    void slot_RealtimeDrawContourWifi(QVector<float>& depthVec, double minZ, double maxZ, bool isRead);
    void slot_RealtimeDrawContourSerialPort(QVector<float>& depthVec, double minZ, double maxZ, bool isRead);


private:
    /*methods*/
    void createMapTileManagerConnections();
    void createDataProcessor();
    void destroyDataProcessor();

    void setDataProcessorConnections();
    void resetDataProcessorConnections();

    ConsoleListModel* consoleList();
    void createControllers();
    void createDeviceManagerConnections();
    void createLinkManagerConnections();
    bool isOpenedFile() const;
    bool isFactoryMode() const;

    QString getFilePath() const;
    void fixFilePathString(QString& filePath) const;
    void loadLLARefFromSettings();

    /*data*/
    Console* consolePtr_;
    // 3d scene controllers
    std::shared_ptr<BoatTrackControlMenuController>       boatTrackControlMenuController_;
    std::shared_ptr<NavigationArrowControlMenuController> navigationArrowControlMenuController_;
    std::shared_ptr<BottomTrackControlMenuController>     bottomTrackControlMenuController_;
    std::shared_ptr<MpcFilterControlMenuController>       mpcFilterControlMenuController_;
    std::shared_ptr<NpdFilterControlMenuController>       npdFilterControlMenuController_;
    std::shared_ptr<IsobathsViewControlMenuController>    isobathsViewControlMenuController_;
    std::shared_ptr<MosaicViewControlMenuController>      mosaicViewControlMenuController_;
    std::shared_ptr<ImageViewControlMenuController>       imageViewControlMenuController_;
    std::shared_ptr<MapViewControlMenuController>         mapViewControlMenuController_;
    std::shared_ptr<PointGroupControlMenuController>      pointGroupControlMenuController_;
    std::shared_ptr<PolygonGroupControlMenuController>    polygonGroupControlMenuController_;
    std::shared_ptr<Scene3DControlMenuController>         scene3dControlMenuController_;
    std::shared_ptr<Scene3dToolBarController>             scene3dToolBarController_;
    std::shared_ptr<UsblViewControlMenuController>        usblViewControlMenuController_;
    std::unique_ptr<DeviceManagerWrapper>                 deviceManagerWrapperPtr_;
    std::unique_ptr<LinkManagerWrapper>                   linkManagerWrapperPtr_;
    std::unique_ptr<map::TileManager>                     tileManager_;

    std::shared_ptr<BLEManager>  bleManager_;
    std::shared_ptr<UdpManager>  udpManager_;
    std::shared_ptr<SerialPortManager> serialPortManager_;
    std::shared_ptr<Locations>   locations_;

    // data processor
    DataProcessor* dataProcessor_;
    QThread* dataProcThread_;
    std::unique_ptr<DataHorizon> dataHorizon_; // this thread

    QQmlApplicationEngine* qmlAppEnginePtr_;
    Dataset* datasetPtr_;
    QPointer<GraphicsScene3dView> scene3dViewPtr_;
    Logger logger_;
    QList<qPlot2D*> plot2dList_;
    QList<QMetaObject::Connection> linkManagerWrapperConnections_;
    QString openedfilePath_, openedFileFilter_;
    EnumFileType currentFileType_;
    QString filePath_;
    QString fChName_;
    QString sChName_;

    bool isFileOpening_;

    bool isGPSAlive_;

    int  currMapLevel_ = 0;
    bool isAutoRenderSpan_ = true;

    bool isBatchCorrect_ = false;
    bool isDepthCorrect_ = false;

    QVector<QMetaObject::Connection> dataProcessorConnections_;
    DataProcessorType dataProcessorState_ = DataProcessorType::kUndefined;

};
