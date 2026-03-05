#include "blemanager.h"
#include <QDebug>
#include <QDateTime>
#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

// #include "serialport.h"


// FiFoTModem fifoTModem_;
quint64 totalRxCount = 0;


RealTimeParser::RealTimeParser(QObject *parent) : QObject(parent)
{
    timer_ = new QTimer(this);
    timer_->setInterval(30); //每 30ms 解析一次
    connect(timer_, &QTimer::timeout, this, &RealTimeParser::processQueue, Qt::QueuedConnection);
}


RealTimeParser::~RealTimeParser()
{
    stop();
}


const BoatPoint& RealTimeParser::getCurrentBoatPoint()
{
    return lastBoatPt_;
}

void RealTimeParser::setHasLast(bool haslast)
{
    hasLast_ = haslast;
}


QStringList RealTimeParser::splitQString(const QString &str,char delimiter)
{
    QStringList tokens;
    tokens.reserve(15);
    const QChar* tokenStart = str.constData();
    const QChar* p = tokenStart;
    const QChar* endPos = tokenStart+ str.size();
    for (; p != endPos; ++p) {
        if (*p == delimiter) {
            if (p > tokenStart) {
                tokens.append(QStringView(tokenStart, p - tokenStart).toString());
            } else {
                tokens.append(QString());
            }
            tokenStart = p + 1;
            continue;
        }
    }
    if (p > tokenStart) {
        tokens.append(QStringView(tokenStart,p-tokenStart).toString());
    } else {
        tokens.append(QString());
    }

    return tokens;
}

void RealTimeParser::enqueueData(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    m_buffer.append(data);
}

void RealTimeParser::start()
{
    if (m_running) return;
    m_running = true;

    timer_->start();
}

void RealTimeParser::stop()
{
    if (!m_running) return;
    m_running = false;
    timer_->stop();
    emit stopped();

    // deleteLater 会在所属线程事件循环安全删除定时器（this 的子对象）
    // 如果有需要可以显式停止/删除
}

void RealTimeParser::processQueue()
{
    count_++;

    QByteArray buffer;
    {
        QMutexLocker locker(&m_mutex);
        if (m_buffer.isEmpty()) return;
        buffer.swap(m_buffer);
    }

    while (true)
    {
        int startIndex = buffer.indexOf('$');
        if (startIndex == -1) break;
        int endIndex = buffer.indexOf('\n', startIndex);
        if (endIndex == -1) break;

        QByteArray sentence = buffer.mid(startIndex, endIndex - startIndex + 1);
        buffer.remove(0, endIndex + 1);
        if (!verifyChecksum(sentence))  continue;

        BoatPoint pt;
        QString str = QString::fromLatin1(sentence);
        QStringList fields = splitQString(str, ',');


        // ----------- 1.1 提取时间time（所有语句第1字段）-----------
        QString utcTimeStr,timeText;  // hhmmss(原始),转化后
        if (fields.size() > 1) {
            QString t = fields[1].trimmed();
            if (t.size() >= 6)
                utcTimeStr = t.mid(0, 6);
        }
        // ----------- 1.2 格式化时间time HH:mm:ss -----------
        if ((utcTimeStr.length() >= 6) && (utcTimeStr != "000000")) {
            QString hh = utcTimeStr.mid(0, 2);
            QString mm = utcTimeStr.mid(2, 2);
            QString ss = utcTimeStr.mid(4, 2);
            timeText = QString("%1:%2:%3").arg(hh).arg(mm).arg(ss);
            lastTimeStr_ = timeText;
        } else {
            timeText = lastTimeStr_;
        }

        // ----------- 2. RMC 解析日期date -----------
        if (sentence.startsWith("$GPRMC") || sentence.startsWith("$GNRMC")) {
            if (fields.size() > 9) {
                QString d = fields[9].trimmed();  // ddmmyy
                if (d.size() == 6) {
                    // 转换成 yyyy-MM-dd
                    QString day   = d.mid(0, 2);
                    QString month = d.mid(2, 2);
                    QString year  = "20" + d.mid(4, 2);
                    lastDateStr_  = QString("%1-%2-%3").arg(year).arg(month).arg(day);
                }
            }
        }

        pt.time = timeText;
        pt.date = lastDateStr_;   // 如果没有日期，会为空，直到收到RMC才更新

        if (sentence.startsWith("$GPGGA") || sentence.startsWith("$GNGGA"))
        {
            if (fields.size() > 5) {
                pt.latitude  = parseNMEACoordinate(fields[2].toLatin1(), fields[3]);
                pt.longitude = parseNMEACoordinate(fields[4].toLatin1(), fields[5]);
                pt.heading     = lastBoatPt_.heading;
                pt.speed     = lastBoatPt_.speed;
                pt.depth     = lastBoatPt_.depth;
            }
        }
        else if (sentence.startsWith("$GPRMC") || sentence.startsWith("$GNRMC"))
        {
            if (fields.size() > 8) {
                pt.latitude  = parseNMEACoordinate(fields[3].toLatin1(), fields[4]);
                pt.longitude = parseNMEACoordinate(fields[5].toLatin1(), fields[6]);
                pt.speed     = fields[7].toDouble() * 0.514444; // knot -> m/s
                pt.heading     = fields[8].toDouble();
                pt.depth     = lastBoatPt_.depth;
            }
        }
        else if (sentence.startsWith("$SDDBT"))
        {
            if (fields.size() > 3 && !fields[3].isEmpty()) {
                pt.depth     = fields[3].toDouble();
                pt.latitude  = lastBoatPt_.latitude;
                pt.longitude = lastBoatPt_.longitude;
                pt.heading     = lastBoatPt_.heading;
                pt.speed     = lastBoatPt_.speed;
                pt.time      = lastBoatPt_.time;
                pt.date      = lastBoatPt_.date;
            }
        }
        else {
            continue;
        }

        if (qAbs(pt.latitude) < 0.001 || qAbs(pt.longitude) < 0.001)
            continue;

        if (hasLast_) {
            double dlat = qAbs(pt.latitude - lastBoatPt_.latitude);
            double dlon = qAbs(pt.longitude - lastBoatPt_.longitude);
            if (dlat > 0.1 || dlon > 0.1) {
                qDebug() << "Abnormal GPS jump detected!";
                continue;
            }
        }

        lastBoatPt_ = pt;
        hasLast_ = true;

        // qDebug() << "  pt.depth:" << pt.depth << "   pt.speed:" << pt.speed
        //          << "  " << pt.latitude << "  " << pt.longitude;
        emit parsedPoint(pt);
    }

    if (!buffer.isEmpty()) {
        QMutexLocker locker(&m_mutex);
        m_buffer.prepend(buffer);
    }
}


