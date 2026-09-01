#include "device_manager.h"
#include "qglobal.h"

#include <QTimeZone>
#include <QDateTime>
#include <QUrl>
#include <QCoreApplication>
#include <QTimer>
#include <QDataStream>
#include <QMutexLocker>


#include "console.h"



DiskSonarCache::DiskSonarCache(const QString& filePath) : filePath_(filePath)
{
    // qDebug() << "filePath....." << filePath;
}

DiskSonarCache::~DiskSonarCache()
{
    close();
}

bool DiskSonarCache::openForWrite()
{
    file_.setFileName(filePath_);
    return file_.open(QIODevice::ReadWrite | QIODevice::Append);
}

bool DiskSonarCache::openForRead()
{
    close();
    file_.setFileName(filePath_);
    return file_.open(QIODevice::ReadOnly);
}

void DiskSonarCache::close()
{
    QMutexLocker lk(&mtx_);
    if (file_.isOpen()) {
        file_.flush();
        file_.close();
    }
}
void DiskSonarCache::clearFile()
{
    QMutexLocker lk(&mtx_);

    if (QFile::exists(filePath_)) {
        if (!QFile::remove(filePath_)) {
            qDebug() << "Failed to remove cache file:" << filePath_;
            return ;
        }
    }

    totalFramesWritten_ = 0;
    channelOffsets_.clear();
    openForWrite();
}

void DiskSonarCache::writeFrame(const QByteArray& rawFrame)
{
    file_.write(rawFrame.constData(), PING_SIZE_MAX);
    totalFramesWritten_++;
}

void DiskSonarCache::readFrame(qint64 epochIdx, QByteArray& outFrame)
{
    const qint64 offset = epochIdx * PING_SIZE_MAX;
    if (!file_.seek(offset)) {
        return ;
    }
    outFrame = file_.read(PING_SIZE_MAX);
}



/**------------------------------------DeviceManager-------------------------------------------**/
DeviceManager::DeviceManager(Dataset* datasetPtr): datasetPtr_(datasetPtr)
{
    qRegisterMetaType<int16_t>("int16_t");
    qRegisterMetaType<QVector<uint8_t>>("QVector<uint8_t>");
    qRegisterMetaType<QByteArray>("QByteArray");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<EnumFileType>("EnumFileType");

    QDateTime dateTime = QDateTime::currentDateTime();
    constructionTime = QString::number(dateTime.toTime_t());
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::setProgressDialog(QObject* dialog)
{
    if (progressDialog_ != dialog) {
        progressDialog_ = dialog;
    }
}

void DeviceManager::resetFileAndChannel(int fileCnt)
{
    batchChannelId_ = ChannelId(QUuid::createUuid(), 0);
    minZ_ = 0.0;
    maxZ_ = 0.0;
    depthVec_.clear();
    flag_haveReportAbnormalGPS = false;
    count_abnormalGPS = 0;

    if (diskSonarCache_) {
        diskSonarCache_->close();
        delete diskSonarCache_;
        diskSonarCache_ = nullptr;
    }
    QDir dir;
    QString dirPath = QString(qApp->applicationDirPath().append("/pixL/"));
    if (!dir.mkpath(dirPath)) {
        qDebug() << "mkpath failed:" << dirPath;
        return;
    }

    QString filePath = QDir(dirPath).filePath("pixL.txt");
    diskSonarCache_ = new DiskSonarCache(filePath);
    diskSonarCache_->clearFile();
    datasetPtr_->setDiskSonarCache(diskSonarCache_);
}

void DeviceManager::openFile_CSV(QString filePath, int fileIndex, int fileCnt)
{
    if (isOpeningFile_) {
        pendingFiles_.append({filePath, filetype_CSV, fileIndex, fileCnt});
        return;
    }
    isOpeningFile_ = true;

    QFile file;
    const QUrl url(filePath);
    url.isLocalFile() ? file.setFileName(url.toLocalFile()) : file.setFileName(url.toString());
    if (!file.open(QIODevice::ReadOnly)) {
        isOpeningFile_ = false;
        processNextPendingFile();
        return;
    }

    constexpr auto kFileUuidStr = "12345678-1234-1234-1234-1234567890ab";
    const QUuid someUuid(kFileUuidStr);

    QVector<float> vec_CSV;
    QList<Position> track;
    QTextStream counter(&file);
    int totalLines = 0;
    while(!counter.atEnd()) {
        counter.readLine();
        totalLines++;
    }
    file.seek(0);
    QTextStream in(&file);
    int skipRows = 2;
    int currentLine = 0;

    int validTotal = qMax(1, totalLines - skipRows);
    int progressInterval = qMax(1, validTotal / 100);
    while (!in.atEnd()) {
        QString row = in.readLine();
        if (skipRows > 0) {
            skipRows--;
            continue;
        }
        currentLine++;
        QStringList columns = row.split(",");

        Position pos;
        // pos.lla.latitude  = columns[4].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
        // pos.lla.longitude = columns[3].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
        // pos.lla.altitude  = columns[5].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
        pos.lla.latitude  = columns[1].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
        pos.lla.longitude = columns[0].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
        pos.lla.altitude  = columns[2].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
        track.append(pos);

        minZ_ = std::min(minZ_, pos.lla.altitude);
        maxZ_ = std::max(maxZ_, pos.lla.altitude);
        vec_CSV.append(pos.lla.altitude);
        if (currentLine > 0 && (currentLine % progressInterval == 0 || currentLine == validTotal)) {
            double progress = static_cast<double>(currentLine) / validTotal;

            QString statusText = tr("Processing CSV %1 / %2 (%3%)").arg(currentLine)
                                     .arg(validTotal).arg(static_cast<int>(progress * 100));
            if (progressDialog_) {
                QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, progress));
                QMetaObject::invokeMethod(progressDialog_, "setStatus",   Q_ARG(QVariant, statusText));
            }
            QCoreApplication::processEvents();
        }
        bool enableRender = currentLine == validTotal ? true : false;
        emit positionComplete_file(pos.lla.latitude, pos.lla.longitude, pos.lla.altitude,enableRender);
    }

    qDebug() << "vec_CSV.size()........." << vec_CSV.size();

    file.close();

    if (progressDialog_) {
        QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, 1.0));
        QMetaObject::invokeMethod(progressDialog_, "setStatus",   Q_ARG(QVariant, tr("Processing completed!")));
    }

    emit fileStopsOpening(vec_CSV, minZ_, maxZ_);

    isOpeningFile_ = false;
    processNextPendingFile();
}

