#ifndef BLEMANAGER_H
#define BLEMANAGER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QBluetoothSocket>
#include <QBluetoothLocalDevice>
#include <QStringList>
#include <QVector>
#include <QListWidgetItem>
#include <QTimer>
#include <QQuickWidget>
#include <QQuickView>
#include <QQuickWindow>
#include <QElapsedTimer>
#include <QMutex>
#include <QThread>


#include "dataset_defs.h"

// #include "tmodem.h"
// #include "../Code/fifotmodem.h"
// #include "../Map/gpsprocessing.h"
#define RX_FIFO_SIZE (1024*1024*4)


struct BoatPoint {
    double latitude;
    double longitude;
    double depth;   // m
    double heading;
    double speed;   // m/s

    QString date;   // yyyy-MM-dd
    QString time;   // HH:mm:ss

    BoatPoint(double lo = 0.0, double la = 0.0, double he = 0.0, double sp = 0.0, double de = 0.0)
        : latitude(la), longitude(lo), depth(de), heading(he), speed(sp) {}
};
Q_DECLARE_METATYPE(BoatPoint)



class RealTimeParser : public QObject
{
    Q_OBJECT
public:
    explicit RealTimeParser(QObject *parent = nullptr);
    ~RealTimeParser() override;

    const BoatPoint& getCurrentBoatPoint();
    void setHasLast(bool haslast);


private:
    double parseNMEACoordinate(const QString &coord, const QString &direction);
    char calculateChecksum(const QByteArray &data);
    QStringList splitQString(const QString &str,char delimiter);
    bool verifyChecksum(const QByteArray &nmeaSentence);


public slots:
    void enqueueData(const QByteArray &data);

    // 在 parser 所在线程中启动定时器或处理循环
    void start();
    void stop();

signals:
    // 解析出一个点后发出（queued connection 跨线程安全）
    void parsedPoint(const BoatPoint &pt);
    void stopped();


private slots:
    void processQueue();


private:
    QByteArray m_buffer;
    QMutex m_mutex;
    QTimer *timer_;
    bool m_running = false;

    BoatPoint lastBoatPt_;
    bool hasLast_ = false;

    int count_ = 0;

    QString lastDateStr_;  // "yyyy-MM-dd" 标准化日期
    QString lastTimeStr_;  // "HH:mm:ss" 记录最新时钟时间

};








/*-----------------------------------------------BLEManager-------------------------------------------------*/
class BLEManager : public QObject
{
    Q_OBJECT
public:
    //class传给bleLiveDataAndScanning.qml
    Q_PROPERTY(QStringList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString scanStatus READ scanStatus NOTIFY scanStatusChanged)

    //class传给bleDataPanel.qml
    Q_PROPERTY(QString latitude READ latitude NOTIFY dataPanelUpdate)
    Q_PROPERTY(QString longitude READ longitude NOTIFY dataPanelUpdate)
    Q_PROPERTY(QString angle READ angle NOTIFY dataPanelUpdate)
    Q_PROPERTY(QString speed READ speed NOTIFY dataPanelUpdate)
    Q_PROPERTY(QString depth READ depth NOTIFY dataPanelUpdate)

    Q_PROPERTY(bool dataPaused READ dataPaused WRITE setDataPaused NOTIFY dataPausedChanged)


    //class传给liveDataColor.qml
    Q_PROPERTY(double maxDepthValue READ maxDepthValue WRITE setMaxDepthValue NOTIFY maxDepthValueChanged)
    Q_PROPERTY(float currentDepthValue READ currentDepthValue WRITE resetCurrentDepthValue NOTIFY resetCurrentDepthVal)

public:
    explicit BLEManager(QObject *parent = nullptr);
    void translate();
    ~BLEManager() override;


    bool hasBlePermission();
    void doStartScan();



    QStringList devices() const { return m_devices; }
    bool connected() const { return m_connected; }
    QString scanStatus() const { return m_scanStatus; }

    QString latitude() const { return QString::number(parser_->getCurrentBoatPoint().latitude, 'f', 6); }
    QString longitude() const { return QString::number(parser_->getCurrentBoatPoint().longitude, 'f', 6); }
    QString angle() const { return QString::number(parser_->getCurrentBoatPoint().heading, 'f', 2); }
    QString speed() const { return QString::number(parser_->getCurrentBoatPoint().speed, 'f', 2); }
    QString depth() const { return QString::number(parser_->getCurrentBoatPoint().depth, 'f', 2); }

