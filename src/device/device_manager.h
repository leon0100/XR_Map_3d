#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QList>
#include <QHash>
#include <QGeoPositionInfoSource>
#include <QUuid>
#include <QFile>
#include <QMutex>
#include "dataset.h"
#include "id_binnary.h"

#ifndef PI
#define PI (3.1415926535897932384626)
#endif

#define z0 (6367449.14585577)
#define z1 (6399593.62580000)
#define z2 (-2.15650202422767e+004)
#define z3 (1.09003037983916e+002)
#define z4 (-6.12188017458933e-001)
#define z5 (-3.21444799442298e+004)
#define z6 (1.35366946123277e+002)
#define z7 (-7.09481085293974e-001)
#define z8 (4.06137292124076e-003)
#define z9 (-4.16666666666667e-002)
#define z10 (2.50000000000000e-001)
#define z11 (2.52731128372009e-003)
#define z12 (7.57013608867423e-006)
#define z13 (1.38888888888889e-003)
#define z14 (-8.33333333333333e-002)
#define z15 (1.66666666666667e-001)
#define z16 (-1.66666666666667e-001)
#define z17 (3.33333333333333e-001)
#define z18 (1.12324945943115e-003)
#define z19 (8.33333333333333e-003)
#define z20 (-1.66666666666667e-001)
#define z21 (1.96742576567650e-001)
#define z22 (4.04369805395214e-003)


// #define TEMP_PATH (qApp->applicationDirPath().append("/temp/").append(QString::number(qApp->applicationPid())))
// #define PATH_OFFLINE_TEMP (TEMP_PATH.append("/tempoff"))
// #define PATH_OFFLINE_TEMP_HFREQ (TEMP_PATH.append("/tempoffhfreq"))
// #define PATH_PIX (TEMP_PATH.append("/pixL"))
// #define PATH_PIX_HFREQ (TEMP_PATH.append("/pixH"))
#define PING_SIZE_MAX 1024
#define GPS_ERROR_DISTANCE 5000






class DiskSonarCache {
public:
    explicit DiskSonarCache(const QString& filePath);
    ~DiskSonarCache();

    bool openForWrite();
    bool openForRead();
    void flush();
    void close();

    qint64 writeFrame(const ChannelId& channelId, uint8_t subChannelId, const QByteArray& rawFrame);
    bool readFrame(const ChannelId& channelId, uint8_t subChannelId, qint64 epochIdx, QByteArray& outFrame);

    void registerChannelOffset(const ChannelId& channelId, uint8_t subChannelId, qint64 startEpochIdx);
    qint64 frameCount() const { return totalFramesWritten_; }

private:
    QString filePath_;
    QFile file_;
    QHash<QPair<QUuid, int>, qint64> channelOffsets_;
    qint64 totalFramesWritten_ = 0;
    QMutex mtx_;
};



class DeviceManager : public QObject
{
    Q_OBJECT

public:
    DeviceManager(Dataset* datasetPtr);
    ~DeviceManager();

    void setProgressDialog(QObject* dialog);
    void resetFileAndChannel(int fileCnt);


public slots:
    void openFile_CSV(QString filePath, int fileIndex, int fileCnt);
    void openFile_tsl(QString filePath, EnumFileType currentFileType, int fileIndex, int fileCnt);

signals:
    void dataSend(QByteArray data);
    void chartComplete(const ChannelId& channelId, const ChartParameters& chartParams, const QVector<QVector<uint8_t>>& data, bool enableRender);
    void rawDataRecieved(const ChannelId& channelId, RawData rawData);
    void positionComplete(double lat, double lon, uint32_t date, uint32_t time);
    void positionComplete_file(double lat, double lon,int depth, bool enableRender);
    void fileStopsOpening();
    void fileStopsOpening2(QVector<float>& depth, double minZ, double maxZ);


private:
    void openFileData_tslw(QByteArray &tslByteArray, int fileIndex, int fileCnt);
    void openFileData_tslw2(QByteArray &tslByteArray, int fileIndex, int fileCnt);
    void openFileData_tsl3(QByteArray &tslByteArray, int fileIndex, int fileCnt);
    void openFileData_tsl3_2(QByteArray &tslByteArray, int fileIndex, int fileCnt);

    double dm_to_dd(double ddmmmmmmm);
    void GaussPC_calculation(double B, double L, double *x, double *y);
    float GpsCal_GaussPC_DST(double PointA_lon, double PointA_lat, double PointB_lon, double PointB_lat);
    void processNextPendingFile();



private:
    Dataset* datasetPtr_ = nullptr;
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


    QByteArray tslHeadByteArray;
    StructTslHead tslHead;
    bool flag_deleteAbnormalGPS = false;
    bool flag_haveReportAbnormalGPS = false;
    int count_abnormalGPS = 0;
    loggerCfg_t fileInfo_loggerCfg;
    typSnrCtrl fileInfo_snrCtrl;
    QString sonarFreqString = "",sonarFreqString_2 = "";
    QString constructionTime = "";


    DiskSonarCache* diskSonarCache_ = nullptr;
    qint64 totalSonarFramesWritten_ = 0;
};