void DeviceManager::openFile_tsl(QString filePath, EnumFileType currentFileType, int fileIndex, int fileCnt)
{
    if (isOpeningFile_) {
        pendingFiles_.append({filePath, currentFileType, fileIndex, fileCnt});
        return;
    }
    isOpeningFile_ = true;

    QFile tslFile;
    tslFile.setFileName(filePath);
    QByteArray tslByteArray;
    if(tslFile.open(QFileDevice::ReadOnly)) {
        tslByteArray = tslFile.readAll();
        tslFile.close();
    }
    if(currentFileType == filetype_tslw) {
        openFileData_tslw(tslByteArray, fileIndex, fileCnt);
    }
    else if(currentFileType == filetype_tsl3) {
        openFileData_tsl3_2(tslByteArray, fileIndex, fileCnt);
    }

    isOpeningFile_ = false;
    processNextPendingFile();
}
//(星标：嵌套逻辑，重要！！！！)用 QTimer::singleShot(0, this, lambda) 在下一次事件循环迭代触发下一个文件，
//确保当前调用栈完全展开（ fileStopsOpening → onFileStopsOpening 已跑完）后再开始新文件。
void DeviceManager::processNextPendingFile()
{
    if (pendingFiles_.isEmpty()) {
        return;
    }
    PendingFile next = pendingFiles_.takeFirst();
    QTimer::singleShot(0, this, [this, next]() {
        if (next.type == filetype_CSV) {
            openFile_CSV(next.path, next.index, next.cnt);
        } else {
            openFile_tsl(next.path, next.type, next.index, next.cnt);
        }
    });
}


double DeviceManager::dm_to_dd(double ddmmmmmmm)
{
    double dm = (double)ddmmmmmmm / 100.0f;
    int dd = (int)dm;
    double mm = (dm-dd) / 0.6f;
    return ((double)dd+mm);
}

void DeviceManager::GaussPC_calculation(double B, double L, double *x, double *y)
{
    double l, l2;
    double N, w0, w1, w2, w3, w4, cosb, cosb2;
    double dx, dy;

    l = L - ((int)L / 6 * 6 + 3);

    B = B * PI / 180.0;
    l = l * PI / 180.0;
    l2 = l * l;

    cosb = cos(B);
    cosb2 = cosb * cosb;

    N =  z1 +  (z2  + (z3  + z4  * cosb2) * cosb2) * cosb2;
    w0 = z5 +  (z6  + (z7  + z8  * cosb2) * cosb2) * cosb2;
    w1 = z9 +  (z10 + (z11 + z12 * cosb2) * cosb2) * cosb2;
    w2 = z13 + (z14 + z15 * cosb2) * cosb2;
    w3 = z16 + (z17 + z18 * cosb2) * cosb2;
    w4 = z19 + (z20 + (z21 + z22 * cosb2) * cosb2) * cosb2;

    *y = z0 * B + (w0 + (0.5 + (w1 + w2 * l2) * l2) * l2 * N) * cosb * sin(B);
    dy = z0 * B + (w0 + (0.5 + (w1 + w2 * l2) * l2) * l2 * N) * cosb * sin(B);

    *x = (1.0 + (w3 + w4 * l2) * l2) * l * N * cosb;
    dx = (1.0 + (w3 + w4 * l2) * l2) * l * N * cosb;

    *x += 500000.0;
}