double RealTimeParser::parseNMEACoordinate(const QString &coord, const QString &direction)
{
    if (coord.isEmpty()) return 0.0;

    int dotIndex = coord.indexOf('.');

    if (dotIndex < 2) return 0.0;

    QString degreesStr = coord.left(dotIndex - 2);

    QString minutesStr = coord.mid(dotIndex - 2);

    double degrees = degreesStr.toDouble();
    double minutes = minutesStr.toDouble();

    double result = degrees + minutes / 60.0;

    // 根据方向调整正负
    if (direction == "S" || direction == "W")  {
        result = -result;
    }

    return result;
}

char RealTimeParser::calculateChecksum(const QByteArray &data)
{
    char checksum = 0;
    for (char c : data) {
        checksum ^= c;
    }
    return checksum;
}
bool RealTimeParser::verifyChecksum(const QByteArray &nmeaSentence)
{
    int checksumIndex = nmeaSentence.indexOf('*');
    if (checksumIndex == -1) {
        return false;
    }

    QByteArray data = nmeaSentence.mid(1, checksumIndex - 1);

    // 提取校验和
    QByteArray checksumStr = nmeaSentence.mid(checksumIndex + 1, 2);
    bool ok;
    int receivedChecksum = checksumStr.toInt(&ok, 16);
    if (!ok) {
        return false;
    }

    char calculatedChecksum = calculateChecksum(data);
    return calculatedChecksum == receivedChecksum;
}