    bool dataPaused() const { return realDataPause_; }
    void setDataPaused(bool paused) { realDataPause_ = paused; emit dataPausedChanged(paused);}

    double maxDepthValue() const { return m_maxDepthValue;}
    double currentDepthValue() const { return 0.0; };


public: //qml传给class
    Q_INVOKABLE void setBleLiveScanningVisible(bool visible);

    Q_INVOKABLE void startStopScan(bool scan);
    Q_INVOKABLE void connectToDevice(int index);
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void sendData(const QString &datas);
    Q_INVOKABLE void sendData2(const QByteArray &data);
    Q_INVOKABLE void clearRealData();
    Q_INVOKABLE QStringList getDeviceNames() const;

    Q_INVOKABLE void setMaxDepthValue(double depth);
    Q_INVOKABLE void resetCurrentDepthValue(double );




signals:
    void scanCanceled();
    void devicesChanged();
    void connectedChanged(bool isCononect);
    void dataReceived(const QByteArray &data);
    void scanStatusChanged(const QString &status);

    void dataPanelUpdate();
    void parsedPoint(const BoatPoint &pt);
    void dataPausedChanged(bool realDataPause);

    void maxDepthValueChanged();
    void resetCurrentDepthVal();

    void positionComplete(double lat, double lon, double depth);
    void depthComplete(float depth);
    void fileStopsOpening_CSV(QVector<float>& depth, double minZ, double maxZ);
    void signal_drawRealtimeContour(bool isDraw);


public slots:
    void slot_drawRealtimeContour(bool isDraw);

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &deviceInfo);
    void onScanFinished();
    void onScanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void onScanCanceled();
    void onServiceDiscovered(const QBluetoothUuid &uuid);
    void onServiceScanDone();
    void serviceStateChanged(QLowEnergyService::ServiceState s);

    //特性值由事件改变时发出此信号在外设上
    void BleServiceCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    //当特征读取请求成功返回其值时
    void BleServiceCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    //当特性值成功更改为newValue时
    void BleServiceCharacteristicWrite(const QLowEnergyCharacteristic &c, const QByteArray &value);

    void onDeviceConnected();
    void onDeviceDisconnected();
    void onControllerError(QLowEnergyController::Error error);

    void slotTrackTimeout();

    void slot_parserRealtimePt(const BoatPoint &pt);

public:
    double latitude_ = 000.000;
    double longitude_ = 000.000;
    double angle_ = 000.000;
    double speed_ = 0.0;
    double depth_ = 0.0;



private:
    QStringList m_devices;
    int currentDeviceIndex_ = -1;
    bool m_connected = false;
    QString m_scanStatus;
    QString m_currentDeviceName;

    void setScanStatus(const QString &status);

    bool liveScanVisble_ = false;
    int currentTrackIndex_ = 0;



private:
    QLowEnergyController *bleController_ = nullptr;
    QLowEnergyService *bleServer_ = nullptr;

    QLowEnergyCharacteristic m_writeCharacteristic;
    QLowEnergyCharacteristic m_readCharacteristic;
    QLowEnergyDescriptor m_notificationDesc;

    QLowEnergyService::WriteMode m_writeMode;

    QBluetoothDeviceDiscoveryAgent *discoveryAgent = nullptr;
    QList<QBluetoothDeviceInfo> devicesList_;

    void searchCharacteristic();
    void SendMsg(QString text);

    QQuickView* loadingQuickView_;


private:
    QElapsedTimer recvTimer_;
    RealTimeParser *parser_ = nullptr;
    QThread *parserThread_;

    quint64 lastElapsed_ = 0;
    quint32 bleCount_ = 0;
    bool realDataPause_ = false;

    double m_maxDepthValue = 0.0;

    QList<Position> track_;
    QTimer *trackTimer_;

    QVector<float> depthHistory_; // 存储深度历史数据
    double minDepth_ = 0.0; // 最小深度
    double maxDepth_ = 0.0; // 最大深度

    bool isDrawRealtimeContour_ = false;


public:

    // 开始/停止定时发送信号
    void startContourSignal();
    void stopContourSignal();

private:
    QTimer* contourTimer_ = nullptr;  // 定时器


};



#endif // BLEMANAGER_H
