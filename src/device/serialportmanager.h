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
    //class传给bleDataPanel.qml
    Q_PROPERTY(QString latitude    READ latitude     NOTIFY dataPanelUpdate)
    Q_PROPERTY(QString longitude   READ longitude    NOTIFY dataPanelUpdate)
    Q_PROPERTY(QString angle       READ angle        NOTIFY dataPanelUpdate)
    Q_PROPERTY(QString speed       READ speed        NOTIFY dataPanelUpdate)
    Q_PROPERTY(QString depth       READ depth        NOTIFY dataPanelUpdate)

    Q_PROPERTY(bool dataReading    READ dataReading  WRITE setDataReading     NOTIFY dataReadingChanged)



public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    QStringList availablePorts();

    QString latitude()  const   { return QString::number(latitude_, 'f', 6); }
    QString longitude() const   { return QString::number(longitude_, 'f', 6); }
    QString angle()     const   { return QString::number(angle_ / 10.f, 'f', 1);}
    QString speed()     const   { return QString::number(speed_/ 100 * 0.514444f, 'f', 2); }
    QString depth()     const   { return QString::number(depth_/ 100.0f, 'f', 2); }

    bool dataReading() const { return readingDrawTrack_; }
    void setDataReading(bool isReading) { readingDrawTrack_ = isReading; emit dataReadingChanged(isReading);}

    Q_INVOKABLE void scanPorts();
    Q_INVOKABLE void toggleConnection(QString port, int baudRate);

    bool isConnected();

    void clearRealData();

    static char calculateChecksum(const QByteArray &data);
    static bool verifyChecksum(const QByteArray &nmeaSentence);



signals:
    void portsUpdated();
    void connectChanged(bool connected);
    void dataReceived(QString data);
    void dataPanelUpdate();
    void dataReadingChanged(bool isReading);

    void positionComplete(double lat, double lon, double depth, bool isRead);
    void signal_drawRealtimeContour(QVector<float>& depth, double minZ, double maxZ, bool isRead);
    void chartComplete(const ChannelId& channelId, const ChartParameters& chartParams, const QVector<QVector<uint8_t>>& data, bool enableRender);



private slots:
    void handleReadyRead();

private:

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



public:
    double latitude_ = 000.000;
    double longitude_ = 000.000;
    double angle_ = 000.000;
    double speed_ = 0.0;
    double depth_ = 0.0;

private:
    int baudRate_ = 230400;

    QSerialPort *serialPort_;
    QStringList m_availablePorts;
    QByteArray readAllBuffer_;
    bool hasGPSData_ = false;

    int tmodemSn_ = 0;

    QByteArray m_tsl3Buffer;
    int nowIndex_  = 0;
    int tslIndex_ = 0;
    QVector<float> depthHistory_;
    double minDepth_ = 0.0, maxDepth_ = 0.0;
    bool readingDrawTrack_ = true;
    ChannelId batchChannelId_{QUuid(), 0};
};


#endif // SERIALPORTMANAGER_H