/*--------------------------------------------BLEManager----------------------------------------------*/
BLEManager::BLEManager(QObject *parent) : QObject(parent)
{
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    discoveryAgent->setLowEnergyDiscoveryTimeout(5000);

    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BLEManager::onDeviceDiscovered);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BLEManager::onScanFinished);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &BLEManager::onScanCanceled);
    connect(discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this, &BLEManager::onScanError);

    loadingQuickView_ = new QQuickView();
    loadingQuickView_->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    loadingQuickView_->setSource(QUrl("qrc:/Bluetooth/loading.qml"));
    loadingQuickView_->setResizeMode(QQuickView::SizeRootObjectToView);
    loadingQuickView_->resize(75, 75);
    QMetaObject::invokeMethod(loadingQuickView_, [this]() { loadingQuickView_->hide(); }, Qt::QueuedConnection);

    parser_ = new RealTimeParser();
    parserThread_ = new QThread(this);
    parser_->moveToThread(parserThread_);
    connect(parser_, &RealTimeParser::parsedPoint, this, &BLEManager::parsedPoint);
    connect(parser_, &RealTimeParser::parsedPoint, this, &BLEManager::slot_parserRealtimePt,Qt::QueuedConnection);
    connect(parser_, &RealTimeParser::parsedPoint, this, &BLEManager::dataPanelUpdate);
    connect(parser_,&RealTimeParser::stopped, this, [this](){ parserThread_->quit(); });


    /*-----------------------------------tmodem----------------------------------*/
    // static quint8 rxBuffer[RX_FIFO_SIZE];
    // FiFoTModemInit(&fifoTModem_, rxBuffer, RX_FIFO_SIZE);

    if(!trackTimer_){
        trackTimer_ = new QTimer(this);
        connect(trackTimer_, &QTimer::timeout, this, &BLEManager::slotTrackTimeout);
    }

}

void BLEManager::translate()
{
    m_scanStatus = tr("Ready");
}


BLEManager::~BLEManager()
{
    if (parser_) {
        QMetaObject::invokeMethod(parser_, "stop", Qt::QueuedConnection);
        parser_->deleteLater();
        parser_ = nullptr;
    }

    parserThread_->quit();
    parserThread_->wait();
}


bool BLEManager::hasBlePermission()
{
#ifdef Q_OS_ANDROID
    auto scan    = QtAndroid::checkPermission("android.permission.BLUETOOTH_SCAN");
    auto connect = QtAndroid::checkPermission("android.permission.BLUETOOTH_CONNECT");

    return scan == QtAndroid::PermissionResult::Granted && connect == QtAndroid::PermissionResult::Granted;
#else
    return true;
#endif
}

void BLEManager::doStartScan()
{
    if(!discoveryAgent) return;

    if(discoveryAgent->isActive()) {
       discoveryAgent->stop();
    }

    m_devices.clear();
    devicesList_.clear();

    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    setScanStatus(tr("Scanning..."));
}


void BLEManager::startStopScan(bool scan)
{
    if (!scan) {
        if (discoveryAgent && discoveryAgent->isActive()) {
            discoveryAgent->stop();
            setScanStatus(tr("Scan has Stop"));
            QMetaObject::invokeMethod(parser_, "stop", Qt::QueuedConnection);
        }
        return;
    }

#ifdef Q_OS_ANDROID
    if (!hasBlePermission()) {
        // qDebug() << "Requesting BLE permissions...";
        QtAndroid::requestPermissions(QStringList() << "android.permission.BLUETOOTH_SCAN"
                                                    << "android.permission.BLUETOOTH_CONNECT"
                                                    << "android.permission.ACCESS_FINE_LOCATION",
            [this](const QtAndroid::PermissionResultMap &result) {
                bool granted = true;
                for (auto it = result.begin(); it != result.end(); ++it) {
                    if (it.value() != QtAndroid::PermissionResult::Granted)
                        granted = false;
                }

                if (granted) {
                    qDebug() << "BLE permission granted";
                    // 必须回到 Qt 主线程  !!!
                    QMetaObject::invokeMethod(this, [this]() { doStartScan(); }, Qt::QueuedConnection);
                }
                else {
                    qDebug() << "BLE permission denied";
                    setScanStatus(tr("Permission denied"));
                }
            });

        return;
    }
#endif

    doStartScan();  // 已有权限，直接启动
}

