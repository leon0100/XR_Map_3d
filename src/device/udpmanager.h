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






class UdpManager : public QObject
{
    Q_OBJECT
public:
    explicit UdpManager(QObject *parent = nullptr);

    ~UdpManager();


    void stopHeartbeat();

    void disConnectUdp();


private:
    uint8_t crc8_poly7(const uint8_t *data, int len);
    uint16_t crc16_modbus(const uint8_t *data, int len);
    QByteArray buildXrmapActivePayload(uint16_t map_ver,  const QString &map_name,  uint32_t map_size,
            uint16_t all_map_CRC16,
            uint32_t all_map_CRC32,  uint16_t pkt_bytes, uint16_t MAP_PKT_NUM,  uint32_t unix_sec);
    QByteArray buildTModemFrame_xrmap(uint8_t dev_addr, uint8_t sn, bool needAck,
                                      uint8_t commandByte, const QByteArray &payload);
    void parseTModemFrame(const QByteArray& rawData);
    void parseTsl3FromTModem();
    double dm_to_dd(double ddmmmmmmm);




private slots:
    void onReadyRead();
    void onHeartbeatTimeout();




signals:
    void dataReceived(const QByteArray& data);






private:
    QUdpSocket* m_udpSocket;
    QString m_remoteIp;
    quint16 m_remotePort;
    int m_heartbeatCnt = 0;
    int tmodemSn_ = 0;

    QTimer* m_heartbeatTimer = nullptr;
    QByteArray m_tsl3Buffer;
    int nowIndex  = 0;
    int tslIndex_ = 0;
    QList<QByteArray> tslByteList;

};

#endif // UDPMANAGER_H