float DeviceManager::GpsCal_GaussPC_DST(double PointA_lon, double PointA_lat, double PointB_lon, double PointB_lat)
{
    double xA, yA, xB, yB, DST;

    GaussPC_calculation(PointA_lat, PointA_lon, &xA, &yA);
    GaussPC_calculation(PointB_lat, PointB_lon, &xB, &yB);

    xB -= xA;
    yB -= yA;
    DST = sqrt(xB * xB + yB * yB);

    return DST;
}

void DeviceManager::openFileData_tslw(QByteArray &tslByteArray, int fileIndex, int fileCnt)
{
    tslByteArray.remove(0, 64);

    /*-将去掉文件头的所有剩下的声呐数据按照一帧一帧的模式放入临时容器-*/
    QList<QByteArray> tslByteList;
    int byteCount = 0;
    int nowIndex = 0;
    int maxCount = tslByteArray.count();
    byteCount = sizeof(pack_head_w)+sizeof(ping_info_w)+sizeof(navi_info_w)+sizeof(aux_info_w) +240 +1;
    while((maxCount -nowIndex) >= byteCount) {
        tslByteList.append(tslByteArray.mid(nowIndex,byteCount));
        nowIndex += byteCount;
    }
    if(tslByteList.isEmpty()) return;

    int tslCount = tslByteList.count();
    qDebug() << "tslWCnt.size()........." << tslCount;
    const int MEDIAN_WINDOW = 13;          // 窗口大小（奇数）
    const float SPIKE_THRESHOLD = 10.0f;   // 跳变阈值（米），超过用中值替代
    QList<LLA> buffer;
    int idx = sizeof(pack_head_w) + sizeof(ping_info_w) + sizeof(navi_info_w) + sizeof(aux_info_w);
    for(int i = 0; i < tslCount; i++)
    {
        QByteArray tslDataTemp = tslByteList.at(i);

        tsl_w tslSingleStru;
        memcpy(&tslSingleStru, tslDataTemp, idx);
        tslSingleStru.boat.longitude = dm_to_dd((double)tslSingleStru.boat.longitude/100000.0f) * 100000;
        tslSingleStru.boat.latitude  = dm_to_dd((double)tslSingleStru.boat.latitude /100000.0f) * 100000;
        LLA lla;
        lla.latitude  = tslSingleStru.boat.latitude  / 100000.f;
        lla.longitude = tslSingleStru.boat.longitude / 100000.f;
        if(lla.latitude < 0.000001f && lla.longitude < 0.000001f) {
            continue;
        }
        lla.altitude  = tslSingleStru.auxInfo.depth  / 100.f;

        buffer.append(lla);
        if (buffer.size() < MEDIAN_WINDOW) {
            // 窗口还没满的时候：为了保证轨迹点数量一致，将前 (W/2) 个点直接推入 track
            if (buffer.size() <= MEDIAN_WINDOW * 0.5) {
                minZ_ = std::min(minZ_, lla.altitude);
                maxZ_ = std::max(maxZ_, lla.altitude);
                depthVec_.append(lla.altitude);
            }
        }
        else {
            // 窗口超过设定大小，移除最旧的点，让窗口滑动
            if (buffer.size() > MEDIAN_WINDOW) {
                buffer.removeFirst();
            }

            // 获取窗口正中间的点
            int midIndex = MEDIAN_WINDOW * 0.5;
            LLA targetLla = buffer[midIndex];

            // 提取当前窗口内的所有深度，用于求中值
            QVector<float> depths;
            for (const LLA& item : buffer) {
                depths.append(item.altitude);
            }
            // 排序找中位数
            std::sort(depths.begin(), depths.end());
            float medianDepth = depths[MEDIAN_WINDOW * 0.5];

            // 异常值判断：与中位数偏差是否超过阈值
            if (std::abs(targetLla.altitude - medianDepth) > SPIKE_THRESHOLD) {
                targetLla.altitude = medianDepth; // 使用中值替代
                buffer[midIndex].altitude = medianDepth;
            }

            // 将过滤后的中心点加入最终轨迹
            minZ_ = std::min(minZ_, targetLla.altitude);
            maxZ_ = std::max(maxZ_, targetLla.altitude);
            depthVec_.append(targetLla.altitude);
        }


        // ----------- 将声呐数据发送到 Dataset ------------
        // QVector<QVector<uint8_t>> dataVec;
        // QVector<uint8_t> channelData;
        // for(int i = 0; i < 240; i++) {
        //     channelData.append((uint8_t)tslDataTemp[idx + i]);
        // }
        // for(int i = 240; i < PING_SIZE_MAX; i++) {
        //     channelData.append((uint8_t)'\0');
        // }
        // dataVec.append(channelData);
        QByteArray rawDat;
        for(int i = 0; i < 240; i++) {
            rawDat.append(tslDataTemp[idx+i]);
        }
        for(int i = 240; i < PING_SIZE_MAX; i++) {
            rawDat.append('\0');
        }
        diskSonarCache_->writeFrame(rawDat);

        float loRng = tslSingleStru.ping.loRng;
        float depth = tslSingleStru.auxInfo.depth;
        int pingSize = 240;
        float upRng = 0.0;

        ChartParameters chartParams;
        chartParams.depth       = depth;
        chartParams.pingSize    = pingSize;
        chartParams.upRng       = 0.0;
        chartParams.loRng       = loRng;
        chartParams.temperature = tslSingleStru.auxInfo.temperature * 10.0f;
        chartParams.heading     = tslSingleStru.boat.heading;
        chartParams.speed       = tslSingleStru.boat.speed * 10 / 0.514444f;
        chartParams.time        = tslSingleStru.boat.time;
        chartParams.longitude   = lla.longitude;
        chartParams.latitude    = lla.latitude;

        // emit chartComplete(batchChannelId_, chartParams, dataVec, true);
        // emit positionComplete_file(lla.latitude, lla.longitude, lla.altitude, enableRender);
        datasetPtr_->addPosition_file(lla.latitude, lla.longitude, lla.altitude, false);
        datasetPtr_->addChartMeta(batchChannelId_, chartParams, false);

        if (i % 200 == 0) {
            double progress = static_cast<double>(i + 1) / tslCount;
            QString statusText = tr("Openging files %1 of %2 (%3%)")
                                     .arg(fileIndex+1).arg(fileCnt).arg(static_cast<int>(progress * 100.0 + 0.5));
            QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, progress));
            QMetaObject::invokeMethod(progressDialog_, "setStatus",   Q_ARG(QVariant, statusText));
            QCoreApplication::processEvents();
        }
    }

    QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, 1.0));
    if(fileIndex == (fileCnt - 1)) {
        datasetPtr_->triggerRenderUpdate();
        emit fileStopsOpening(depthVec_, minZ_, maxZ_);
        QMetaObject::invokeMethod(progressDialog_, "setStatus", Q_ARG(QVariant, tr("Processing completed!")));
        QTimer::singleShot(2000, progressDialog_, [this]() {
            if(progressDialog_) {
                QMetaObject::invokeMethod(progressDialog_, "close");
            }});
    }

}