void BLEManager::setBleLiveScanningVisible(bool visible)
{
    liveScanVisble_ = visible;
    if(visible) {
        qDebug() << "BLEManager::setBleLiveScanning............." << visible;
        if(!connected()) {
            startStopScan(true);
        }
    }


    // //nie:test
    // if(visible) {
    //     QString filePath = "E:/myQtProject/tsl_test_data/data of copy of tst.csv";
    //     QFile file;
    //     const QUrl url(filePath);
    //     url.isLocalFile() ? file.setFileName(url.toLocalFile()) : file.setFileName(url.toString());
    //     if (!file.open(QIODevice::ReadOnly)) {
    //         return;
    //     }

    //     track_.clear();

    //     QTextStream in(&file);
    //     int skip_rows = 2;

    //     QVector<float> vec_CSV;
    //     double minZ = 0.0, maxZ = 0.0;
    //     while (!in.atEnd()) {
    //         QString row = in.readLine();
    //         if (skip_rows > 0) {
    //             skip_rows--;
    //             continue;
    //         }

    //         QStringList columns = row.split(",");
    //         track_.append(Position());

    //         track_.last().lla.latitude  = columns[5].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
    //         track_.last().lla.longitude = columns[4].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
    //         track_.last().lla.altitude  = columns[6].replace(QLatin1Char(','), QLatin1Char('.')).toDouble();

    //         minZ = std::min(minZ, track_.last().lla.altitude);
    //         maxZ = std::max(maxZ, track_.last().lla.altitude);
    //         vec_CSV.append(track_.last().lla.altitude);
    //         // qDebug() << " track.last().lla.longitude:" << track.last().lla.longitude << "   latitude:" << track.last().lla.latitude;
    //         // emit positionComplete_CSV(track.last().lla.latitude, track.last().lla.longitude,track.last().lla.altitude);
    //     }
    //     qDebug() << "vec_CSV.size()........." << vec_CSV.size();

    //     file.close();

    //     trackTimer_->start(50);
    // }

}

void BLEManager::slotTrackTimeout()
{
    if (currentTrackIndex_ >= track_.size()) {
        trackTimer_->stop();
        qDebug() << "Simulated track finished";
        return;
    }

    const Position& pos = track_[currentTrackIndex_];

    emit positionComplete(pos.lla.latitude, pos.lla.longitude, pos.lla.altitude);

    // emit depthComplete(static_cast<float>(pos.lla.altitude)); // 深度信号

    currentTrackIndex_++;
}

void BLEManager::slot_parserRealtimePt(const BoatPoint &pt)
{
    // qDebug() << "pt..........." << pt.latitude << "  " << pt.longitude;
    emit positionComplete(pt.latitude, pt.longitude, pt.depth);

    depthHistory_.append(static_cast<float>(pt.depth));
    
    // 更新深度范围
    if (depthHistory_.size() == 1) {
        // 初始化深度范围
        minDepth_ = pt.depth;
        maxDepth_ = pt.depth;
    } else {
        // 更新最小深度
        if (pt.depth < minDepth_) {
            minDepth_ = pt.depth;
        }
        // 更新最大深度
        if (pt.depth > maxDepth_) {
            maxDepth_ = pt.depth;
        }
    }
    
    // 发射fileStopsOpening_CSV信号，实现实时等高线绘制
    emit fileStopsOpening_CSV(depthHistory_, minDepth_, maxDepth_);

    emit signal_drawRealtimeContour(true);
}


void BLEManager::slot_drawRealtimeContour(bool isDraw)
{
    isDrawRealtimeContour_ = isDraw;
}

void BLEManager::connectToDevice(int index)
{
#ifdef Q_OS_WIN
    QMetaObject::invokeMethod(loadingQuickView_, [this](){ loadingQuickView_->show(); }, Qt::QueuedConnection);
#endif
    if(index < 0 || index >= devicesList_.size()) {
        return;
    }
    QBluetoothDeviceInfo chooseDevice = devicesList_.at(index);
    if (bleController_) {
        bleController_->disconnectFromDevice();
        bleController_->deleteLater();
        bleController_ = nullptr;
    }
    bleController_ = QLowEnergyController::createCentral(chooseDevice, this);

    connect(bleController_, &QLowEnergyController::serviceDiscovered, this, &BLEManager::onServiceDiscovered);
    connect(bleController_, &QLowEnergyController::discoveryFinished, this, &BLEManager::onServiceScanDone);
    connect(bleController_, &QLowEnergyController::connected, this, &BLEManager::onDeviceConnected);
    connect(bleController_, &QLowEnergyController::disconnected, this, &BLEManager::onDeviceDisconnected);
    connect(bleController_, QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),this, &BLEManager::onControllerError);

    bleController_->connectToDevice();

    if(discoveryAgent->isActive()){
        discoveryAgent->stop();
    }

}

