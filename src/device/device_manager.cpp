#include "device_manager.h"
#include "location_reader.h"

#include <QTimeZone>
#include <QDateTime>
#include <QUrl>
#include <QCoreApplication>



DeviceManager::DeviceManager()
    : mavlinkLink_(nullptr),
    streamList_(this),
    lastAddress_(-1),
    progress_(0),
    isConsoled_(false),
    break_(false),
    upgradeUuid_(QUuid()),
    upgradeAddr_(0)
{
    qRegisterMetaType<ProtoBinOut>("ProtoBinOut");
    qRegisterMetaType<int16_t>("int16_t");
    qRegisterMetaType<QVector<uint8_t>>("QVector<uint8_t>");
    qRegisterMetaType<QByteArray>("QByteArray");
    qRegisterMetaType<IDBinUsblSolution::UsblSolution>("IDBinUsblSolution::UsblSolution");
    qRegisterMetaType<IDBinDVL::BeamSolution>("IDBinDVL::BeamSolution");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<IDBinDVL::DVLSolution>("IDBinDVL::DVLSolution");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<FrameParser>("FrameParser");
    qRegisterMetaType<EnumFileType>("EnumFileType");
}

DeviceManager::~DeviceManager()
{

}

float DeviceManager::vruVoltage()
{
    return vru_.voltage;
}

float DeviceManager::vruCurrent()
{
    return vru_.current;
}

float DeviceManager::vruVelocityH()
{
    return vru_.velocityH;
}

int DeviceManager::pilotArmState()
{
    return vru_.armState;
}

int DeviceManager::pilotModeState()
{
    return vru_.flightMode;
}

void DeviceManager::setProgressDialog(QObject* dialog)
{
    if (progressDialog_ != dialog) {
        progressDialog_ = dialog;
    }
}

void DeviceManager::resetFileAndChannelId(int fileCnt)
{
    batchChannelId_ = ChannelId(QUuid::createUuid(), 0);
    minZ_ = 0.0;
    maxZ_ = 0.0;
}