void DeviceManager::openFileData_tslw2(QByteArray &tslByteArray, int fileIndex, int fileCnt)
{
    for(int i = 0; i < 64; i++) {
        tslHeadByteArray.append(tslByteArray[i]);
    }

    tslByteArray.remove(0, 64);

    QList<QByteArray> tslByteList;
    int byteCount = 0;
    int nowIndex = 0;
    int maxCount = tslByteArray.count();
    int idx = sizeof(pack_head_w)+sizeof(ping_info_w)+sizeof(navi_info_w)+sizeof(aux_info_w);
    byteCount = idx + 240 + 1;
    while((maxCount -nowIndex) >= byteCount) {
        tslByteList.append(tslByteArray.mid(nowIndex,byteCount));
        nowIndex += byteCount;
    }

    if(tslByteList.isEmpty() || !progressDialog_) {
        return;
    }

    tsl_w tslSingleStruct;
    memcpy(&tslSingleStruct, tslByteList.first(), idx);
    double last_lon = dm_to_dd((double)tslSingleStruct.boat.longitude/100000.0f) * 100000;
    double last_lat = dm_to_dd((double)tslSingleStruct.boat.latitude /100000.0f) * 100000;

    int tslCount = tslByteList.count();
    for(int i = 0; i < tslCount; i++)
    {
        QByteArray tslDataTemp = tslByteList.at(i);

        quint8 chk = 0;
        for(int i = 3; i < tslDataTemp.count()-1; i++) {
            chk ^= tslDataTemp.at(i);
        }

        if(chk != (quint8)tslDataTemp.at(tslDataTemp.count()-1)) {
            continue;
        }

        tsl_w tslSingleStruct;
        memcpy((void*)&tslSingleStruct, tslDataTemp, idx);
        tslSingleStruct.boat.longitude = dm_to_dd((double)tslSingleStruct.boat.longitude/100000.0f)*100000;
        tslSingleStruct.boat.latitude  = dm_to_dd((double)tslSingleStruct.boat.latitude/100000.0f)*100000;
        LLA lla;
        lla.latitude  = tslSingleStruct.boat.latitude  / 100000.0f;
        lla.longitude = tslSingleStruct.boat.longitude / 100000.0f;
        double distance = GpsCal_GaussPC_DST((double)tslSingleStruct.boat.longitude/100000.0f,
                                    (double)tslSingleStruct.boat.latitude/100000.0f, last_lon, last_lat);
        if((lla.latitude < 0.000001f && lla.longitude < 0.000001f) || (distance > GPS_ERROR_DISTANCE)) {
            if(flag_haveReportAbnormalGPS == false) {
                flag_haveReportAbnormalGPS = true;
                flag_deleteAbnormalGPS = GIF->dialogYesNoSync(tr("Delete abnormal GPS coordinates?"));
            }

            if(flag_deleteAbnormalGPS == true) {
                count_abnormalGPS++;
                last_lon = lla.longitude;
                last_lat = lla.latitude;
                continue;
            }
        }


        QByteArray rawDat;
        for(int i = 0; i < 240; i++) {
            rawDat.append(tslDataTemp[idx+i]);
        }
        for(int i = 240; i < PING_SIZE_MAX; i++) {
            rawDat.append('\0');
        }
        diskSonarCache_->writeFrame(rawDat);

        float loRng = tslSingleStruct.ping.loRng;
        float depth = tslSingleStruct.auxInfo.depth;
        int pingSize = 240;
        float upRng = 0.0;

        lla.altitude  = depth / 100.f;

        ChartParameters chartParams;
        chartParams.depth       = depth;
        chartParams.pingSize    = pingSize;
        chartParams.upRng       = upRng;
        chartParams.loRng       = loRng;
        chartParams.temperature = tslSingleStruct.auxInfo.temperature * 10.0f;
        chartParams.heading     = tslSingleStruct.boat.heading;
        chartParams.speed       = tslSingleStruct.boat.speed * 10 / 0.514444f;
        chartParams.time        = tslSingleStruct.boat.time;
        chartParams.longitude   = lla.longitude;
        chartParams.latitude    = lla.latitude;

        datasetPtr_->addPosition_file(lla.latitude, lla.longitude, lla.altitude, false);
        datasetPtr_->addChartMeta(batchChannelId_, chartParams, false);

        depthVec_.append(lla.altitude);
        minZ_ = std::min(minZ_, lla.altitude);
        maxZ_ = std::max(maxZ_, lla.altitude);

        if (i % 200 == 0) {
            double progress = static_cast<double>(i + 1) / tslCount;
            QString statusText = tr("Openging files %1 of %2 (%3%)")
                        .arg(fileIndex+1).arg(fileCnt).arg(static_cast<int>(progress * 100.0 + 0.5));
            QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, progress));
            QMetaObject::invokeMethod(progressDialog_, "setStatus",   Q_ARG(QVariant, statusText));
            QCoreApplication::processEvents();
        }
    }

    QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, 1.0));
    if(fileIndex == (fileCnt - 1)) {
        datasetPtr_->triggerRenderUpdate();
        emit fileStopsOpening(depthVec_, minZ_, maxZ_);
        QMetaObject::invokeMethod(progressDialog_, "setStatus", Q_ARG(QVariant, tr("Processing completed!")));
        QTimer::singleShot(2000, progressDialog_, [this]() {
            if(progressDialog_) {
                QMetaObject::invokeMethod(progressDialog_, "close");
            }
        });
    }

}


