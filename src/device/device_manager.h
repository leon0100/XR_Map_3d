#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QList>
#include <QHash>
#include <QGeoPositionInfoSource>
#include <QUuid>
#include "id_binnary.h"
#include "dataset.h"

class LocationReader;
class DeviceManager : public QObject
{
    Q_OBJECT

public:
    DeviceManager();
    ~DeviceManager();

    void setProgressDialog(QObject* dialog);
    void resetFileAndChannel(int fileCnt);


public slots:
    void openFile_CSV(QString filePath, int fileIndex, int fileCnt);
    void openFile_tsl(QString filePath, EnumFileType currentFileType, int fileIndex, int fileCnt);
    void closeFile();
    void setProtoBinConsoled(bool isConsoled);

    void beaconActivationReceive(uint8_t id);

    void onStartUpgradingFirmware(QUuid linkUuid, uint8_t address, const QByteArray& firmware);
    void onUpgradingFirmwareDone();

signals:
    void sendChartSetup(const ChannelId& channelId, uint16_t resol, uint16_t count, uint16_t offset);
    void sendTranscSetup(const ChannelId& channelId, uint16_t freq, uint8_t pulse, uint8_t boost);
    void sendSoundSpeeed(const ChannelId& channelId, uint32_t soundSpeed);

    void dataSend(QByteArray data);
    void chartComplete(const ChannelId& channelId, const ChartParameters& chartParams, const QVector<QVector<uint8_t>>& data, bool enableRender);
    void rawDataRecieved(const ChannelId& channelId, RawData rawData);
    void chartSetupChanged();
    void distSetupChanged();
    void datasetChanged();
    void transChanged();
    void soundChanged();
    void UARTChanged();
    void deviceVersionChanged();
    void devChanged();
    void streamChanged();
    void vruChanged();
    void eventComplete(int timestamp, int id, int unixt);
    void positionComplete(double lat, double lon, uint32_t date, uint32_t time);
    void positionComplete_file(double lat, double lon,int depth, bool enableRender);
    void signalpositionSonar();
    void tempComplete(float val);
    void fileStopsOpening();
    void fileStopsOpening2(QVector<float>& depth, double minZ, double maxZ);
    void chartLossesChanged();


private:
    void delAllDev();
    void openFileData_tslw(QByteArray &tslByteArray, int fileIndex, int fileCnt);
    void openFileData_tsl3(QByteArray &tslByteArray, int fileIndex, int fileCnt);
    double dm_to_dd(double ddmmmmmmm);
    void processNextPendingFile();

    // VruData vru_;
    QHash<QUuid, int> otherProtocolStat_;
    QUuid lastUuid_;
    QUuid proxyLinkUuid_;
    QUuid mavlinUuid_;
    int lastAddress_;
    int progress_;
    bool isConsoled_;
    volatile bool break_;

    QTimer* beacon_timer = nullptr;
    QUuid upgradeUuid_;
    uint8_t upgradeAddr_;
    QByteArray upgradeData_;
    LocationReader* locReader_;

    QObject* progressDialog_ = nullptr;
    ChannelId batchChannelId_{QUuid(), 0};
    QVector<float> depthVec_;
    double minZ_ = 0.0, maxZ_ = 0.0;
    struct PendingFile {
        QString path;
        EnumFileType type;
        int index;
        int cnt;
    };
    bool isOpeningFile_ = false;
    QList<PendingFile> pendingFiles_;
};