void DeviceManager::initStreamList()
{
    streamList_.initTimer();
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
        emit fileStopsOpening();
        isOpeningFile_ = false;
        processNextPendingFile();
        return;
    }

    Parsers::FrameParser frameParser;
    constexpr auto kFileUuidStr = "12345678-1234-1234-1234-1234567890ab";
    const QUuid someUuid(kFileUuidStr);

    delAllDev();

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
        pos.lla.latitude  = columns[5].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
        pos.lla.longitude = columns[4].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
        pos.lla.altitude  = columns[6].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
        track.append(pos);

        minZ_ = std::min(minZ_, pos.lla.altitude);
        maxZ_ = std::max(maxZ_, pos.lla.altitude);
        vec_CSV.append(pos.lla.altitude);
        if (currentLine > 0 && (currentLine % progressInterval == 0 || currentLine == validTotal))
        {
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

    emit fileStopsOpening2(vec_CSV, minZ_, maxZ_);  //这一步使得最后将读取到的轨迹内容绘制到scene3d_view上

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
    {
        QByteArray tslByteArray;
        if(tslFile.open(QFileDevice::ReadOnly)) {
            tslByteArray = tslFile.readAll();
            tslFile.close();
        }
        if(currentFileType == filetype_tslw) {
            openFileData_tslw(tslByteArray, fileIndex, fileCnt);
        }
        else if(currentFileType == filetype_tsl3) {
            openFileData_tsl3(tslByteArray, fileIndex, fileCnt);
        }

    }

    isOpeningFile_ = false;
    processNextPendingFile();
}
//(星标：嵌套逻辑，重要！！！！)用 QTimer::singleShot(0, this, lambda) 在下一次事件循环迭代触发下一个文件，
//确保当前调用栈完全展开（ fileStopsOpening2 → onFileStopsOpening2 已跑完）后再开始新文件。
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

    int tslWCnt = tslByteList.count();
    QVector<float> vec_CSV;
    QList<LLA> track;
    // ============ 声呐数据参数 =============
    qDebug() << "tslWCnt.size()........." << tslWCnt;
    const int MEDIAN_WINDOW = 13;          // 窗口大小（奇数）
    const float SPIKE_THRESHOLD = 10.0f;   // 跳变阈值（米），超过用中值替代
    QList<LLA> buffer;
    int progressInterval = qMax(1, tslWCnt / 4);
    int idx = sizeof(pack_head_w) + sizeof(ping_info_w) + sizeof(navi_info_w) + sizeof(aux_info_w);
    for(int cnt = 0; cnt < tslWCnt; cnt++)
    {
        QByteArray tslDataTemp = tslByteList.at(cnt);

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
                vec_CSV.append(lla.altitude);
                track.append(lla);
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
            vec_CSV.append(targetLla.altitude);
            track.append(targetLla);
        }


        // ----------- 将声呐数据发送到 Dataset ------------
        QVector<QVector<uint8_t>> dataVec;
        QVector<uint8_t> channelData;
        for(int i = 0; i < 240; i++) {
            channelData.append((uint8_t)tslDataTemp[idx + i]);
        }
        for(int i = 240; i < PING_SIZE_MAX; i++) {
            channelData.append((uint8_t)'\0');
        }
        dataVec.append(channelData);

        float loRng = tslSingleStru.ping.loRng;
        float depth = tslSingleStru.auxInfo.depth;
        int pingSize = 240;
        float upRng = 0.0;

        ChartParameters chartParams;
        // chartParams.sfEnd       = sfEnd;
        // chartParams.btStart     = btStart;
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
        emit chartComplete(batchChannelId_, chartParams, dataVec, true);

        // qDebug() << "lla.latitude " << lla.latitude << "  " << lla.longitude << "  " << lla.altitude;
        bool enableRender = (cnt + 1) == tslWCnt ? true : false;

        emit positionComplete_file(lla.latitude, lla.longitude, lla.altitude, enableRender);

        // 更新进度条
        if (progressDialog_ && (cnt % progressInterval == 0 || cnt == (tslWCnt - 1))) {
            double progress = static_cast<double>(cnt + 1) / tslWCnt;
            // QString statusText = tr("Processing frame %1 of %2 (%3%)")
            //                         .arg(cnt + 1).arg(tslWCnt).arg(static_cast<int>(progress * 100));
            QString statusText = tr("Openging files %1 of %2 (%3%)")
                                     .arg(fileIndex + 1).arg(fileCnt).arg(static_cast<int>(progress * 100));
            QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, progress));
            QMetaObject::invokeMethod(progressDialog_, "setStatus",   Q_ARG(QVariant, statusText));
            QCoreApplication::processEvents();
        }
    }

    if (progressDialog_) {
        QMetaObject::invokeMethod(progressDialog_, "setProgress", Q_ARG(QVariant, 1.0));
        QMetaObject::invokeMethod(progressDialog_, "setStatus",   Q_ARG(QVariant, tr("Processing completed!")));
    }

    emit fileStopsOpening2(vec_CSV, minZ_, maxZ_);
}