void DeviceManager::openFileData_tsl3(QByteArray &tslByteArray, int fileIndex, int fileCnt)
{
    tslByteArray.remove(0, 512);  /*- 清除文件头信息，只保留声呐数据 -*/
    /*-将去掉文件头的所有剩下的声呐数据，按照一帧一帧的模式放入临时容器-*/
    QList<QByteArray> tslByteList;
    int nowIndex = 0;
    int byteCount = 0;
    int tslWCnt = tslByteArray.count();
    while((nowIndex) < (tslWCnt-100))
    {
        if('#' == tslByteArray.at(nowIndex)) {
            byteCount = sizeof(pack_head_t3)+sizeof(ping_info_t3)+sizeof(navi_info_t3)+sizeof(aux_info_t3)+
                        U8_TO_16(tslByteArray.at(nowIndex+22),tslByteArray.at(nowIndex+23))+1;
            if((byteCount >= 100) && (byteCount <= 2048)) {
                /*- 检查一下当前序号再加上获取的像素个数是不是已经超过整体长度了，超过的话进入下个循环 -*/
                if((nowIndex + byteCount) > tslWCnt) {
                    nowIndex++;
                    continue;
                }
                else {
                    QByteArray tslByteArray_xor = tslByteArray.mid(nowIndex,byteCount);
                    quint8 chk = 0;
                    for(int i = 3;i < tslByteArray_xor.count()-1;i++) {
                        chk ^= tslByteArray_xor.at(i);
                    }

                    if(chk == (quint8)tslByteArray_xor.at(tslByteArray_xor.count()-1)) {
                        tslByteList.append(tslByteArray.mid(nowIndex,byteCount));
                        nowIndex += byteCount;
                    }
                    else {
                        nowIndex++;
                        continue;
                    }
                }
            }
            else {
                nowIndex++;
                continue;
            }
        }
        else {
            nowIndex++;
            continue;
        }
    }


    int tsl3Cnt = tslByteList.count();
    int idx = sizeof(pack_head_t3)+sizeof(ping_info_t3)+sizeof(navi_info_t3)+sizeof(aux_info_t3);
    qDebug() << "tsl3Cnt.size()........." << tsl3Cnt;
    const int   MEDIAN_WINDOW   = 13;     // 窗口大小（奇数）
    const float SPIKE_THRESHOLD = 10.0f;  // 跳变阈值（米），超过用中值替代
    QList<LLA> buffer;
    for(int i = 0; i < tsl3Cnt; i++)
    {
        QByteArray tslDataTemp = tslByteList.at(i);

        tsl_3 tslSingleStru;
        memcpy(&tslSingleStru, tslDataTemp, idx);
        LLA lla;
        lla.latitude  = dm_to_dd(tslSingleStru.boat.latitude);
        lla.longitude = dm_to_dd(tslSingleStru.boat.longitude);
        float depth = tslSingleStru.auxInfo.depth;
        if(tslSingleStru.ping.frequency == snrFrq455) {
            if(depth > 10000) {
                depth = 0;
            }
            else if(depth > 30000) {
                depth = 0;
            }
        }
        lla.altitude = depth / 100.f;
        if(lla.latitude < 0.000001f && lla.longitude < 0.000001f) {
            continue;
        }

        buffer.append(lla);
        int bufSize = buffer.size();
        if (bufSize < MEDIAN_WINDOW) {
            // 窗口还没满的时候：为了保证轨迹点数量一致，将前 (W/2) 个点直接推入 track
            if (bufSize <= MEDIAN_WINDOW * 0.5) {
                minZ_ = std::min(minZ_, lla.altitude);
                maxZ_ = std::max(maxZ_, lla.altitude);
                depthVec_.append(lla.altitude);
                // track.append(lla);
            }
        }
        else {
            // 窗口超过设定大小，移除最旧的点，让窗口滑动
            if (bufSize > MEDIAN_WINDOW) {
                buffer.removeFirst();
            }

            // 获取窗口正中间的点
            int midIndex = MEDIAN_WINDOW / 2;
            LLA targetLla = buffer[midIndex];

            // 提取当前窗口内的所有深度，用于求中值
            QVector<float> depths;
            for (const LLA& item : buffer) {
                depths.append(item.altitude);
            }
            // 排序找中位数
            std::sort(depths.begin(), depths.end());
            float medianDepth = depths[MEDIAN_WINDOW / 2];

            // 异常值判断：与中位数偏差是否超过阈值
            if (std::abs(targetLla.altitude - medianDepth) > SPIKE_THRESHOLD) {
                targetLla.altitude = medianDepth; // 使用中值替代
                buffer[midIndex].altitude = medianDepth;
            }

            // 将过滤后的中心点加入最终轨迹
            minZ_ = std::min(minZ_, targetLla.altitude);
            maxZ_ = std::max(maxZ_, targetLla.altitude);
            depthVec_.append(targetLla.altitude);
            // track.append(targetLla);
        }


        // ----------- 将声呐数据发送到 Dataset ------------
        int pingSize = tslSingleStru.ping.size;
        QVector<QVector<uint8_t>> dataVec;
        QVector<uint8_t> channelData;
        for(int i = 0; i < pingSize; i++) {
            channelData.append((uint8_t)tslDataTemp[idx + i]);
        }
        for(int i = pingSize; i < PING_SIZE_MAX; i++) {
            channelData.append((uint8_t)'\0');
        }
        dataVec.append(channelData);

        float upRng = tslSingleStru.ping.upRng;
        float loRng = tslSingleStru.ping.loRng;


        ChartParameters chartParams;
        chartParams.depth       = depth;
        chartParams.pingSize    = pingSize;
        chartParams.upRng       = upRng;
        chartParams.loRng       = loRng;
        chartParams.temperature = tslSingleStru.auxInfo.temperature;
        chartParams.heading     = tslSingleStru.boat.heading;
        chartParams.speed       = tslSingleStru.boat.speed;
        chartParams.time        = tslSingleStru.boat.time;
        chartParams.longitude   = lla.longitude;
        chartParams.latitude    = lla.latitude;
        // qDebug() << "lla.latitude " << lla.latitude << "  " << lla.longitude << "  " << lla.altitude;
        bool enableRender = (fileIndex == fileCnt - 1) && ((i + 1) == tsl3Cnt);
        emit chartComplete(batchChannelId_, chartParams, dataVec, enableRender);

        emit positionComplete_file(lla.latitude, lla.longitude, lla.altitude, enableRender);

        // 更新进度条
        if (progressDialog_ && (i % 200 == 0 || i == (tsl3Cnt - 1))) {
            double progress = static_cast<double>(i + 1) / tsl3Cnt;
            QString statusText = tr("Openging files %1 of %2 (%3%)")
                            .arg(fileIndex+1).arg(fileCnt).arg(static_cast<int>(progress * 100.0 + 0.5));
            QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, progress));
            QMetaObject::invokeMethod(progressDialog_, "setStatus",   Q_ARG(QVariant, statusText));
            QCoreApplication::processEvents();
        }
    }
    // qDebug() << "track-tsl3 size().............." << track.size() << "  minZ:" << minZ << "  maxZ:" << maxZ;

    if (progressDialog_) {
        QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, 1.0));
        if(fileIndex == (fileCnt - 1)) {
            QMetaObject::invokeMethod(progressDialog_, "setStatus", Q_ARG(QVariant, tr("Processing completed!")));
            QTimer::singleShot(2500, progressDialog_, [this]() {
                if(progressDialog_) {
                    QMetaObject::invokeMethod(progressDialog_, "close");
            }});
        }
    }

    if(fileIndex == fileCnt - 1) {
        emit fileStopsOpening(depthVec_, minZ_, maxZ_);
    }
}


