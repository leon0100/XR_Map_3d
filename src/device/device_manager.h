#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QList>
#include <QHash>
#include <QGeoPositionInfoSource>
#include <QUuid>
#include "link.h"
#include "stream_list.h"
#include "id_binnary.h"

class LocationReader;
class DeviceManager : public QObject
{
    Q_OBJECT

public:
    DeviceManager();
    ~DeviceManager();

    Q_INVOKABLE float vruVoltage();
    Q_INVOKABLE float vruCurrent();
    Q_INVOKABLE float vruVelocityH();
    Q_INVOKABLE int   pilotArmState();
    Q_INVOKABLE int   pilotModeState();

    void setProgressDialog(QObject* dialog);
    void resetFileAndChannelId(int fileCnt);


public slots:
    Q_INVOKABLE StreamListModel* streamsList();

    void initStreamList();
    void openFile_CSV(QString filePath);
    void openFile_tsl(QString filePath, EnumFileType currentFileType);
    void closeFile();
    void onLinkOpened(QUuid uuid, Link *link);
    void onLinkClosed(QUuid uuid, Link* link);
    void onLinkDeleted(QUuid uuid, Link* link);
    void binFrameOut(Parsers::ProtoBinOut protoOut);
    void setProtoBinConsoled(bool isConsoled);

    void beaconActivationReceive(uint8_t id);
    void beaconDirectQueueAsk();
    bool isbeaconDirectQueueAsk() { return isUSBLBeaconDirectAsk; }
    void setUSBLBeaconDirectAsk(bool is_ask);

    void onStartUpgradingFirmware(QUuid linkUuid, uint8_t address, const QByteArray& firmware);
    void onUpgradingFirmwareDone();

    void createLocationReader();
    void destroyLocationReader();
    void shutdown();

    void onPositionUpdated(const QGeoPositionInfo& info);

signals:
    void sendChartSetup (const ChannelId& channelId, uint16_t resol, uint16_t count, uint16_t offset);
    void sendTranscSetup(const ChannelId& channelId, uint16_t freq, uint8_t pulse, uint8_t boost);
    void sendSoundSpeeed(const ChannelId& channelId, uint32_t soundSpeed);

    void dataSend(QByteArray data);
    void chartComplete(const ChannelId& channelId, const ChartParameters& chartParams, const QVector<QVector<uint8_t>>& data, bool enableRender);
    void rawDataRecieved(const ChannelId& channelId, RawData rawData);
    void distComplete(const ChannelId& channelId, int dist);
    void usblSolutionComplete(IDBinUsblSolution::UsblSolution data);
    void dopplerBeamComlete(IDBinDVL::BeamSolution* beams, uint16_t cnt);
    void dvlSolutionComplete(IDBinDVL::DVLSolution dvlSolution);
    void chartSetupChanged();
    void distSetupChanged();
    void datasetChanged();
    void transChanged();
    void soundChanged();
    void UARTChanged();
    void upgradeProgressChanged(int progressStatus);
    void deviceVersionChanged();
    void devChanged();
    void streamChanged();
    void vruChanged();
    void writeProxyFrame(Parsers::FrameParser frame);
    void writeMavlinkFrame(Parsers::FrameParser frame);
    void eventComplete(int timestamp, int id, int unixt);
    void rangefinderComplete(const ChannelId& channelId, float distance);
    void positionComplete(double lat, double lon, uint32_t date, uint32_t time);
    void positionComplete_file(double lat, double lon,int depth, bool enableRender);
    void signalpositionSonar();
    void positionCompleteRTK(Position position);
    void depthComplete(float depth);
    void gnssVelocityComplete(double hSpeed, double course);
    void attitudeComplete(float yaw, float pitch, float roll);
    void tempComplete(float val);
    void encoderComplete(float e1, float e2, float e3);
    void fileStopsOpening();
    void fileStopsOpening2(QVector<float>& depth, double minZ, double maxZ);
    void chartLossesChanged();
    void sendProtoFrame(const Parsers::ProtoBinOut& protoOut);
    void fileOpened();


private:
    void delAllDev();
    void openFileData_tslw(QByteArray &tslByteArray);
    void openFileData_tsl3(QByteArray &tslByteArray);
    double dm_to_dd(double ddmmmmmmm);

    /*data*/
    struct VruData {
        VruData() :
            voltage(NAN),
            current(NAN),
            velocityH(NAN),
            armState(-1),
            flightMode(-1)
        {};

        void cleanVru()
        {
            voltage = NAN;
            current = NAN;
            velocityH = NAN;
            armState = -1;
            flightMode = -1;
        };

        float voltage;
        float current;
        float velocityH;
        int armState;
        int flightMode;
    };

    VruData vru_;
    Link* mavlinkLink_;
    QHash<QUuid, int> otherProtocolStat_;
    StreamList streamList_;
    QUuid lastUuid_;
    QUuid proxyLinkUuid_;
    QUuid mavlinUuid_;
    int lastAddress_;
    int progress_;
    bool isConsoled_;
    volatile bool break_;

    bool isUSBLBeaconDirectAsk = false;
    QTimer* beacon_timer = nullptr;
    QUuid upgradeUuid_;
    uint8_t upgradeAddr_;
    QByteArray upgradeData_;
    LocationReader* locReader_;

    QObject* progressDialog_ = nullptr;
    ChannelId batchChannelId_{QUuid(), 0};
    double minZ_ = 0.0, maxZ_ = 0.0;
};