void BLEManager::disconnectDevice()
{
    if(bleServer_) {
        bleServer_->disconnect();
        bleServer_->deleteLater();
        bleServer_ = nullptr;
    }
    if (bleController_) {
        bleController_->disconnectFromDevice();
    }

    QMetaObject::invokeMethod(parser_, "stop", Qt::QueuedConnection);
}


void BLEManager::sendData(const QString &data)
{
    if (!bleServer_) {
        return;
    }

    QByteArray byteData = data.toUtf8();
    bleServer_->writeCharacteristic(m_writeCharacteristic,byteData, m_writeMode);
}

void BLEManager::sendData2(const QByteArray &data)
{
    if(!bleServer_) {
        return;
    }
    bleServer_->writeCharacteristic(m_writeCharacteristic,data,m_writeMode);
}

void BLEManager::clearRealData()
{
    parser_->setHasLast(false);
}

QStringList BLEManager::getDeviceNames() const
{
    QStringList names;
    return names;
}

void BLEManager::setMaxDepthValue(double depth)
{
    m_maxDepthValue = depth > m_maxDepthValue ? depth : m_maxDepthValue;
    emit maxDepthValueChanged();
    emit resetCurrentDepthVal();
}

void BLEManager::resetCurrentDepthValue(double)
{
    m_maxDepthValue = 0.0;
    emit maxDepthValueChanged();
    emit resetCurrentDepthVal();
}

void BLEManager::onDeviceDiscovered(const QBluetoothDeviceInfo &deviceInfo)
{
    if (deviceInfo.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        QString deviceName = deviceInfo.name();
        QString deviceAddress = deviceInfo.address().toString();

        // 检查是否已存在该设备
        bool deviceExists = false;
        for (auto& existingDevice : devicesList_) {
            if (existingDevice.address() == deviceInfo.address()) {
                deviceExists = true;
                break;
            }
        }

        if (!deviceExists) {
            QString displayText = QString("%1 : %2").arg(deviceName.isEmpty() ? "未知设备" : deviceName,deviceAddress);
            // qDebug() << "displayText........." << displayText;
            if(displayText.startsWith("XR500")){
                devicesList_.append(deviceInfo);
                m_devices.append(displayText);
                emit devicesChanged(); //发给qml
            }
        }

    } 

}

void BLEManager::onScanFinished()
{
    int deviceCnt = m_devices.size();
    setScanStatus(QString(tr("find  %1 devices")).arg(deviceCnt));
    if(deviceCnt == 0) {
        m_devices.append("No Devices Found");
        emit devicesChanged(); //发给qml
    }
}

void BLEManager::onScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    qDebug() << "扫描错误:" << error << discoveryAgent->errorString();
    setScanStatus("扫描错误: " + discoveryAgent->errorString());
}

void BLEManager::onScanCanceled()
{
    setScanStatus("scan has canceled");
}

void BLEManager::onDeviceConnected()
{
    bleController_->discoverServices();

    // startContourSignal();
}

void BLEManager::onDeviceDisconnected()
{
    m_connected = false;
    emit connectedChanged(m_connected);

    if (bleServer_) {
        bleServer_->deleteLater();
        bleServer_ = nullptr;
    }

    // stopContourSignal();
}

void BLEManager::onControllerError(QLowEnergyController::Error error)
{
    qDebug() << "控制器错误:" << error;
}

void BLEManager::onServiceDiscovered(const QBluetoothUuid &uuid)
{
    // qDebug() << "发现服务 uuid:" << uuid.toString();
}


