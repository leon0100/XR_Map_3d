// d:/MyProject/XR_Map_3d/src/device/serialportmanager.h
#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QByteArray>
#include <QDateTime>
#include <QTimer>
#include <QtEndian>
#include <QHostAddress>

#include "dataset_defs.h"

#include "tmodem.h"
#include "tsl3.h"
#include "id_binnary.h"



#define  TEMP_PATH_SERIAL       (qApp->applicationDirPath().append("/temp/").append(QString::number(qApp->applicationPid())))
#define  PATH_PIX_LFREQ_SERIAL  (TEMP_PATH_SERIAL.append("/pixL"))

class SerialPortManager : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QStringList availablePorts READ availablePorts                       NOTIFY portsUpdated)
    Q_PROPERTY(bool    connected          READ isConnected                          NOTIFY connectChanged)


public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    QStringList availablePorts();

    Q_INVOKABLE void scanPorts();
    Q_INVOKABLE void toggleConnection(QString port, int baudRate);

    bool isConnected();

    static char calculateChecksum(const QByteArray &data);
    static bool verifyChecksum(const QByteArray &nmeaSentence);



signals:
    void portsUpdated();
    void connectChanged(bool connected);
    void dataReceived(QString data);

    void positionComplete(double lat, double lon, double depth, bool isRead);
    void signal_drawRealtimeContour(QVector<float>& depth, double minZ, double maxZ, bool isRead);
    void chartComplete(const ChannelId& channelId, const ChartParameters& chartParams, const QVector<QVector<uint8_t>>& data, bool enableRender);



private slots:
    void handleReadyRead();
    void onHeartbeatTimeout();

private:
    void disConnectUdp();
    void clearRealData();

    QString getCurrentWifiName();
    uint8_t crc8_poly7(const uint8_t *data, int len);
    uint16_t crc16_modbus(const uint8_t *data, int len);
    QByteArray buildXrmapActivePayload(uint16_t map_ver, const QString &map_name, uint32_t map_size,
            uint16_t all_map_CRC16, uint32_t all_map_CRC32,  uint16_t pkt_bytes, uint16_t MAP_PKT_NUM,  uint32_t unix_sec);
    QByteArray buildTModemFrame_xrmap(uint8_t dev_addr, uint8_t sn, bool needAck,
                         uint8_t commandByte, const QByteArray &payload);
    void parseTModemFrame(QByteArray& rawData);
    void parseTsl3FromTModem();
    double dm_to_dd(double ddmmmmmmm);
    QByteArray decompressTsl3(const QByteArray &compressed);


private:
    int baudRate_ = 19200;

    QSerialPort *serialPort_;
    QStringList m_availablePorts;
    QByteArray readAllBuffer_;
    bool hasGPSData_ = false;

    QTimer* m_heartbeatTimer = nullptr;
    int m_heartbeatCnt = 0;
    int tmodemSn_ = 0;

    QByteArray m_tsl3Buffer;
    int nowIndex_  = 0;
    int tslIndex_ = 0;
    QVector<float> depthHistory_;
    double minDepth_ = 0.0, maxDepth_ = 0.0;
    bool readingDrawTrack_ = true;
    typSnrCtrl fileInfo_snrCtrl;
    QString constructionTime_;
    ChannelId batchChannelId_{QUuid(), 0};
};


#endif // SERIALPORTMANAGER_H
