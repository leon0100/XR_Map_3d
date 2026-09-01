#pragma once

#include <QObject>
#include <QQmlApplicationEngine>
#include <QStandardItemModel>
#include <QQmlContext>
#include <QThread>
#include <QFileDialog>
#include "dataset.h"
#include "data_processor.h"
#include "qPlot2D.h"
#include "scene3d_view.h"
#include "boat_track_control_menu_controller.h"
#include "bottom_track_control_menu_controller.h"
#include "isobaths_view_control_menu_controller.h"
#include "image_view_control_menu_controller.h"
#include "tile_manager.h"
#include "data_horizon.h"
#include "device_manager.h"
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

    Q_PROPERTY(QString   filePath            READ getFilePath                NOTIFY filePathChanged)
    Q_PROPERTY(bool      isFileOpening       READ getIsFileOpening           NOTIFY sendIsFileOpening)
    Q_PROPERTY(bool      isSeparateReading   READ getIsSeparateReading       CONSTANT)
    Q_PROPERTY(QString   ch1Name             READ getChannel1Name            NOTIFY channelListUpdated  FINAL)
    Q_PROPERTY(QString   ch2Name             READ getChannel2Name            NOTIFY channelListUpdated  FINAL)
    Q_PROPERTY(int       currMapLevel        READ getCurrMapLevel            NOTIFY currentMapLevelChanged)
    Q_PROPERTY(QObject*  progress            READ progress    WRITE setProgress   NOTIFY progressChanged)

    void setEngine(QQmlApplicationEngine *engine);
    Dataset* getDatasetPtr();
    DataProcessor* getDataProcessorPtr() const;
    void refreshMap(LLA lla);
    void saveCurrentMapState(std::function<void(double lat, double lon)>writer);

    QHash<QUuid, QString> getLinkNames() const;


public slots:
    void setTimelinePosition(double position);
    void resetAim();
    void UILoad(QObject* object, const QUrl& url);
    void setMosaicChannels(const QString& firstChStr, const QString& secondChStr);
    bool getIsFileOpening() const;
    bool getIsSeparateReading() const;
    void onChannelsUpdated();
    int  getDataProcessorState() const;
    int  getCurrMapLevel() const;
    void createDatasetConnections();
    void createScene3dConnections();
    void onTileSetChanged(std::shared_ptr<map::TileSet> tileSet);

    QObject* progress() const;
    void setProgress(QObject* dialog);

    Q_INVOKABLE QString getChannel1Name() const;
    Q_INVOKABLE QString getChannel2Name() const;
    Q_INVOKABLE QVariant getConvertedMousePos(int indx, int mouseX, int mouseY);

    Q_INVOKABLE void onFitAllInViewButtonClicked();
    Q_INVOKABLE void onIsNorthLocationButtonChanged(bool state);
    Q_INVOKABLE void onNavigationArrowVisibleChanged(bool checked);
    Q_INVOKABLE void openFileFromMenu();
    Q_INVOKABLE void clearRouteData();
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE void location(uint8_t type);
    Q_INVOKABLE void setAutoRenderSpan(bool isAuto);
    Q_INVOKABLE void exitApp();
    Q_INVOKABLE void switchMapType(int sourceType);
    Q_INVOKABLE int  poolSize();

signals:
    void connectionChanged(bool duplex = false);
    void filePathChanged();
    void sendIsFileOpening();
    void channelListUpdated();

    void scrrenModeChanged();
    void currentMapLevelChanged();

    void progressChanged();

    void drawRealtimeContour(bool isRead);


private slots:
    void onFileStopsOpening(QVector<float>& depthVec, double minZ, double maxZ);
    void onSendMapTextureIdByTileIndx(const map::TileIndex& tileIndx, GLuint textureId); // TODO: maybe store map texture id in mapView

    void onZoomLevelChanged(int level);

    void slot_RealtimeDrawContourBle(QVector<float>& depthVec, double minZ, double maxZ, bool isRead);
    void slot_RealtimeDrawContourWifi(QVector<float>& depthVec, double minZ, double maxZ, bool isRead);
    void slot_RealtimeDrawContourSerialPort(QVector<float>& depthVec, double minZ, double maxZ, bool isRead);


private:
    void onDataProcesstorStateChanged(const DataProcessorType& state);
    void createMapTileManagerConnections();
    void createDataProcessor();
    void destroyDataProcessor();

    void setDataProcessorConnections();
    void resetDataProcessorConnections();

    void createControllers();
    void createDeviceManagerConnections();
    bool isOpenedFile() const;

    QString getFilePath() const;
    void fixFilePathString(QString& filePath) const;
    void loadLLARefFromSettings();

private:
    std::shared_ptr<BoatTrackControlMenuController>     boatTrackControlMenuController_;
    std::shared_ptr<BottomTrackControlMenuController>   bottomTrackControlMenuController_;
    std::shared_ptr<IsobathsViewControlMenuController>  isobathsViewControlMenuController_;
    std::shared_ptr<ImageViewControlMenuController>     imageViewControlMenuController_;
    std::unique_ptr<map::TileManager>                   tileManager_;

    std::shared_ptr<DeviceManager> deviceManager_;
    std::shared_ptr<BLEManager>    bleManager_;
    std::shared_ptr<UdpManager>    udpManager_;
    std::shared_ptr<SerialPortManager> serialPortManager_;
    std::shared_ptr<Locations>     locations_;

    DataProcessor* dataProcessor_;
    QThread* dataProcThread_;
    std::unique_ptr<DataHorizon> dataHorizon_; // this thread

    QQmlApplicationEngine* qmlAppEnginePtr_;
    Dataset* datasetPtr_;
    QPointer<GraphicsScene3dView> scene3dViewPtr_;
    QList<qPlot2D*> plot2dList_;
    QString openedfilePath_, openedFileFilter_;
    EnumFileType currentFileType_;
    QString filePath_;
    QString fChName_;
    QString sChName_;
    bool isFileOpening_;
    QObject* progress_ = nullptr;
    QMetaObject::Connection oldTileSetConnection_;

    int  currMapLevel_ = 0;
    bool isAutoRenderSpan_ = true;

    QVector<QMetaObject::Connection> dataProcessorConnections_;
    DataProcessorType dataProcessorState_ = DataProcessorType::kUndefined;
};