void DeviceManager::openFileData_tsl3(QByteArray &tslByteArray, int fileIndex, int fileCnt)
{
    tslByteArray.remove(0, 512);  /*- 清除文件头信息，只保留声呐数据 -*/
    /*-将去掉文件头的所有剩下的声呐数据，按照一帧一帧的模式放入临时容器-*/
    QList<QByteArray> tslByteList;
    int nowIndex = 0;
    int byteCount = 0;
    int tslWCnt = tslByteArray.count();
    int progressInterval = qMax(1, tslWCnt / 3);
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
    QVector<float> vec_CSV;
    QList<LLA> track;
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
        if(lla.latitude < 0.000001f && lla.longitude < 0.000001f) {
            continue;
        }
        lla.altitude  = tslSingleStru.auxInfo.depth / 100.f;

        buffer.append(lla);
        int bufSize = buffer.size();
        if (bufSize < MEDIAN_WINDOW) {
            // 窗口还没满的时候：为了保证轨迹点数量一致，将前 (W/2) 个点直接推入 track
            if (bufSize <= MEDIAN_WINDOW * 0.5) {
                minZ_ = std::min(minZ_, lla.altitude);
                maxZ_ = std::max(maxZ_, lla.altitude);
                vec_CSV.append(lla.altitude);
                track.append(lla);
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
            vec_CSV.append(targetLla.altitude);
            track.append(targetLla);
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
        float depth = tslSingleStru.auxInfo.depth;
        if(tslSingleStru.ping.frequency == snrFrq455) {
            if(depth > 10000) {
                depth = 0;
            }
            else if(depth > 30000) {
                depth = 0;
            }
        }

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
        emit chartComplete(batchChannelId_, chartParams, dataVec, true);

        // qDebug() << "lla.latitude " << lla.latitude << "  " << lla.longitude << "  " << lla.altitude;
        bool enableRender = (i + 1) == tsl3Cnt ? true : false;
        emit positionComplete_file(lla.latitude, lla.longitude, lla.altitude, enableRender);

        // 更新进度条
        if (progressDialog_ && (i % progressInterval == 0 || i == (tsl3Cnt - 1))) {
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
        QMetaObject::invokeMethod(progressDialog_, "setStatus",   Q_ARG(QVariant, tr("Processing completed!")));
    }

    emit fileStopsOpening2(vec_CSV, minZ_, maxZ_);
}


void DeviceManager::closeFile()
{
    delAllDev();
    vru_.cleanVru();
    emit vruChanged();
}

void DeviceManager::onLinkOpened(QUuid uuid, Link *link)
{
    if (link) {
        if (link->getIsProxy()) {
            proxyLinkUuid_ = uuid;
            connect(this, &DeviceManager::writeProxyFrame, link, &Link::writeFrame);
        }
    }
}

void DeviceManager::onLinkClosed(QUuid uuid, Link *link)
{
    Q_UNUSED(uuid);

    if (link) {
        this->disconnect(link);
        otherProtocolStat_.remove(uuid);
        if(uuid == mavlinUuid_) {
            mavlinUuid_ = QUuid();
        }
    }
}

void DeviceManager::onLinkDeleted(QUuid uuid, Link *link)
{
    Q_UNUSED(uuid);

    if (link) {
        this->disconnect(link);
        otherProtocolStat_.remove(uuid);
        if(uuid == mavlinUuid_) {
            mavlinUuid_ = QUuid();
        }
    }
}

void DeviceManager::binFrameOut(Parsers::ProtoBinOut protoOut)
{
    emit sendProtoFrame(protoOut);
}

void DeviceManager::setProtoBinConsoled(bool isConsoled)
{
    isConsoled_ = isConsoled;
}

void DeviceManager::beaconActivationReceive(uint8_t id) {
    Q_UNUSED(id)
}

void DeviceManager::beaconDirectQueueAsk() {
}

void DeviceManager::setUSBLBeaconDirectAsk(bool is_ask) {
    isUSBLBeaconDirectAsk = is_ask;
    qDebug("Beacon auto scan is: %d", is_ask);
    if(is_ask == true) {
        if (!beacon_timer) {
            beacon_timer = new QTimer(this);
            QObject::connect(beacon_timer, &QTimer::timeout, this, &DeviceManager::beaconDirectQueueAsk);
        }
        beacon_timer->setInterval(3000);
        beacon_timer->start();
    } else {
        if (beacon_timer) {
            beacon_timer->stop();
        }
    }
}

StreamListModel* DeviceManager::streamsList()
{
    return streamList_.streamsList();
}

void DeviceManager::onStartUpgradingFirmware(QUuid linkUuid, uint8_t address, const QByteArray& firmware)
{
    upgradeUuid_ = linkUuid;
    upgradeAddr_ = address;
    upgradeData_ = firmware;
}

void DeviceManager::onUpgradingFirmwareDone()
{
    upgradeUuid_ = QUuid();
    upgradeAddr_ = 0;
    upgradeData_.clear();
}

void DeviceManager::createLocationReader()
{
    if (locReader_) {
        return;
    }

    locReader_ = new LocationReader(this);
    connect(locReader_, &LocationReader::positionUpdated, this, &DeviceManager::onPositionUpdated, Qt::QueuedConnection);
}

void DeviceManager::destroyLocationReader()
{
    if (!locReader_) {
        return;
    }

    locReader_->deleteLater();
    locReader_ = nullptr;
}

void DeviceManager::shutdown()
{
    destroyLocationReader();
}

void DeviceManager::onPositionUpdated(const QGeoPositionInfo &info)
{
    IDBinNav::SimpleNav smplNav;
    smplNav.latitude = info.coordinate().latitude();
    smplNav.longitude = info.coordinate().longitude();
    smplNav.depth = 0;
    smplNav.yaw = info.attribute(QGeoPositionInfo::Attribute::Direction) ;
    smplNav.pitch = 0;
    smplNav.roll = 0;

    emit positionComplete(smplNav.latitude, smplNav.longitude, info.timestamp().toSecsSinceEpoch(), info.timestamp().toMSecsSinceEpoch());
    emit attitudeComplete(smplNav.yaw, 0.0, 0.0);
}

void DeviceManager::delAllDev()
{
    QList<QUuid> keysToDelete;

}