void DeviceManager::openFileData_tsl3_2(QByteArray &tslByteArray, int fileIndex, int fileCnt)
{
    /*- 获取文件头 -*/
    if(tslHeadByteArray.isEmpty()) {
        for(int i = 0; i < 512; i++) {
            tslHeadByteArray.append(tslByteArray[i]); /*-取出文件头-*/
        }
    }

    if(tslHead.time == "- -") {
        /*-最新的才有<>字符-*/
        if(tslHeadByteArray.contains('<')) {
            /*-按<区分，后面应该有>在-*/
            QList<QByteArray> list_head = tslHeadByteArray.split('<');
            for(int i = 0;i < list_head.count()-1; i++) {
                QByteArray head = list_head.at(i).trimmed();
                if(head.startsWith('t')) {
                    /*-time-*/
                    if(head.count() > 10) {
                        tslHead.time = head.mid(5);
                        tslHead.time.remove(6,1);
                        tslHead.time.insert(2,'/');
                        tslHead.time.insert(5,'/');
                        tslHead.time.insert(8,' ');
                        tslHead.time.insert(11,':');
                        tslHead.time.insert(14,':');
                    }
                    /*-tsl-*/
                    else {
                        tslHead.tslVer = head.mid(4).toFloat();
                    }
                }
                /*-app-*/
                if(head.startsWith('a')) {
                    tslHead.appVer = head.mid(8);
                }

                /*-sn-*/
                if(head.startsWith('s')) {
                    tslHead.sn = head.mid(4);
                }

                /*-logParm-*/
                if(head.startsWith('l')) {
                    QByteArray logParm = head.mid(8);
                    memcpy(&fileInfo_loggerCfg, logParm, sizeof(loggerCfg_t)-2);

                    logParm.remove(0,sizeof(loggerCfg_t)-2);
                    memcpy(&fileInfo_snrCtrl, logParm, sizeof(typSnrCtrl)-2);
                }
            }
        }
    }

    tslByteArray.remove(0, 512);

    QList<QByteArray> tslByteList;
    int nowIndex = 0;
    int byteCount = 0;
    int maxCount = tslByteArray.count();
    int idx = sizeof(pack_head_t3)+sizeof(ping_info_t3)+sizeof(navi_info_t3)+sizeof(aux_info_t3);
    while((nowIndex) < (maxCount-100)) {
        if('#' == tslByteArray.at(nowIndex)) {
            byteCount = idx + U8_TO_16(tslByteArray.at(nowIndex+22),tslByteArray.at(nowIndex+23))+1;
            if((byteCount >= 100) && (byteCount <= 2048)) {
                /*-检查一下当前序号再加上获取的像素个数是不是超过整体长度了，超过的话进入下个循环-*/
                if((nowIndex + byteCount) > maxCount) {
                    nowIndex++;
                    continue;
                }
                else {
                    QByteArray tslByteArray_xor = tslByteArray.mid(nowIndex,byteCount);
                    quint8 chk = 0;
                    for(int i = 3;i < tslByteArray_xor.count()-1;i++) {
                        chk ^= tslByteArray_xor.at(i);
                    }

                    if(chk == (quint8)tslByteArray_xor.at(tslByteArray_xor.count()-1)) {
                        tslByteList.append(tslByteArray.mid(nowIndex,byteCount));
                        nowIndex += byteCount;
                    }
                    else {
                        nowIndex++;
                        continue;
                    }
                }
            }
            else {
                nowIndex++;
                continue;
            }
        }
        else {
            nowIndex++;
            continue;
        }
    }

    if(tslByteList.isEmpty() || !progressDialog_) {
        return;
    }

    tsl_3 tslSingleStruct;
    memcpy(&tslSingleStruct, tslByteList.first(), idx);
    double last_lon = dm_to_dd(tslSingleStruct.boat.longitude);
    double last_lat = dm_to_dd(tslSingleStruct.boat.latitude);
    if(((last_lon < 0.000001f) && (last_lat < 0.000001f))) {
        if(flag_haveReportAbnormalGPS == false) {
            flag_haveReportAbnormalGPS = true;
            flag_deleteAbnormalGPS = GIF->dialogYesNoSync(tr("Delete abnormal GPS coordinates?"));
        }
    }

    int tslCount = tslByteList.count();
    for(int i = 0; i < tslCount; i++)
    {
        QByteArray tslDataTemp = tslByteList.at(i);
        tsl_3 tslSingleStruct;
        memcpy(&tslSingleStruct, tslDataTemp, idx);
        tslSingleStruct.boat.longitude = dm_to_dd(tslSingleStruct.boat.longitude);
        tslSingleStruct.boat.latitude  = dm_to_dd(tslSingleStruct.boat.latitude);
        double distance = GpsCal_GaussPC_DST(tslSingleStruct.boat.longitude, tslSingleStruct.boat.latitude, last_lon, last_lat);
        if((tslSingleStruct.boat.longitude < 0.000001f) && (tslSingleStruct.boat.latitude < 0.000001f)) {
            if(flag_haveReportAbnormalGPS == false) {
                flag_haveReportAbnormalGPS = true;
                flag_deleteAbnormalGPS = GIF->dialogYesNoSync(tr("Delete abnormal GPS coordinates?"));

            }
            if(flag_deleteAbnormalGPS == true) {
                count_abnormalGPS++;
                continue;
            }
        }
        else if(distance > GPS_ERROR_DISTANCE) {
            if(last_lon < 0.000001f && last_lat < 0.000001f) {
                last_lon = tslSingleStruct.boat.longitude;
                last_lat = tslSingleStruct.boat.latitude;
            }
            else {
                if(flag_haveReportAbnormalGPS == false) {
                    flag_haveReportAbnormalGPS = true;
                    flag_deleteAbnormalGPS = GIF->dialogYesNoSync(tr("Delete abnormal GPS coordinates?"));
                }

                if(flag_deleteAbnormalGPS == true) {
                    count_abnormalGPS++;
                    last_lon = tslSingleStruct.boat.longitude;
                    last_lat = tslSingleStruct.boat.latitude;
                    continue;
                }
            }
        }

        QByteArray rawDat;
        for(int i=0; i<tslSingleStruct.ping.size; i++) {
            rawDat.append(tslDataTemp[idx +i]);
        }
        for(int i = tslSingleStruct.ping.size; i < PING_SIZE_MAX; i++) {
            rawDat.append('\0');
        }
        // out.writeRawData(rawDat.data(),PING_SIZE_MAX);
        diskSonarCache_->writeFrame(rawDat);
        int pingSize = tslSingleStruct.ping.size;
        float upRng  = tslSingleStruct.ping.upRng;
        float loRng  = tslSingleStruct.ping.loRng;
        LLA lla;
        lla.latitude  = tslSingleStruct.boat.latitude;
        lla.longitude = tslSingleStruct.boat.longitude;
        float depth = tslSingleStruct.auxInfo.depth;
        if(tslSingleStruct.ping.frequency == snrFrq455) {
            if(depth > 10000) {
                depth = 0;
            }
            else if(depth > 30000) {
                depth = 0;
            }
        }
        lla.altitude = depth / 100.f;

        ChartParameters chartParams;
        chartParams.depth       = depth;
        chartParams.pingSize    = pingSize;
        chartParams.upRng       = upRng;
        chartParams.loRng       = loRng;
        chartParams.temperature = tslSingleStruct.auxInfo.temperature;
        chartParams.heading     = tslSingleStruct.boat.heading;
        chartParams.speed       = tslSingleStruct.boat.speed;
        chartParams.time        = tslSingleStruct.boat.time;
        chartParams.latitude    = lla.latitude;
        chartParams.longitude   = lla.longitude;
        // int pingCnt = tslSingleStruct.ping.size;
        // QVector<QVector<uint8_t>> dataVec;
        // QVector<uint8_t> channelData;
        // for(int i = 0; i < pingCnt; i++) {
        //     channelData.append((uint8_t)tslDataTemp[idx + i]);
        // }
        // for(int i = pingCnt; i < PING_SIZE_MAX; i++) {
        //     channelData.append((uint8_t)'\0');
        // }
        // dataVec.append(channelData);
        // datasetPtr_->addChart(batchChannelId_, chartParams, dataVec, false);

        datasetPtr_->addPosition_file(lla.latitude, lla.longitude, lla.altitude, false);
        datasetPtr_->addChartMeta(batchChannelId_, chartParams, false);

        depthVec_.append(lla.altitude);
        minZ_ = std::min(minZ_, lla.altitude);
        maxZ_ = std::max(maxZ_, lla.altitude);

        if (i % 200 == 0) {
            double progress = static_cast<double>(i + 1) / tslCount;
            QString statusText = tr("Openging files %1 of %2 (%3%)")
                        .arg(fileIndex+1).arg(fileCnt).arg(static_cast<int>(progress * 100.0 + 0.5));
            QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, progress));
            QMetaObject::invokeMethod(progressDialog_, "setStatus",   Q_ARG(QVariant, statusText));
            QCoreApplication::processEvents();
        }
    }

    QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, 1.0));
    if(fileIndex == (fileCnt - 1)) {
        datasetPtr_->triggerRenderUpdate();
        emit fileStopsOpening(depthVec_, minZ_, maxZ_);
        QMetaObject::invokeMethod(progressDialog_, "setStatus", Q_ARG(QVariant, tr("Processing completed!")));
        QTimer::singleShot(2000, progressDialog_, [this]() {
            if(progressDialog_) {
                QMetaObject::invokeMethod(progressDialog_, "close");
            }
        });
    }

}
