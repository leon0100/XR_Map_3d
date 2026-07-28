#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QtEndian>
#include <QTimer>


#include "tmodem.h"
#include "tsl3.h"
#include "id_binnary.h"



#define  TEMP_PATH  (qApp->applicationDirPath().append("/temp/").append(QString::number(qApp->applicationPid())))
#define  PATH_PIX_LFREQ   (TEMP_PATH.append("/pixL"))


class UdpManager : public QObject
{
    Q_OBJECT
public:
    explicit UdpManager(QObject *parent = nullptr);

    ~UdpManager();


    void disConnectUdp();
    void clearRealData();


    Q_PROPERTY(QString wifiName    READ wifiName                          NOTIFY wifiNameChanged)
    Q_PROPERTY(QString remoteIp    READ remoteIp     WRITE setRemoteIp    NOTIFY remoteIpChanged)
    Q_PROPERTY(QString remotePort  READ remotePort   WRITE setRemotePort  NOTIFY remotePortChanged)


    QString remoteIp() const;
    void setRemoteIp(const QString& ip);

    QString remotePort() const;
    void setRemotePort(QString port);

    QString wifiName() const;


    Q_INVOKABLE void openUdp(bool open);
    Q_INVOKABLE void setDataReading(bool isReading);



private:
    QString  getCurrentWifiName();
    uint8_t  crc8_poly7(const uint8_t *data, int len);
    uint16_t crc16_modbus(const uint8_t *data, int len);
    QByteArray buildXrmapActivePayload(uint16_t map_ver, const QString &map_name, uint32_t map_size,
        uint16_t all_map_CRC16, uint32_t all_map_CRC32,  uint16_t pkt_bytes, uint16_t MAP_PKT_NUM,  uint32_t unix_sec);
    QByteArray buildTModemFrame_xrmap(uint8_t dev_addr, uint8_t sn, bool needAck,
                            uint8_t commandByte, const QByteArray &payload);
    void parseTModemFrame(const QByteArray& rawData);
    void parseTsl3FromTModem();
    double dm_to_dd(double ddmmmmmmm);
    QByteArray decompressTsl3(const QByteArray &compressed);




private slots:
    void onReadyRead();
    void onHeartbeatTimeout();




signals:
    void remoteIpChanged();
    void remotePortChanged();
    void wifiNameChanged();
    void isConnectedChanged();
    void dataReadingChanged();
    void signalCancelUdpOn(bool isOn);


    void dataReceived(const QByteArray& data);

    void positionComplete(double lat, double lon, double depth, bool isRead);
    void signal_drawRealtimeContour(QVector<float>& depth, double minZ, double maxZ, bool isRead);





private:
    QUdpSocket* m_udpSocket;
    QString m_wifi;
    bool m_isConnected = false;
    QString m_remoteIp;
    QString m_remotePort;
    int m_heartbeatCnt = 0;
    int tmodemSn_ = 0;

    QTimer* m_heartbeatTimer = nullptr;
    QByteArray m_tsl3Buffer;
    int nowIndex_  = 0;
    int tslIndex_ = 0;
    QVector<float> depthHistory_;
    double minDepth_ = 0.0, maxDepth_ = 0.0;
    bool readingDrawTrack_ = true;
    typSnrCtrl fileInfo_snrCtrl;
    QString constructionTime_;

};

#endif // UDPMANAGER_H