void BLEManager::onServiceScanDone()
{
    QMetaObject::invokeMethod(loadingQuickView_, [this]() {
        loadingQuickView_->hide();
        m_connected = true;
        emit connectedChanged(m_connected);
    }, Qt::QueuedConnection);

    if(bleServer_) {
        bleServer_->disconnect();
        bleServer_->deleteLater();
        bleServer_ = nullptr;
    }
    //默认指定UUID服务：这个serviceUuid就是BLE调试软件中显示的Unknown Service的UUID。
    // const QLatin1String serviceUuid("{00001801-0000-1000-8000-00805f9b34fb}");
    // const QLatin1String serviceUuid("{00001800-0000-1000-8000-00805f9b34fb}");
    const QLatin1String serviceUuid("{0000baf0-0000-1000-8000-00805f9b34fb}");
    bleServer_ = bleController_->createServiceObject(QBluetoothUuid(serviceUuid),this);
    if(!bleServer_) {
        qDebug() << "Service not found";
        return;
    }

    qDebug() << "BLEManager::onServiceScanDone().........";

    parserThread_->start();
    QMetaObject::invokeMethod(parser_, "start", Qt::QueuedConnection);

    connect(bleServer_, &QLowEnergyService::stateChanged, this,&BLEManager::serviceStateChanged);
    connect(bleServer_, &QLowEnergyService::characteristicChanged, this,&BLEManager::BleServiceCharacteristicChanged);
    connect(bleServer_, &QLowEnergyService::characteristicRead, this,&BLEManager::BleServiceCharacteristicRead);
    connect(bleServer_, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)), this,
            SLOT(BleServiceCharacteristicWrite(QLowEnergyCharacteristic,QByteArray)));

    if(bleServer_->state() == QLowEnergyService::DiscoveryRequired) {
        //触发对其包含的服务，特征和描述符的发现
        QTimer::singleShot(500, this, [this]() { bleServer_->discoverDetails(); });
    } else {
        searchCharacteristic();
    }
}

void BLEManager::searchCharacteristic()
{
    if(!bleServer_){
        return;
    }
    if (bleServer_->state() != QLowEnergyService::ServiceDiscovered) {
        qDebug() << "Service not discovered, current state:" << bleServer_->state();
        return;
    }

    const QBluetoothUuid READ_WRITE_UUID(QStringLiteral("{0000baf1-0000-1000-8000-00805f9b34fb}"));
    const QBluetoothUuid READ_UUID(QStringLiteral("{0000ABF2-0000-1000-8000-00805F9B34FB}"));
    const QBluetoothUuid WRITE_UUID(QStringLiteral("{0000BAF1-0000-1000-8000-00805F9B34FB}"));
    QList<QLowEnergyCharacteristic> list = bleServer_->characteristics();
    // qDebug() << "list.count()= " << list.count();
    for(int i = 0;i < list.count(); i++)
    {
        QLowEnergyCharacteristic c = list.at(i);
        if(c.uuid() == READ_UUID) {
            // 读取
            if(c.properties() & QLowEnergyCharacteristic::Notify)
            {
                m_notificationDesc = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if(m_notificationDesc.isValid()) {
                    bleServer_->writeDescriptor(m_notificationDesc,QByteArray::fromHex("0100"));
                }
            }
        }
        else if(c.uuid() == WRITE_UUID) {
            //写入
            if(c.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
                m_writeCharacteristic = c;
                m_writeMode = QLowEnergyService::WriteWithoutResponse;
            }
        }

    }
}

void BLEManager::SendMsg(QString text)
{
    QByteArray array = text.toUtf8();

    bleServer_->writeCharacteristic(m_writeCharacteristic,array, m_writeMode);
}


void BLEManager::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    if(s == QLowEnergyService::ServiceDiscovered) {
        searchCharacteristic();
    }
}


//读取到数据
void BLEManager::BleServiceCharacteristicChanged(const QLowEnergyCharacteristic &c,const QByteArray &value)
{
    // qDebug() << "characteristic Changed signals emisison.......";
    // qDebug() << value;


        // totalRxCount += value.size();
        // FiFoTModemWrite(&fifoTModem_, reinterpret_cast<const quint8*>(value.data()), value.size());



    bleCount_++;
    qint64 now = recvTimer_.elapsed();
    qint64 interval = now - lastElapsed_;
    lastElapsed_ = now;
    Q_UNUSED(c)

    // 将原始数据通过 queued connection 交给 parser 入队，尽量不阻塞主线程
    QMetaObject::invokeMethod(parser_, "enqueueData", Qt::QueuedConnection, Q_ARG(QByteArray, value));
}

void BLEManager::BleServiceCharacteristicRead(const QLowEnergyCharacteristic &c,const QByteArray &value)
{
    // qDebug() << "characteristic Read signals emission" << QString(value);
    Q_UNUSED(c)
}


//如果写尝试成功,characteristicWritten()发出信号，下面槽响应。
void BLEManager::BleServiceCharacteristicWrite(const QLowEnergyCharacteristic &c,const QByteArray &value)
{
    // qDebug() << "characteristic Written signals emission" << QString(value);
    Q_UNUSED(c)
}

void BLEManager::setScanStatus(const QString &status)
{
    if (m_scanStatus != status) {
        m_scanStatus = status;
        emit scanStatusChanged(status);
    }
}
