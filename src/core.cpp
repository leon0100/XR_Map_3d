#include "core.h"

#include <ctime>
#include "bottom_track.h"
#include "hotkeys_manager.h"
#ifdef Q_OS_WINDOWS
#include <Windows.h>
#endif


Core::Core() : QObject(),
    consolePtr_(new Console),
    deviceManagerWrapperPtr_(std::make_unique<DeviceManagerWrapper>(this)),
    dataProcessor_(nullptr),
    dataProcThread_(nullptr),
    dataHorizon_(std::make_unique<DataHorizon>()),
    qmlAppEnginePtr_(nullptr),
    datasetPtr_(new Dataset),
    scene3dViewPtr_(nullptr),
    openedfilePath_(),
    filePath_(),
    isFileOpening_(false),
    isGPSAlive_(false)
{
    qRegisterMetaType<uint8_t>("uint8_t");
    createControllers();
    logger_.setDatasetPtr(datasetPtr_);
    createDeviceManagerConnections();
    createDatasetConnections();

    createDataProcessor();
}

Core::~Core()
{
    destroyDataProcessor();
}

void Core::setEngine(QQmlApplicationEngine *engine)
{
    qmlAppEnginePtr_ = engine;
    QObject::connect(qmlAppEnginePtr_, &QQmlApplicationEngine::objectCreated, this, &Core::UILoad, Qt::QueuedConnection);

    qmlAppEnginePtr_->rootContext()->setContextProperty("BoatTrackControlMenuController",       boatTrackControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("NavigationArrowControlMenuController", navigationArrowControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("BottomTrackControlMenuController",     bottomTrackControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("IsobathsViewControlMenuController",    isobathsViewControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("MosaicViewControlMenuController",      mosaicViewControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("ImageViewControlMenuController",       imageViewControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("MapViewControlMenuController",         mapViewControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("PointGroupControlMenuController",      pointGroupControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("Scene3DControlMenuController",         scene3dControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("Scene3dToolBarController",             scene3dToolBarController_.get());

    qmlAppEnginePtr_->rootContext()->setContextProperty("BleManager",      bleManager_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("UdpManager",      udpManager_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("SerialPort",      serialPortManager_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("Locations",       locations_.get());

    // ── 注册 dataProcessor ──
    qmlAppEnginePtr_->rootContext()->setContextProperty("dataProcessor", dataProcessor_);

    bool flasherState = false;
#ifdef FLASHER
    flasherState = true;
#endif

    qmlAppEnginePtr_->rootContext()->setContextProperty("FLASHER_STATE", flasherState);
}

Console* Core::getConsolePtr()
{
    return consolePtr_;
}

Dataset* Core::getDatasetPtr()
{
    return datasetPtr_;
}

DataProcessor* Core::getDataProcessorPtr() const
{
    return dataProcessor_;
}

DeviceManagerWrapper* Core::getDeviceManagerWrapperPtr() const
{
    return deviceManagerWrapperPtr_.get();
}

void Core::refreshMap(LLA lla)
{
    if(!scene3dViewPtr_) {
        qDebug() << "refreshMap: scene3dViewPtr_ null";
        return;
    }
    std::weak_ptr<GraphicsScene3dView::Camera> camera = scene3dViewPtr_->camera();
    if (auto cameraShared = camera.lock();cameraShared) {
        cameraShared->setYerevanLla(lla);
    }

    scene3dViewPtr_->updateMapView();
}

void Core::saveCurrentMapState(std::function<void(double lat, double lon)>writer)
{
    if(!scene3dViewPtr_ || !writer) {
        return;
    }

    std::weak_ptr<GraphicsScene3dView::Camera> camera = scene3dViewPtr_->camera();
    if(auto cam = camera.lock(); cam){
        double lat = cam->viewLlaRef_.refLla.latitude;
        double lon = cam->viewLlaRef_.refLla.longitude;
        writer(lat, lon);
    }
}

void Core::consoleInfo(QString msg)
{
    getConsolePtr()->put(QtMsgType::QtInfoMsg, msg);
}

void Core::consoleWarning(QString msg)
{
    getConsolePtr()->put(QtMsgType::QtWarningMsg, msg);
}

void Core::consoleProto(FrameParser &parser, bool isIn)
{
    QString str_mode;
    QString comment = "";

    switch (parser.type()) {
        case CONTENT:
            str_mode = "DATA";
            if (parser.resp()) {
                switch(parser.frame()[6]) {
                    case respNone:          comment = "[respNone]";         break;
                    case respOk:            comment = "[respOk]";           break;
                    case respErrorCheck:    comment = "[respErrorCheck]";   break;
                    case respErrorPayload:  comment = "[respErrorPayload]"; break;
                    case respErrorID:       comment = "[respErrorID]";      break;
                    case respErrorVersion:  comment = "[respErrorVersion]"; break;
                    case respErrorType:     comment = "[respErrorType]";    break;
                    case respErrorKey:      comment = "[respErrorKey]";     break;
                    case respErrorRuntime:  comment = "[respErrorRuntime]"; break;
                    default: comment = QString("[resp %1]").arg((int)parser.frame()[6]);  break;
                }
            }
            else {
                if (parser.id() == ID_EVENT) {
                    comment = QString("Event ID %1").arg(*(uint32_t*)(&parser.frame()[10]));
                }
            }
            break;
        case SETTING:
            str_mode = "SET";
            break;
        case GETTING:
            str_mode = "GET";
            break;
        default:
            str_mode = "NAN";
            break;
    }

    QString str_dir;
    isIn ? str_dir = "-->> " : str_dir = "<<-- ";
}


void Core::openLogFile(const QString& filePath, bool isAppend, bool onCustomEvent)
{
    isFileOpening_ = true;
    emit sendIsFileOpening();

    QTimer::singleShot(15, this, [this, filePath, isAppend, onCustomEvent]()->void { // 15 ms delay
        QString localfilePath = filePath;

        if (onCustomEvent) {
            fixFilePathString(localfilePath);
            filePath_ = localfilePath;
            emit filePathChanged();
        }

        QCoreApplication::processEvents(QEventLoop::AllEvents);

        if (!isAppend) {
            resetDataProcessorConnections();
            datasetPtr_->resetDataset();
            dataHorizon_->clear();
            QMetaObject::invokeMethod(dataProcessor_, "clearProcessing", Qt::QueuedConnection);
            setDataProcessorConnections();
            dataHorizon_->setIsFileOpening(isFileOpening_);
        }

        if (scene3dViewPtr_) {
            if (!isAppend) {
                scene3dViewPtr_->clear(true);
            }
        }


        datasetPtr_->setState(Dataset::DatasetState::kFile);

        emit deviceManagerWrapperPtr_->sendOpenFile(localfilePath);

        openedfilePath_ = localfilePath;

        if (scene3dViewPtr_) {
            scene3dViewPtr_->fitAllInView();
        }
        datasetPtr_->setRefPositionByFirstValid();
        datasetPtr_->usblProcessing();

        if (scene3dViewPtr_) {
            scene3dViewPtr_->addPoints(datasetPtr_->beaconTrack(), QColor(255, 0, 0), 10);
            scene3dViewPtr_->addPoints(datasetPtr_->beaconTrack1(), QColor(0, 255, 0), 10);
        }

        onChannelsUpdated();
    });
}

bool Core::closeLogFile()
{
    // qDebug() << "Core::closeLogFile()";
    if (datasetPtr_) {
        datasetPtr_->resetRenderBuffers();
    }
    if (scene3dViewPtr_) {
        scene3dViewPtr_->clear(true);
        scene3dViewPtr_->getNavigationArrowPtr()->resetPositionAndAngle();
    }
    dataHorizon_->clear();
    QMetaObject::invokeMethod(dataProcessor_, "clearProcessing", Qt::QueuedConnection);

    if (!isOpenedFile()) {
        return false;
    }

    if (datasetPtr_) {
        datasetPtr_->resetDataset();
    }
    emit deviceManagerWrapperPtr_->sendCloseFile();
    openedfilePath_.clear();

    return true;
}


bool Core::openXTF(const QByteArray& data)
{
    datasetPtr_->setState(Dataset::DatasetState::kFile);

    const QVector<DatasetChannel> channelList = datasetPtr_->channelsList();
    if (channelList.size() < 2) {
        return false;
    }

    auto linkNames = getLinkNames();
    QString fChName;
    QString sChName;
    if (linkNames.contains(channelList.at(0).channelId_.uuid)) {
        fChName = channelList.at(0).portName_;
    }
    if (linkNames.contains(channelList.at(1).channelId_.uuid)) {
        sChName = channelList.at(0).portName_;
    }

    if (!plot2dList_.isEmpty() && plot2dList_.at(0) && channelList.size() >= 2) {
        plot2dList_.at(0)->setDataChannel(false, channelList[0].channelId_, channelList[0].subChannelId_, fChName, channelList[1].channelId_, channelList[1].subChannelId_, sChName);
        plot2dList_.at(0)->plotUpdate();
    }

    for (int i = 0; i < plot2dList_.size(); i++) {
        if (plot2dList_.at(i) != NULL && i < channelList.size()) {
            if (i == 0) {
                plot2dList_.at(i)->setDataChannel(false, channelList[0].channelId_, channelList[0].subChannelId_, fChName, channelList[1].channelId_, channelList[1].subChannelId_, sChName);
                plot2dList_.at(i)->plotUpdate();
            }
        }
    }

    return true;
}

bool Core::openCSV(QString name, int separatorType, int firstRow, int colTime,
                bool isUtcTime, int colLat, int colLon, int colAltitude, int colNorth, int colEast, int colUp)
{
    QString& filePath = name;
    if (filePath.startsWith("file:")) {
        filePath = QUrl(filePath).toLocalFile();
    }

    qDebug() << "name:................" << name;
    bool isAppend = false;
    bool onCustomEvent = false;
    isFileOpening_ = true;
    emit sendIsFileOpening();

    QTimer::singleShot(15, this, [this, filePath, isAppend, onCustomEvent]()->void { // 15 ms delay
        QString localfilePath = filePath;
        if (onCustomEvent) {
            fixFilePathString(localfilePath);
            filePath_ = localfilePath;
            emit filePathChanged();
        }

        // linkManagerWrapperPtr_->closeOpenedLinks();
        // removeLinkManagerConnections();

        QCoreApplication::processEvents(QEventLoop::AllEvents);

        if (scene3dViewPtr_) {
            if (!isAppend) {
                scene3dViewPtr_->clear(true);
            }
        }

        // QMetaObject::invokeMethod(dataProcessor_, "setIsOpeningFile", Qt::QueuedConnection, Q_ARG(bool, true));

        datasetPtr_->setState(Dataset::DatasetState::kFile);

        emit deviceManagerWrapperPtr_->sendOpenFile_CSV(localfilePath, 0, 1);

        openedfilePath_ = localfilePath;

        if (scene3dViewPtr_) {
            scene3dViewPtr_->fitAllInView();
        }
        qDebug() << "datasetPtr_->setRefPositionByFirstValid()..................";

        //nie:test暂时注释掉没有影响
        // datasetPtr_->setRefPositionByFirstValid();
        // datasetPtr_->usblProcessing();

        // if (scene3dViewPtr_) {
        //     scene3dViewPtr_->addPoints(datasetPtr_->beaconTrack(), QColor(255, 0, 0), 10);
        //     scene3dViewPtr_->addPoints(datasetPtr_->beaconTrack1(), QColor(0, 255, 0), 10);
        // }

        // onChannelsUpdated();
    });

    return true;
}

bool Core::openProxy(const QString& address, const int port, bool isTcp)
{
    Q_UNUSED(address);
    Q_UNUSED(port);
    Q_UNUSED(isTcp);

    return false;
}

bool Core::closeProxy()
{
    return false;
}

bool Core::upgradeFW(const QString& name, QObject* dev)
{
    QUrl url(name);
    QFile file(url.isLocalFile() ? url.toLocalFile() : name);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // if (auto* devQProp = dynamic_cast<DevQProperty*>(dev); devQProp) {
    //     devQProp->sendUpdateFW(file.readAll());
    // }

    return true;
}

void Core::upgradeChanged(int progressStatus)
{
    // if(progressStatus == DevDriver::successUpgrade) {
    //     //        restoreBaudrate();
    // }
}

bool Core::exportUSBLToCSV(QString filePath)
{
    QString export_file_name = isOpenedFile() ? openedfilePath_.section('/', -1).section('.', 0, 0) : QDateTime::currentDateTime().toString("yyyy.MM.dd_hh:mm:ss").replace(':', '.');

    logger_.creatExportStream(filePath + "/" + export_file_name + ".csv");

    logger_.dataExport("epoch,yaw,pitch,roll,north,east,ping_counter,carrier_counter,snr,azimuth_deg,elevation_deg,distance_m\n");

    for (int i = 0; i < datasetPtr_->size(); i += 1) {
        Epoch* epoch = datasetPtr_->fromIndex(i);

        if (epoch == NULL)
            continue;

        North_East_Down boatPosNed = epoch->getPositionGNSS().ned;

        // pos.ned.isCoordinatesValid() && epoch->isAttAvail() &&
        if( epoch->isUsblSolutionAvailable()) {
            QString row_data;

            row_data.append(QString("%1").arg(i));
            row_data.append(QString(",%1,%2,%3").arg(epoch->yaw()).arg(epoch->pitch()).arg(epoch->roll()));
            row_data.append(QString(",%1,%2").arg(boatPosNed.n).arg(boatPosNed.e));
            row_data.append(QString(",%1,%2,%3").arg(epoch->usblSolution().ping_counter).
                            arg(epoch->usblSolution().carrier_counter).arg(epoch->usblSolution().snr));
            row_data.append(QString(",%1,%2,%3").arg(epoch->usblSolution().azimuth_deg).
                            arg(epoch->usblSolution().elevation_deg).arg(epoch->usblSolution().distance_m));

            row_data.append("\n");
            logger_.dataExport(row_data);
        }
    }

    logger_.endExportStream();

    return true;
}

bool Core::exportPlotAsCVS(QString filePath, const ChannelId& channelId, float decimation)
{
    QString export_file_name = isOpenedFile() ? openedfilePath_.section('/', -1).section('.', 0, 0)
                        : QDateTime::currentDateTime().toString("yyyy.MM.dd_hh:mm:ss").replace(':', '.');
    logger_.creatExportStream(filePath + "/" + export_file_name + ".csv");

    bool meas_nbr = true;
    bool event_id = true;
    bool rangefinder = false;
    bool bottom_depth = true;
    bool pos_lat_lon = true;
    bool pos_time = true;

    bool external_pos_lla = true;
    bool external_pos_neu = true;
    bool sonar_height = true;
    bool bottom_height = true;

    bool ext_pos_lla_find = false;
    bool ext_pos_ned_find = false;

    bool contactInfo = true;
    bool contactDistance = true;

    int row_cnt = datasetPtr_->size();

    auto btP = datasetPtr_->getBottomTrackParam();
    datasetPtr_->setChannelOffset(channelId, btP.offset.x, btP.offset.y, btP.offset.z);
    datasetPtr_->spatialProcessing();

    for (int i = 0; i < row_cnt; i++) {
        Epoch* epoch = datasetPtr_->fromIndex(i);

        Position position = epoch->getExternalPosition();
        ext_pos_lla_find |= position.lla.isValid();
        ext_pos_ned_find |= position.ned.isValid();
    }

    if (meas_nbr)
        logger_.dataExport("Number,");

    if (event_id) {
        logger_.dataExport("Event UNIX,");
        logger_.dataExport("Event timestamp,");
        logger_.dataExport("Event ID,");
    }

    if (rangefinder)
        logger_.dataExport("Rangefinder,");

    if (bottom_depth)
        logger_.dataExport("Beam distance,");

    if (pos_lat_lon) {
        logger_.dataExport("Latitude,");
        logger_.dataExport("Longitude,");

        if (pos_time) {
            logger_.dataExport("GNSS UTC Date,");
            logger_.dataExport("GNSS UTC Time,");
        }
    }

    if (external_pos_lla && ext_pos_lla_find) {
        logger_.dataExport("ExtLatitude,");
        logger_.dataExport("ExtLongitude,");
        logger_.dataExport("ExtAltitude,");
    }

    if (external_pos_neu && ext_pos_ned_find) {
        logger_.dataExport("ExtNorth,");
        logger_.dataExport("ExtEast,");
        logger_.dataExport("ExtHeight,");
    }

    if (sonar_height)
        logger_.dataExport("SonarHeight,");

    if (bottom_height)
        logger_.dataExport("BottomHeight,");

    if (contactInfo) {
        logger_.dataExport("ContactTitle,");
    }
    if (contactDistance) {
        logger_.dataExport("ContactDistance");
    }

    logger_.dataExport("\n");

    int prev_timestamp = 0;
    int prev_unix = 0;
    int prev_event_id = 0;
    float prev_dist_proc = 0;
    double prev_lat = 0, prev_lon = 0;

    float decimation_m = decimation;
    float decimation_path = 0;
    LLARef lla_ref;
    North_East_Down last_pos_ned;

    for (int i = 0; i < row_cnt; i++) {
        Epoch* epoch = datasetPtr_->fromIndex(i);

        if (!epoch->contact_.isValid()) {
            if (decimation_m > 0) {
                if (!epoch->isPosAvail())
                    continue;

                Position boatPos = epoch->getPositionGNSS();

                if (boatPos.lla.isCoordinatesValid()) {
                    if (!lla_ref.isInit) {
                        lla_ref = LLARef(boatPos.lla);
                        boatPos.LLA2NED(&lla_ref);
                        last_pos_ned = boatPos.ned;
                    }
                    else {
                        boatPos.LLA2NED(&lla_ref);
                        float dif_n = boatPos.ned.n - last_pos_ned.n;
                        float dif_e = boatPos.ned.e - last_pos_ned.e;
                        last_pos_ned = boatPos.ned;
                        decimation_path += sqrtf(dif_n*dif_n + dif_e*dif_e);
                        if(decimation_path < decimation_m)
                            continue;
                        decimation_path -= decimation_m;
                    }
                }
                else {
                    continue;
                }
            }
        }

        QString row_data;

        if (meas_nbr)
            row_data.append(QString("%1,").arg(i));

        if (event_id) {
            if (epoch->eventAvail()) {
                prev_timestamp = epoch->eventTimestamp();
                prev_event_id = epoch->eventID();
                prev_unix = epoch->eventUnix();
            }
            row_data.append(QString("%1,%2,%3,").arg(prev_unix).arg(prev_timestamp).arg(prev_event_id));
        }

        if (rangefinder)
            epoch->distAvail() ? row_data.append(QString("%1,").arg((float)epoch->rangeFinder()))
                               : row_data.append("0,");

        if (bottom_depth) {
            prev_dist_proc = epoch->distProccesing(channelId);
            row_data.append(QString("%1,").arg((float)(prev_dist_proc)));
        }

        if (pos_lat_lon) {
            if (epoch->isPosAvail()) {
                prev_lat = epoch->lat();
                prev_lon = epoch->lon();
            }

            row_data.append(QString::number(prev_lat, 'f', 8));
            row_data.append(",");
            row_data.append(QString::number(prev_lon, 'f', 8));
            row_data.append(",");

            if (pos_time) {
                if (epoch->isPosAvail() && epoch->positionTimeUnix() != 0) {
                    DateTime time_epoch = *epoch->time();

                    DateTime* dt = epoch->time();
                    if (time_epoch.sec > 0) {
                        time_epoch.sec -= 18;
                        dt = &time_epoch;
                    }
                    // DateTime* dt = epoch->positionTime();
                    volatile tm t_sep = dt->getDateTime();
                    t_sep.tm_year += 1900;
                    t_sep.tm_mon += 1;

                    row_data.append(QString("%1-%2-%3").arg(t_sep.tm_year).arg(t_sep.tm_mon).arg(t_sep.tm_mday));
                    row_data.append(",");
                    row_data.append(QString("%1:%2:%3").arg(t_sep.tm_hour).arg(t_sep.tm_min)
                                        .arg((double)t_sep.tm_sec+(double)dt->nanoSec/1e9));
                    row_data.append(",");
                }
                else {
                    row_data.append(",");
                    row_data.append(",");
                }
            }
        }

        Position position = epoch->getExternalPosition();

        if (external_pos_lla && ext_pos_lla_find) {
            row_data.append(QString::number(position.lla.latitude, 'f', 10));
            row_data.append(",");
            row_data.append(QString::number(position.lla.longitude, 'f', 10));
            row_data.append(",");
            row_data.append(QString::number(position.lla.altitude, 'f', 3));
            row_data.append(",");
        }

        if (external_pos_neu && ext_pos_ned_find) {
            row_data.append(QString::number(position.ned.n, 'f', 10));
            row_data.append(",");
            row_data.append(QString::number(position.ned.e, 'f', 10));
            row_data.append(",");
            row_data.append(QString::number(-position.ned.d, 'f', 3));
            row_data.append(",");
        }

        Epoch::Echogram* sensor = epoch->chart(channelId);

        if (sonar_height) {
            if (sensor != NULL && qIsFinite(sensor->sensorPosition.ned.d)) {
                row_data.append(QString::number(-sensor->sensorPosition.ned.d, 'f', 3));
            }
            else if (sensor != NULL && qIsFinite(sensor->sensorPosition.lla.altitude)) {
                row_data.append(QString::number(sensor->sensorPosition.lla.altitude, 'f', 3));
            }
            row_data.append(",");
        }

        if (bottom_height) {
            if(sensor != NULL && qIsFinite(sensor->bottomProcessing.bottomPoint.ned.d)) {
                row_data.append(QString::number(-sensor->bottomProcessing.bottomPoint.ned.d, 'f', 3));
            }
            else if (sensor != NULL && qIsFinite(sensor->bottomProcessing.bottomPoint.lla.altitude)) {
                row_data.append(QString::number(sensor->bottomProcessing.bottomPoint.lla.altitude, 'f', 3));
            }
            row_data.append(",");
        }

        auto& contact = epoch->contact_;
        if (contact.isValid()) {
            if (contactInfo) {
                row_data.append(contact.info);
                row_data.append(",");
            }
            if (contactDistance) {
                row_data.append(QString::number(contact.echogramDistance, 'f', 4));
            }
        }

        row_data.append("\n");
        logger_.dataExport(row_data);
    }

    logger_.endExportStream();

    return true;
}

bool Core::exportPlotAsXTF(QString filePath)
{
    if (plot2dList_.empty()) {
        return false;
    }

    QString export_file_name = isOpenedFile() ? openedfilePath_.section('/', -1).section('.', 0, 0)
           : QDateTime::currentDateTime().toString("yyyy.MM.dd_hh:mm:ss").replace(':', '.');
    logger_.creatExportStream(filePath + "/_" + export_file_name + ".xtf");

    auto ch1 = plot2dList_[0]->plotDatasetChannel();
    auto subCh1 = plot2dList_[0]->plotDatasetSubChannel();
    auto ch2 = plot2dList_[0]->plotDatasetChannel2();
    auto subCh2 = plot2dList_[0]->plotDatasetSubChannel2();

    return true;
}

void Core::setPlotStartLevel(int level)
{
    for (int i = 0; i < plot2dList_.size(); i++) {
        if (plot2dList_.at(i) != NULL) {
            plot2dList_.at(i)->setEchogramLowLevel(level);
        }
    }
}

void Core::setPlotStopLevel(int level)
{
    for (int i = 0; i < plot2dList_.size(); i++) {
        if (plot2dList_.at(i) != NULL)
            plot2dList_.at(i)->setEchogramHightLevel(level);
    }
}

void Core::setTimelinePosition(double position)
{
    for (int i = 0; i < plot2dList_.size(); i++) {
        if (plot2dList_.at(i) != NULL)
            plot2dList_.at(i)->setTimelinePosition(position);
    }

    if(scene3dViewPtr_ && datasetPtr_ && datasetPtr_->size() > 0) {
        int dataSize = datasetPtr_->size();
        int epochIndex = qRound(position * (dataSize - 1));
        if(epochIndex < 0) {
            epochIndex = 0;
        }
        if(epochIndex >= dataSize) {
           epochIndex = dataSize - 1;
        }

        if(auto navArrow = scene3dViewPtr_->getNavigationArrowPtr()) {
            if(Epoch* ep = datasetPtr_->fromIndex(epochIndex); ep) {
                const Position boatPos = ep->getPositionGNSS();
                if(boatPos.ned.isCoordinatesValid()) {
                    float yawDeg = ep->yaw();
                    if(!std::isfinite(yawDeg)) {
                        yawDeg = datasetPtr_->getLastYaw();
                    }
                    if(std::isfinite(yawDeg)) {
                        navArrow->setPositionAndAngle(QVector3D(boatPos.ned.n, boatPos.ned.e,
                                   !std::isfinite(boatPos.ned.d) ? 0.0f : boatPos.ned.d), yawDeg - 90.0f);
                        if(navArrow->isVisible()) {
                            scene3dViewPtr_->ensureInView(QVector3D(boatPos.ned.n, boatPos.ned.e, 0.0f));
                        }
                    }
                }
            }
        }
    }

}

void Core::resetAim()
{
    for (int i = 0; i < plot2dList_.size(); i++) {
        if (plot2dList_.at(i) != NULL)
            plot2dList_.at(i)->resetAim();
    }
}

void Core::UILoad(QObject* object, const QUrl& url)
{
    Q_UNUSED(url)

    // loadLLARefFromSettings();//windows环境下注释掉这个吧！

#if !defined(Q_OS_ANDROID)
    HotkeysManager hotkeysManager;
    auto hotkeysMap = hotkeysManager.loadHotkeysMapping();
    auto hotkeysVariant = HotkeysManager::toVariantMap(hotkeysMap);
    qmlAppEnginePtr_->rootContext()->setContextProperty("hotkeysMapScan", hotkeysVariant);
#endif

    scene3dViewPtr_ = object->findChild<GraphicsScene3dView*>();
    plot2dList_ = object->findChildren<qPlot2D*>();
    scene3dViewPtr_->setDataset(datasetPtr_);
    locations_->setDataset(datasetPtr_);
    scene3dViewPtr_->setDataProcessorPtr(dataProcessor_);
    datasetPtr_->setScene3D(scene3dViewPtr_);
    scene3dViewPtr_->setProgressDialog(progress_);

    for (int i = 0; i < plot2dList_.size(); i++) {
        if (plot2dList_.at(i) != NULL) {
            plot2dList_.at(i)->setPlot(datasetPtr_);
            plot2dList_.at(i)->setDataProcessor(dataProcessor_);
            scene3dViewPtr_->bottomTrack()->installEventFilter(plot2dList_.at(i));
            scene3dViewPtr_->getBoatTrackPtr()->installEventFilter(plot2dList_.at(i));
            scene3dViewPtr_->getContactsPtr()->installEventFilter(plot2dList_.at(i));
            plot2dList_.at(i)->installEventFilter(scene3dViewPtr_->bottomTrack().get());
            plot2dList_.at(i)->installEventFilter(scene3dViewPtr_->getBoatTrackPtr().get());
            plot2dList_.at(i)->installEventFilter(scene3dViewPtr_->getContactsPtr().get());
        }
    }

    scene3dViewPtr_->setQmlRootObject(object);
    scene3dViewPtr_->setQmlAppEngine(qmlAppEnginePtr_);

    boatTrackControlMenuController_->setQmlEngine(object);
    boatTrackControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    navigationArrowControlMenuController_->setQmlEngine(object);
    navigationArrowControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    bottomTrackControlMenuController_->setQmlEngine(object);
    bottomTrackControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    isobathsViewControlMenuController_->setQmlEngine(object);
    isobathsViewControlMenuController_->setDataProcessorPtr(dataProcessor_);
    isobathsViewControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    mosaicViewControlMenuController_->setQmlEngine(object);
    mosaicViewControlMenuController_->setDataProcessorPtr(dataProcessor_);
    mosaicViewControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    imageViewControlMenuController_->setQmlEngine(object);
    imageViewControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    mapViewControlMenuController_->setQmlEngine(object);
    mapViewControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    // npdFilterControlMenuController_->setQmlEngine(object);
    // npdFilterControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    // mpcFilterControlMenuController_->setQmlEngine(object);
    // mpcFilterControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    pointGroupControlMenuController_->setQmlEngine(object);
    pointGroupControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    // polygonGroupControlMenuController_->setQmlEngine(object);
    // polygonGroupControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    scene3dToolBarController_->setQmlEngine(object);
    scene3dToolBarController_->setDataProcessorPtr(dataProcessor_);
    scene3dToolBarController_->setGraphicsSceneView(scene3dViewPtr_);

    scene3dControlMenuController_->setQmlEngine(object);
    scene3dControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    // usblViewControlMenuController_->setQmlEngine(object);
    // usblViewControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

    onChannelsUpdated();

    createMapTileManagerConnections();
    createScene3dConnections();

    QMetaObject::invokeMethod(dataProcessor_, "setBottomTrackPtr", Qt::QueuedConnection,
                              Q_ARG(BottomTrack*, scene3dViewPtr_->bottomTrack().get()));
    QMetaObject::invokeMethod(deviceManagerWrapperPtr_->getWorker(), "createLocationReader", Qt::QueuedConnection);
}

void Core::setMosaicChannels(const QString& firstChStr, const QString& secondChStr)
{
    qDebug() << "Core::setMosaicChannels..................";
    if (datasetPtr_ && dataProcessor_ && scene3dViewPtr_) {
        auto [ch1, sub1, name1] = datasetPtr_->channelIdFromName(firstChStr);
        auto [ch2, sub2, name2] = datasetPtr_->channelIdFromName(secondChStr);

        Q_UNUSED(name1)
        Q_UNUSED(name2)

        QMetaObject::invokeMethod(dataProcessor_, "setMosaicChannels", Qt::QueuedConnection,
                Q_ARG(ChannelId, ch1), Q_ARG(uint8_t, sub1), Q_ARG(ChannelId, ch2), Q_ARG(uint8_t, sub2));
    }
}

bool Core::getIsFileOpening() const
{
    return isFileOpening_;
}

bool Core::getIsSeparateReading() const
{
    return false;
}

void Core::onChannelsUpdated()
{
    // qDebug() << "Core::onChannelsUpdated()...................";
    auto chs = datasetPtr_->channelsList();
    int chSize = chs.size();
    if (!chSize) {
        fChName_.clear();
        sChName_.clear();
        emit channelListUpdated();
        return;
    }

    QString fChName;
    QString sChName;

    if (openedfilePath_.isEmpty()) {
        auto linkNames = getLinkNames();
        // if (chSize > 0 && linkNames.contains(chs[0].channelId_.uuid)) {
        //     fChName = chs[0].portName_;
        // }
        // if (chSize > 1 && linkNames.contains(chs[1].channelId_.uuid)) {
        //     sChName = chs[1].portName_;
        // }
        if (chSize > 0) {
            if (linkNames.contains(chs[0].channelId_.uuid)) {
                fChName = chs[0].portName_;
            } else if (!chs[0].portName_.isEmpty()) {
                fChName = chs[0].portName_;
            }
        }
        if (chSize > 1) {
            if (linkNames.contains(chs[1].channelId_.uuid)) {
                sChName = chs[1].portName_;
            } else if (!chs[1].portName_.isEmpty()) {
                sChName = chs[1].portName_;
            }
        }
    }
    else {
        if (chSize > 0) {
            fChName = chs[0].portName_;
        }
        if (chSize > 1) {
            sChName = chs[1].portName_;
        }
    }

    // if (fChName.isEmpty() && sChName.isEmpty()) {
    //     return;
    // }
    if (fChName.isEmpty() && sChName.isEmpty() && chSize > 0 && chs[0].portName_.isEmpty()) {
        return;
    }
    if (fChName.isEmpty() && chSize > 0) {
        fChName = chs[0].portName_;
    }
    if (sChName.isEmpty() && chSize > 1) {
        sChName = chs[1].portName_;
    }

    const int numPlots = plot2dList_.size();
    for (int i = 0; i < numPlots; i++) {
        if (chSize >= 2) {
            plot2dList_.at(i)->setDataChannel(false, chs[0].channelId_, chs[0].subChannelId_, fChName, chs[1].channelId_, chs[1].subChannelId_, sChName);
            plot2dList_.at(i)->plotUpdate();
            fChName_ = QString("%1|%2|%3").arg(fChName, QString::number(chs[0].channelId_.address), QString::number(chs[0].subChannelId_));
            sChName_ = QString("%1|%2|%3").arg(sChName, QString::number(chs[1].channelId_.address), QString::number(chs[1].subChannelId_));
        }
        if (chSize == 1) {
            plot2dList_.at(i)->setDataChannel(false, chs[0].channelId_, chs[0].subChannelId_, fChName);
            plot2dList_.at(i)->plotUpdate();
            fChName_ = QString("%1|%2|%3").arg(fChName, QString::number(chs[0].channelId_.address), QString::number(chs[0].subChannelId_));
        }
    }

    emit channelListUpdated();
}

QString Core::getChannel1Name() const
{
    return fChName_;
}

QString Core::getChannel2Name() const
{
    return sChName_;
}

QVariant Core::getConvertedMousePos(int indx, int mouseX, int mouseY)
{
    // qDebug() << "Core::getConvertedMousePos..." << indx << "  " << mouseX << "  " << mouseY;
    QVariantMap retVal;

    int currIndx = indx - 1;
    int secIndx = currIndx == 0 ? 1 : 0;

    if (plot2dList_.size() < 2) {
        return retVal;
    }

    auto& firstPlot =  plot2dList_.at(currIndx);
    auto& secondPlot =  plot2dList_.at(secIndx);

    bool isCurrHor = firstPlot->isHorizontal();
    bool isSecHor  = secondPlot->isHorizontal();

    const float currDepth = firstPlot->getDepthByMousePos(mouseX, mouseY, isCurrHor);
    const int currEpochIndx = firstPlot->getEpochIndxByMousePos(mouseX, mouseY, isCurrHor);

    if (currEpochIndx == -1) {
        retVal["x"] = mouseX;
        retVal["y"] = mouseY;
        return retVal;
    }

    const auto mousePos = secondPlot->getMousePosByDepthAndEpochIndx(currDepth, currEpochIndx, isSecHor);

    retVal["x"] = mousePos.x();
    retVal["y"] = mousePos.y();

    return retVal;
}

void Core::setIsAttitudeExpected(bool state)
{
    dataHorizon_->setIsAttitudeExpected(state);
}

void Core::openFileFromMenu()
{
    QString defaultPath = openedfilePath_.isNull() ? qApp->applicationDirPath() + "/data/" : openedfilePath_;
    QFileDialog dialog(nullptr, tr("Open"), defaultPath,
        "Toslon Sonar Log(*.tsl3);;" "Toslon Sonar Log(*.tslw);;" "Toslon Sonar(*.kml *.kmz);;" "Toslon Sonar Log(*.csv)");
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if(!openedFileFilter_.isEmpty()) {
        dialog.selectNameFilter(openedFileFilter_);
    }

    QStringList fileNames;
    if(dialog.exec() == QDialog::Accepted) {
        fileNames = dialog.selectedFiles();
        openedFileFilter_ = dialog.selectedNameFilter();
    }

    int fileCnt = fileNames.count();
    if(fileCnt < 1) {
        return;
    }

    QFileInfo fi(fileNames.last());
    openedfilePath_ = fi.absolutePath();

    if(datasetPtr_ && datasetPtr_->size() > 0) {
        resetDataProcessorConnections();
        bleManager_->clearRealData();
        udpManager_->clearRealData();
        serialPortManager_->clearRealData();
        datasetPtr_->resetDataset();
        dataHorizon_->clear();
        if (scene3dViewPtr_) {
            scene3dViewPtr_->clear(true);
            scene3dViewPtr_->getNavigationArrowPtr()->resetPositionAndAngle();
        }
        QMetaObject::invokeMethod(dataProcessor_, "clearProcessing2", Qt::QueuedConnection, Q_ARG(bool, true));
        setDataProcessorConnections();
    }

    if (progress_) {
        QMetaObject::invokeMethod(progress_, "open");
    }
    onDataProcesstorStateChanged(DataProcessorType::staticTrack);

    /*-----------------------open kml/kmz--------------------------*/
    if(fileNames.last().endsWith("kml") || fileNames.last().endsWith("kmz")) {
         // currentFileType = filetype_kmlkmz;
         // if(fileCnt > 1){
         //     // QMessageBox::information(NULL, tr("Hint"), tr("Only One file in this format Allowed!"), tr("OK"));
         //     GIF->dialogInfo(Dialog_OK, tr("Only One file in this format Allowed!"));
         // }
         // else if(fileCnt == 1) {
         //     currentKmlPath_ = fileNames.at(0);
         //     fileManager_->extractKmz(currentKmlPath_);
         //     typMercatorPoint kmzMercatorPoint = fileManager_->getKmzMercatorPoint();
         //     mapGraphicsView_->clearAllContour();
         //     mapGraphicsView_->setShowContoursMode(true);
         //     if(kmzMercatorPoint.Xpoint != -1000 && kmzMercatorPoint.Ypoint != -1000)
         //     {
         //         fileManager_->resetKmzMercatorPoint();
         //         typGpsCooDegree lonLat = GPSProcessing::MercatorTolonLat(kmzMercatorPoint);
         //         mapGraphicsView_->locationFromlonlat(lonLat.longitude,lonLat.latitude,17);
         //     }
         //     else
         //     {
         //         mapGraphicsView_->drawShowOutline();
         //     }

         //     mapGraphicsView_->setScreenMode(false);
         //     mapGraphicsView_->setMeasureDistanceMode(false);
         //     mapGraphicsView_->setDrawContoursMode(false);
         //     mapGraphicsView_->setLandMarksMode(false);
         //     isLandMarkClick_ = false;
         //     isFrameSelectActClick_ = false;
         //     isMeasureActClick_ = false;
         //     isContourActClick_ = false;
         //     frameSelectBtn_->setStyleSheet("QPushButton { background-color: none; }");
         //     measureDisBtn_->setStyleSheet("QPushButton { background-color:none;}");
         //     contoursBtn_->setStyleSheet("QPushButton { background-color: none;}");

         //     mapManager_->clearFileManagement();

         //     emit callKmzDockWidget(currentKmlPath_);
         // }

         // return;
    }
    /*-----------------------open tsl3/tslw/csv--------------------------*/
    else
    {
        int size_sum = 0;
        for(int i = 0; i < fileCnt; i++) {
            QFile file(fileNames.at(i));

            size_sum += file.size() / 1024 / 1024;
            if(size_sum > 1024) {
                GIF->dialogInfo(Dialog_OK, tr("File size is too large!"));
                return;
            }

            if (progress_) {
                QString statusText = tr("Opening file %1 / %2").arg(i+1).arg(fileCnt);
                QMetaObject::invokeMethod(progress_, "setStatus", Q_ARG(QVariant, statusText));
                double progress = static_cast<double>(i) / fileCnt;
                QMetaObject::invokeMethod(progress_, "setProgress", Q_ARG(QVariant, progress));
                QMetaObject::invokeMethod(progress_, "setIndeterminate", Q_ARG(QVariant, false));
                QCoreApplication::processEvents();
            }
        }

        if(fileNames.last().endsWith("csv")) {
            currentFileType_ = filetype_CSV;
        }
        else if(fileNames.last().endsWith("tsl3")) {
            currentFileType_ = filetype_tsl3;
        }
        else if(fileNames.last().endsWith("tslw")) {
            currentFileType_ = filetype_tslw;
        }
        else if(fileNames.last().endsWith("txt")) {
            currentFileType_ = filetype_serial;
            return;
        }

        deviceManagerWrapperPtr_->resetFileAndChannelId(fileCnt);

        //读取内容并调用相应的处理函数
        fileNames.sort();
        for(int i = 0; i < fileCnt; i++) {
            /*-按照已选择的文件名路径打开文件，给下一步做铺垫-*/
            QString nowFileName = fileNames.at(i);

            if(currentFileType_ == filetype_tslw) {
                emit deviceManagerWrapperPtr_->sendOpenFile_tsl(nowFileName, filetype_tslw, i, fileCnt);
            }
            else if(currentFileType_ == filetype_tsl3) {
                emit deviceManagerWrapperPtr_->sendOpenFile_tsl(nowFileName, filetype_tsl3, i, fileCnt);
            }
            else if(currentFileType_ == filetype_CSV) {
                emit deviceManagerWrapperPtr_->sendOpenFile_CSV(nowFileName, i, fileCnt);
            }

            // openedfilePath_ = nowFileName;
        }

    }

}

void Core::clearRouteData()
{
    if(!datasetPtr_) {
        return;
    }
    if(datasetPtr_->size() == 0) {
        GIF->dialogInfo(Dialog_OK, "No Track Points Available!");
        return;
    }
    if(dataProcessorState_ == DataProcessorType::bletoothTrack) {
        GIF->dialogYesNo(tr("Confirm Clear All Historical Data?"),[this](bool confirmed) {
            if(confirmed) {
                bleManager_->clearRealData();
                datasetPtr_->resetDataset();
                dataHorizon_->clear();
                QMetaObject::invokeMethod(dataProcessor_, "clearProcessing", Qt::QueuedConnection);

                if (scene3dViewPtr_) {
                    scene3dViewPtr_->clear(true);
                    scene3dViewPtr_->getNavigationArrowPtr()->resetPositionAndAngle();
                }

                emit isobathsViewControlMenuController_->edgeLimitChanged(100);
            }
        });
    }
    else if(dataProcessorState_ == DataProcessorType::wifiTrack) {
        GIF->dialogYesNo(tr("Confirm Clear All Historical Data?"),[this](bool confirmed) {
            if(confirmed) {
                udpManager_->clearRealData();
                datasetPtr_->resetDataset();
                dataHorizon_->clear();
                QMetaObject::invokeMethod(dataProcessor_, "clearProcessing", Qt::QueuedConnection);

                if (scene3dViewPtr_) {
                    scene3dViewPtr_->clear(true);
                    scene3dViewPtr_->getNavigationArrowPtr()->resetPositionAndAngle();
                }

                emit isobathsViewControlMenuController_->edgeLimitChanged(100);
            }
        });
    }
    else if(dataProcessorState_ == DataProcessorType::serialPortTrack) {
        GIF->dialogYesNo(tr("Confirm Clear All Historical Data?"),[this](bool confirmed) {
            if(confirmed) {
                serialPortManager_->clearRealData();
                datasetPtr_->resetDataset();
                dataHorizon_->clear();
                QMetaObject::invokeMethod(dataProcessor_, "clearProcessing", Qt::QueuedConnection);
                if (scene3dViewPtr_) {
                    scene3dViewPtr_->clear(true);
                    scene3dViewPtr_->getNavigationArrowPtr()->resetPositionAndAngle();
                }

                const int numPlots = plot2dList_.size();
                for(int i = 0; i < numPlots; i++) {
                    qPlot2D* plot2d = plot2dList_.at(i);
                    if(plot2d){
                        plot2d->clearPlotData();
                    }
                }

                emit isobathsViewControlMenuController_->edgeLimitChanged(100);
            }
        });
    }
    else if(dataProcessorState_ == DataProcessorType::staticTrack) {
        GIF->dialogCheck(tr("Confirm to Clear Isobaths?"),[this](bool confirmed, bool clearTrack) {
            if(confirmed) {
                if(clearTrack) {
                    bleManager_->clearRealData();
                    datasetPtr_->resetDataset();
                    dataHorizon_->clear();
                    if (scene3dViewPtr_) {
                        scene3dViewPtr_->clear(true);
                        scene3dViewPtr_->getNavigationArrowPtr()->resetPositionAndAngle();
                    }
                }
                QMetaObject::invokeMethod(dataProcessor_, "clearProcessing2", Qt::QueuedConnection, Q_ARG(bool,clearTrack));
            }
        }, tr("Clear Track Data"));
    }

}

void Core::clearAll()
{
    if(!datasetPtr_) {
        return;
    }
    if(datasetPtr_->size() == 0) {
        GIF->dialogInfo(Dialog_OK, "No Track Points Available!");
        return;
    }

    GIF->dialogYesNo(tr("Confirm Clear All Historical Data?"),[this](bool confirmed) {
        if(confirmed) {
            bleManager_->clearRealData();
            datasetPtr_->resetDataset();
            dataHorizon_->clear();

            QMetaObject::invokeMethod(dataProcessor_, "clearProcessing", Qt::QueuedConnection);

            if (scene3dViewPtr_) {
                scene3dViewPtr_->clear(true);
                scene3dViewPtr_->getNavigationArrowPtr()->resetPositionAndAngle();
            }
            // emit isobathsViewControlMenuController_->edgeLimitChanged(100);
        }

    });
}

void Core::setAutoRenderSpan(bool isAuto)
{
    isAutoRenderSpan_ = isAuto;
}

void Core::exitApp()
{
    GIF->dialogYesNo(tr("Do You Want to Exit the Application?"), [](bool confirmed) {
        if(confirmed) {
            QCoreApplication::quit();
        }
    });
}

void Core::switchMapType(int sourceType)
{
    MapSourceType type = (MapSourceType)sourceType;
    if (tileManager_) {
        tileManager_->switchMapSource(type);
    }
    if(scene3dViewPtr_){
        scene3dViewPtr_->screetShot_.switchMapSource(type);
    }
}

// void Core::setDepthFilterVisible(bool visible, int value)
// {
//     const int numPlots = plot2dList_.size();
//     for(int i = 0; i < numPlots; i++) {
//         qPlot2D* plot2d = plot2dList_.at(i);
//         if(plot2d) {
//             plot2d->setDepthFilterVisible(visible, value);
//         }
//     }
// }

// void Core::setKeelOffsetValue(int value)
// {
//     const int numPlots = plot2dList_.size();
//     for(int i = 0; i < numPlots; i++) {
//         qPlot2D* plot2d = plot2dList_.at(i);
//         if(plot2d) {
//             plot2d->setKeelOffsetValue(value);
//         }
//     }
// }

void Core::onTileSetChanged(std::shared_ptr<map::TileSet> tileSet)
{
    if(!tileSet) {
        return;
    }

    // 清除 MapView 中的旧瓦片
    if (scene3dViewPtr_ && scene3dViewPtr_->getMapViewPtr()) {
        scene3dViewPtr_->getMapViewPtr()->clear();
    }

    // 断开旧的连接
    QObject::disconnect(oldTileSetConnection_);

    // 建立新的连接
    auto connType = Qt::DirectConnection;
    oldTileSetConnection_ = QObject::connect(tileSet.get(), &map::TileSet::mvAppendTile,  scene3dViewPtr_->getMapViewPtr().get(), &MapView::onTileAppend,  connType);
    QObject::connect(tileSet.get(), &map::TileSet::mvDeleteTile,         scene3dViewPtr_->getMapViewPtr().get(), &MapView::onTileDelete,             connType);
    QObject::connect(tileSet.get(), &map::TileSet::mvUpdateTileImage,    scene3dViewPtr_->getMapViewPtr().get(), &MapView::onTileImageUpdated,       connType);
    QObject::connect(tileSet.get(), &map::TileSet::mvUpdateTileVertices, scene3dViewPtr_->getMapViewPtr().get(), &MapView::onTileVerticesUpdated,    connType);
    QObject::connect(tileSet.get(), &map::TileSet::mvClearAppendTasks,   scene3dViewPtr_->getMapViewPtr().get(), &MapView::onClearAppendTasks,       connType);
    QObject::connect(scene3dViewPtr_->getMapViewPtr().get(), &MapView::deletedFromAppend, tileSet.get(), &map::TileSet::onDeletedFromAppend, connType);

    if(scene3dViewPtr_) {
        std::shared_ptr<MapView> mapView = scene3dViewPtr_->getMapViewPtr();
        mapView->setCurrentMapSource(tileManager_->getCurrentMapType());

        scene3dViewPtr_->updateMapView();
    }
}

void Core::location(uint8_t type)
{
    double latitude = 0.0, longitude = 0.0;
    switch(type) {
        case 0:
            qDebug() << "powerOn..........";
            break;

        case 1:
            qDebug() << "keepBoatView::blemnager......";
            latitude  = bleManager_->latitude_;
            longitude = bleManager_->longitude_;
            break;

        case 2:
            qDebug() << "locationManager......";
            break;

        case 3:
            qDebug() << "keepBoatView::serialPort......";
            latitude  = serialPortManager_->latitude_;
            longitude = serialPortManager_->longitude_;
            break;

        default:
            break;
    }

    if((latitude != 0) & (longitude != 0)) {
        datasetPtr_->location(latitude, longitude);
    }
}

void Core::onFileStopsOpening()
{
    qDebug() << "Core::onFileStopsOpening.................";
    isFileOpening_ = false;
    emit sendIsFileOpening();
    dataHorizon_->setIsFileOpening(isFileOpening_);
}

void Core::onFileStopsOpening2(QVector<float>& depthVec, double minZ, double maxZ)
{
    if(isAutoRenderSpan_) {
        int vecSize = depthVec.size();
        if(vecSize > 200 && vecSize <= 400) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(80);
        } else if(vecSize > 400 && vecSize <= 600) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(60);
        } else if(vecSize > 600 && vecSize <= 800) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(50);
        } else if(vecSize > 800) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(40);
        }
    }
    // qDebug() << "onFileStopsOpening2.............." << minZ << "    " << maxZ;
    datasetPtr_->vec_CSV_ += depthVec;
    datasetPtr_->minDepth_ = minZ;
    datasetPtr_->maxDepth_ = maxZ;
    datasetPtr_->setAutoBounadry();
    QMetaObject::invokeMethod(dataProcessor_, "postMinZ", Qt::QueuedConnection, Q_ARG(float, minZ));
    QMetaObject::invokeMethod(dataProcessor_, "postMaxZ", Qt::QueuedConnection, Q_ARG(float, maxZ));
    scene3dViewPtr_->focusTrackBounds();

    for (int i = 0; i < plot2dList_.size(); i++) {
        qPlot2D* qplot2d = plot2dList_.at(i);
        if (qplot2d != NULL) {
            qplot2d->setTimelinePositionToStart();
        }
    }
}

void Core::onSendMapTextureIdByTileIndx(const map::TileIndex &tileIndx, GLuint textureId)
{
    tileManager_->getTileSetPtr()->setTextureIdByTileIndx(tileIndx, textureId);
}

ConsoleListModel* Core::consoleList()
{
    return consolePtr_->listModel();
}

void Core::createControllers()
{
    boatTrackControlMenuController_       = std::make_shared<BoatTrackControlMenuController>();
    navigationArrowControlMenuController_ = std::make_shared<NavigationArrowControlMenuController>();
    bottomTrackControlMenuController_     = std::make_shared<BottomTrackControlMenuController>();
    // mpcFilterControlMenuController_       = std::make_shared<MpcFilterControlMenuController>();
    // npdFilterControlMenuController_       = std::make_shared<NpdFilterControlMenuController>();
    isobathsViewControlMenuController_    = std::make_shared<IsobathsViewControlMenuController>();
    mosaicViewControlMenuController_      = std::make_shared<MosaicViewControlMenuController>();
    imageViewControlMenuController_       = std::make_shared<ImageViewControlMenuController>();
    mapViewControlMenuController_         = std::make_shared<MapViewControlMenuController>();
    pointGroupControlMenuController_      = std::make_shared<PointGroupControlMenuController>();
    // polygonGroupControlMenuController_    = std::make_shared<PolygonGroupControlMenuController>();
    scene3dControlMenuController_         = std::make_shared<Scene3DControlMenuController>();
    scene3dToolBarController_             = std::make_shared<Scene3dToolBarController>();
    // usblViewControlMenuController_        = std::make_shared<UsblViewControlMenuController>();

    bleManager_                           = std::make_shared<BLEManager>();
    udpManager_                           = std::make_shared<UdpManager>();
    serialPortManager_                    = std::make_shared<SerialPortManager>();
    locations_                            = std::make_shared<Locations>();
}

void Core::createDeviceManagerConnections()
{
    Qt::ConnectionType directionConnection = Qt::ConnectionType::DirectConnection;
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::sendChartSetup, datasetPtr_,  &Dataset::setChartSetup,         directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::sendTranscSetup, datasetPtr_, &Dataset::setTranscSetup,        directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::sendSoundSpeeed, datasetPtr_, &Dataset::setSoundSpeed,         directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::chartComplete, datasetPtr_,   &Dataset::addChart,              directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::distComplete, datasetPtr_,    &Dataset::addDist,               directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::usblSolutionComplete, datasetPtr_, &Dataset::addUsblSolution,  directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::dopplerBeamComlete, datasetPtr_, &Dataset::addDopplerBeam,     directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::dvlSolutionComplete, datasetPtr_, &Dataset::addDVLSolution,    directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::upgradeProgressChanged, this,        &Core::upgradeChanged,    directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::eventComplete, datasetPtr_,   &Dataset::addEvent,              directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::rangefinderComplete, datasetPtr_, &Dataset::addRangefinder,    directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::positionComplete, datasetPtr_,&Dataset::addPosition,           directionConnection);
    QObject::connect(bleManager_.get(), &BLEManager::positionComplete, datasetPtr_, &Dataset::addPosition_realTime,                        directionConnection);
    QObject::connect(udpManager_.get(), &UdpManager::positionComplete, datasetPtr_, &Dataset::addPosition_realTime,                        directionConnection);
    QObject::connect(serialPortManager_.get(), &SerialPortManager::positionComplete, datasetPtr_, &Dataset::addPosition_realTime,          directionConnection);
    QObject::connect(serialPortManager_.get(), &SerialPortManager::chartComplete,    datasetPtr_, &Dataset::addChart,                      directionConnection);


    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::positionComplete_file, datasetPtr_, &Dataset::addPosition_file, directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::positionCompleteRTK,  datasetPtr_, &Dataset::addPositionRTK,    directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::depthComplete, datasetPtr_, &Dataset::addDepth,                 directionConnection);

    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::gnssVelocityComplete, datasetPtr_, &Dataset::addGnssVelocity,   directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::attitudeComplete, datasetPtr_, &Dataset::addAtt,                directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::tempComplete, datasetPtr_, &Dataset::addTemp,                   directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::encoderComplete, datasetPtr_, &Dataset::addEncoder,             directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::fileStopsOpening, this,  &Core::onFileStopsOpening,             directionConnection);
    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::fileStopsOpening2, this, &Core::onFileStopsOpening2,            directionConnection);

    QObject::connect(bleManager_.get(), &BLEManager::signal_drawRealtimeContour, this, &Core::slot_RealtimeDrawContourBle,                  directionConnection);
    QObject::connect(udpManager_.get(), &UdpManager::signal_drawRealtimeContour, this, &Core::slot_RealtimeDrawContourWifi,                 directionConnection);
    QObject::connect(serialPortManager_.get(), &SerialPortManager::signal_drawRealtimeContour, this, &Core::slot_RealtimeDrawContourSerialPort, directionConnection);

    QObject::connect(deviceManagerWrapperPtr_->getWorker(), &DeviceManager::sendProtoFrame, &logger_, &Logger::receiveProtoFrame,           directionConnection);
}

QHash<QUuid, QString> Core::getLinkNames() const
{
    QHash<QUuid, QString> retVal;

    if (isFileOpening_) {
        retVal[deviceManagerWrapperPtr_->getFileUuid()] = QObject::tr("File");
    }

    // const auto linkNames = linkManagerWrapperPtr_->getLinkNames();
    // for (auto it = linkNames.constBegin(); it != linkNames.constEnd(); ++it) {
    //     retVal.insert(it.key(), it.value());
    // }

    return retVal;
}


bool Core::isOpenedFile() const
{
    return !openedfilePath_.isEmpty();
}

bool Core::isFactoryMode() const
{
    return false;
}

QString Core::getFilePath() const
{
    return filePath_;
}

void Core::fixFilePathString(QString& filePath) const
{
    Q_UNUSED(filePath);
#ifdef Q_OS_WINDOWS
    filePath.remove("'");

    DWORD size = GetLongPathNameW(reinterpret_cast<LPCWSTR>(filePath.utf16()), nullptr, 0);
    std::wstring buffer(size, L'\0');
    size = GetLongPathNameW(reinterpret_cast<LPCWSTR>(filePath.utf16()), &buffer[0], size);
    buffer.resize(size);
    filePath = QString::fromStdWString(buffer.c_str());

    filePath.replace("\\", "/");
#endif
}

void Core::saveLLARefToSettings()
{
    if (!datasetPtr_) {
        return;
    }

    try {
        auto ref = datasetPtr_->getLlaRef();

        QSettings settings("Toslon", "ToslonApp");
        QString group{"LLARef"};

        settings.beginGroup(group);
        settings.setValue("refLatSin", ref.refLatSin);
        settings.setValue("refLatCos", ref.refLatCos);
        settings.setValue("refLatRad", ref.refLatRad);
        settings.setValue("refLonRad", ref.refLonRad);
        settings.setValue("refLlaLatitude", ref.refLla.latitude);
        settings.setValue("refLlaLongitude", ref.refLla.longitude);
        settings.setValue("isInit", ref.isInit);
        settings.endGroup();

        settings.sync();

        //qDebug() << "saved: " << ref.refLla.latitude << ref.refLla.longitude;
    }
    catch (const std::exception& e) {
        qCritical() << "Core::saveLLARefToSettings throw exception:" << e.what();
    }
    catch (...) {
        qCritical() << "Core::saveLLARefToSettings throw unknown exception";
    }
}

void Core::loadLLARefFromSettings()
{
    if (!datasetPtr_) {
        return;
    }

    try {
        QSettings settings("Toslon", "ToslonApp");
        QString group{"LLARef"};

        settings.beginGroup(group);
        LLARef ref;
        ref.refLatSin = settings.value("refLatSin", NAN).toDouble();
        ref.refLatCos = settings.value("refLatCos", NAN).toDouble();
        ref.refLatRad = settings.value("refLatRad", NAN).toDouble();
        ref.refLonRad = settings.value("refLonRad", NAN).toDouble();
        ref.refLla.latitude = settings.value("refLlaLatitude", NAN).toDouble();
        ref.refLla.longitude = settings.value("refLlaLongitude", NAN).toDouble();
        ref.isInit = settings.value("isInit", false).toBool();
        settings.endGroup();

        datasetPtr_->setLlaRef(ref, Dataset::LlaRefState::kUndefined);
        //qDebug() << "loaded: " << ref.refLla.latitude << ref.refLla.longitude;
    }
    catch (const std::exception& e) {
        qCritical() << "Core::loadLLARefFromSettings throw exception:" << e.what();
    }
    catch (...) {
        qCritical() << "Core::loadLLARefFromSettings throw unknown exception";
    }
}


void Core::createMapTileManagerConnections()
{
    tileManager_ = std::make_unique<map::TileManager>(this);
    QObject::connect(scene3dViewPtr_, &GraphicsScene3dView::sendRectRequest, tileManager_.get(), &map::TileManager::getRectRequest, Qt::DirectConnection);
    QObject::connect(scene3dViewPtr_, &GraphicsScene3dView::sendLlaRef, tileManager_.get(), &map::TileManager::getLlaRef, Qt::DirectConnection);
    QObject::connect(tileManager_.get(), &map::TileManager::targetTilesLoaded, scene3dViewPtr_, &GraphicsScene3dView::onTargetTilesLoaded, Qt::DirectConnection);

    auto connType = Qt::DirectConnection;
    QObject::connect(tileManager_->getTileSetPtr().get(),    &map::TileSet::mvAppendTile,         scene3dViewPtr_->getMapViewPtr().get(), &MapView::onTileAppend,             connType);
    QObject::connect(tileManager_->getTileSetPtr().get(),    &map::TileSet::mvDeleteTile,         scene3dViewPtr_->getMapViewPtr().get(), &MapView::onTileDelete,             connType);
    QObject::connect(tileManager_->getTileSetPtr().get(),    &map::TileSet::mvUpdateTileImage,    scene3dViewPtr_->getMapViewPtr().get(), &MapView::onTileImageUpdated,       connType);
    QObject::connect(tileManager_->getTileSetPtr().get(),    &map::TileSet::mvUpdateTileVertices, scene3dViewPtr_->getMapViewPtr().get(), &MapView::onTileVerticesUpdated,    connType);
    QObject::connect(tileManager_->getTileSetPtr().get(),    &map::TileSet::mvClearAppendTasks,   scene3dViewPtr_->getMapViewPtr().get(), &MapView::onClearAppendTasks,       connType);
    QObject::connect(scene3dViewPtr_->getMapViewPtr().get(), &MapView::deletedFromAppend,         tileManager_->getTileSetPtr().get(),    &map::TileSet::onDeletedFromAppend, connType);

    QObject::connect(scene3dViewPtr_, &GraphicsScene3dView::sendMapTextureIdByTileIndx, this, &Core::onSendMapTextureIdByTileIndx, connType);
    QObject::connect(tileManager_.get(), &map::TileManager::zoomLevelChanged, this, &Core::onZoomLevelChanged);
    QObject::connect(tileManager_.get(), &map::TileManager::tileSetChanged, this, &Core::onTileSetChanged, connType);
    QObject::connect(datasetPtr_,  &Dataset::signalDrawOutline, scene3dViewPtr_, &GraphicsScene3dView::setPolygonOutlineMode, connType);

}

void Core::onDataProcesstorStateChanged(const DataProcessorType& state)
{
    dataProcessorState_ = state;
    dataProcessor_->setDataProcessType(state);
}

void Core::onZoomLevelChanged(int level)
{
    currMapLevel_ = level;
    scene3dViewPtr_->setCurrentMapLevel(level);
    emit currentMapLevelChanged();
}

void Core::slot_RealtimeDrawContourBle(QVector<float>& depthVec, double minZ, double maxZ, bool isRead)
{
    int vecSize = depthVec.size();
    if(vecSize > 0 && vecSize < 3) {
        qDebug() << "vecSize..........." << vecSize;
        onDataProcesstorStateChanged(DataProcessorType::bletoothTrack);
    }
    if(isAutoRenderSpan_) {
        if(vecSize == 200) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(80);
        }
        else if(vecSize == 400) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(60);
        }
        else if(vecSize == 600) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(50);
        }
        else if(vecSize == 800) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(40);
        }
    }

    datasetPtr_->vec_CSV_  = depthVec;
    datasetPtr_->minDepth_ = minZ;
    datasetPtr_->maxDepth_ = maxZ;
    QMetaObject::invokeMethod(dataProcessor_, "postMinZ", Qt::QueuedConnection, Q_ARG(float, minZ));
    QMetaObject::invokeMethod(dataProcessor_, "postMaxZ", Qt::QueuedConnection, Q_ARG(float, maxZ));
    emit drawRealtimeContour(isRead);
}

void Core::slot_RealtimeDrawContourWifi(QVector<float>& depthVec, double minZ, double maxZ, bool isRead)
{
    int vecSize = depthVec.size();
    if(vecSize > 0 && vecSize < 3) {
        qDebug() << "vecSize..........." << vecSize;
        onDataProcesstorStateChanged(DataProcessorType::wifiTrack);
    }
    if(isAutoRenderSpan_) {
        if(vecSize == 200) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(80);
        }
        else if(vecSize == 400) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(60);
        }
        else if(vecSize == 600) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(50);
        }
        else if(vecSize == 800) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(40);
        }
    }

    datasetPtr_->vec_CSV_  = depthVec;
    datasetPtr_->minDepth_ = minZ;
    datasetPtr_->maxDepth_ = maxZ;
    QMetaObject::invokeMethod(dataProcessor_, "postMinZ", Qt::QueuedConnection, Q_ARG(float, minZ));
    QMetaObject::invokeMethod(dataProcessor_, "postMaxZ", Qt::QueuedConnection, Q_ARG(float, maxZ));
    emit drawRealtimeContour(isRead);

}

void Core::slot_RealtimeDrawContourSerialPort(QVector<float>& depthVec, double minZ, double maxZ, bool isRead)
{
    int vecSize = depthVec.size();
    if(vecSize > 0 && vecSize < 3) {
        qDebug() << "vecSize..........." << vecSize;
        onDataProcesstorStateChanged(DataProcessorType::serialPortTrack);
    }
    if(isAutoRenderSpan_) {
        if(vecSize == 200) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(80);
        }
        else if(vecSize == 400) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(60);
        }
        else if(vecSize == 600) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(50);
        }
        else if(vecSize == 800) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(40);
        }
    }

    datasetPtr_->vec_CSV_  = depthVec;
    datasetPtr_->minDepth_ = minZ;
    datasetPtr_->maxDepth_ = maxZ;
    QMetaObject::invokeMethod(dataProcessor_, "postMinZ", Qt::QueuedConnection, Q_ARG(float, minZ));
    QMetaObject::invokeMethod(dataProcessor_, "postMaxZ", Qt::QueuedConnection, Q_ARG(float, maxZ));
    emit drawRealtimeContour(isRead);
}

void Core::createDatasetConnections()
{
    QObject::connect(datasetPtr_, &Dataset::channelsUpdated, this,               &Core::onChannelsUpdated);

    QObject::connect(datasetPtr_, &Dataset::epochAdded,       dataHorizon_.get(), &DataHorizon::onAddedEpoch);
    QObject::connect(datasetPtr_, &Dataset::positionAdded,    dataHorizon_.get(), &DataHorizon::onAddedPosition);
    QObject::connect(datasetPtr_, &Dataset::chartAdded,       dataHorizon_.get(), &DataHorizon::onAddedChart);
    QObject::connect(datasetPtr_, &Dataset::attitudeAdded,    dataHorizon_.get(), &DataHorizon::onAddedAttitude);
    QObject::connect(datasetPtr_, &Dataset::bottomTrackAdded, dataHorizon_.get(), &DataHorizon::onAddedBottomTrack);
}

int Core::getCurrMapLevel() const
{
    return currMapLevel_;
}

int Core::getDataProcessorState() const
{
    return static_cast<int>(dataProcessorState_);
}

void Core::initStreamList()
{
    deviceManagerWrapperPtr_->initStreamList();
}

QObject* Core::progress() const
{
    return progress_;
}
void Core::setProgress(QObject* dialog)
{
    if (progress_ != dialog) {
        progress_ = dialog;
        DeviceManager* deviceManager = deviceManagerWrapperPtr_->getWorker();
        if (deviceManager) {
            deviceManager->setProgressDialog(dialog);
        }
        emit progressChanged();
    }
}

void Core::createDataProcessor()
{
    dataProcThread_ = new QThread(this);
    dataProcessor_  = new DataProcessor(nullptr, datasetPtr_);

    dataProcessor_->moveToThread(dataProcThread_);

    QObject::connect(dataProcThread_, &QThread::finished, dataProcessor_,  &QObject::deleteLater);
    QObject::connect(dataProcThread_, &QThread::finished, dataProcThread_, &QObject::deleteLater);

    dataProcThread_->setObjectName("DataProcThread");

    setDataProcessorConnections();

    dataProcThread_->start();
}

void Core::destroyDataProcessor()
{
    resetDataProcessorConnections();

    if (dataProcThread_ && dataProcThread_->isRunning()) {
        dataProcThread_->quit();
        dataProcThread_->wait();
    }

    delete dataProcessor_;

    dataProcessor_ = nullptr;
    dataProcThread_ = nullptr;
}

void Core::createScene3dConnections()
{
    QObject::connect(dataHorizon_.get(), &DataHorizon::positionAdded, scene3dViewPtr_, &GraphicsScene3dView::onPositionAdded);
    QObject::connect(scene3dViewPtr_->bottomTrack().get(), &BottomTrack::updatedPoints, dataHorizon_.get(), &DataHorizon::onAddedBottomTrack3D);

    // res work proc
    auto connType = Qt::QueuedConnection;
    // Surface
    QObject::connect(dataProcessor_, &DataProcessor::sendSurfaceTextureTask,        scene3dViewPtr_->getSurfaceViewPtr().get(),     &SurfaceView::setTextureTask,                 connType);
    QObject::connect(dataProcessor_, &DataProcessor::sendSurfaceMinZ,               scene3dViewPtr_->getSurfaceViewPtr().get(),     &SurfaceView::setMinZ,                        connType);
    QObject::connect(dataProcessor_, &DataProcessor::sendSurfaceMaxZ,               scene3dViewPtr_->getSurfaceViewPtr().get(),     &SurfaceView::setMaxZ,                        connType);
    QObject::connect(dataProcessor_, &DataProcessor::sendSurfaceStepSize,           scene3dViewPtr_->getSurfaceViewPtr().get(),     &SurfaceView::setSurfaceStep,                 connType);
    QObject::connect(dataProcessor_, &DataProcessor::sendSurfaceColorIntervalsSize, scene3dViewPtr_->getSurfaceViewPtr().get(),     &SurfaceView::setColorIntervalsSize,          connType);
    QObject::connect(dataProcessor_, &DataProcessor::surfaceBoundaryVerticesUpdated, scene3dViewPtr_->getSurfaceViewPtr().get(),     &SurfaceView::setBoundaryVertices,          connType);
    QObject::connect(dataProcessor_, &DataProcessor::sendPolygonOulineAuto,         scene3dViewPtr_->polygonOutline().get(),        &PolygonOutline::autoGenerateBoundary,  connType);
    // IsobathsView
    QObject::connect(dataProcessor_, &DataProcessor::sendIsobathsLabels,            scene3dViewPtr_->getIsobathsViewPtr().get(),    &IsobathsView::setLabels,                     connType);
    QObject::connect(dataProcessor_, &DataProcessor::sendIsobathsLineSegments,      scene3dViewPtr_->getIsobathsViewPtr().get(),    &IsobathsView::setLineSegments,               connType);
    QObject::connect(dataProcessor_, &DataProcessor::sendIsobathsColoredLineSegments, scene3dViewPtr_->getIsobathsViewPtr().get(),   &IsobathsView::setColoredLineSegments,        connType);
    QObject::connect(dataProcessor_, &DataProcessor::sendIsobathsLineStepSize,      scene3dViewPtr_->getIsobathsViewPtr().get(),    &IsobathsView::setLineStepSize,               connType);
    // Mosaic
    QObject::connect(dataProcessor_, &DataProcessor::sendMosaicColorTable,          scene3dViewPtr_->getSurfaceViewPtr().get(),     &SurfaceView::setMosaicColorTableTextureTask, connType);
    QObject::connect(dataProcessor_, &DataProcessor::sendSurfaceTiles,              scene3dViewPtr_->getSurfaceViewPtr().get(),    &SurfaceView::setTiles,                       connType);
    // clear render
    QObject::connect(dataProcessor_, &DataProcessor::bottomTrackProcessingCleared,  scene3dViewPtr_->bottomTrack().get(),           &BottomTrack::clearData,                      connType);
    QObject::connect(dataProcessor_, &DataProcessor::isobathsProcessingCleared,     scene3dViewPtr_->getIsobathsViewPtr().get(),    &IsobathsView::clear,                         connType);
    QObject::connect(dataProcessor_, &DataProcessor::mosaicProcessingCleared,       this, [](){},                               connType);
    QObject::connect(dataProcessor_, &DataProcessor::surfaceProcessingCleared,      scene3dViewPtr_->getSurfaceViewPtr().get(),     &SurfaceView::clear,                          connType);

    QMetaObject::invokeMethod(dataProcessor_, "askColorTableForMosaic", Qt::QueuedConnection);
}

void Core::setDataProcessorConnections()
{
    // from dataHorizon
    auto connType = Qt::QueuedConnection;
    dataProcessorConnections_.append(QObject::connect(dataHorizon_.get(), &DataHorizon::chartAdded,  dataProcessor_, &DataProcessor::onChartsAdded, connType));
    dataProcessorConnections_.append(QObject::connect(dataHorizon_.get(), &DataHorizon::bottomTrack3DAdded, dataProcessor_, &DataProcessor::onBottomTrack3DAdded, connType));
    dataProcessorConnections_.append(QObject::connect(dataHorizon_.get(), &DataHorizon::mosaicCanCalc, dataProcessor_, &DataProcessor::onMosaicCanCalc, connType));
    dataProcessorConnections_.append(QObject::connect(dataHorizon_.get(), &DataHorizon::sonarPosCanCalc,  datasetPtr_, &Dataset::onSonarPosCanCalc, connType));

    dataProcessorConnections_.append(QObject::connect(dataProcessor_, &DataProcessor::distCompletedByProcessing,   datasetPtr_, &Dataset::onDistCompleted, connType));
    dataProcessorConnections_.append(QObject::connect(dataProcessor_, &DataProcessor::lastBottomTrackEpochChanged, datasetPtr_, &Dataset::onLastBottomTrackEpochChanged, connType));
}

void Core::resetDataProcessorConnections()
{
    for (auto& itm : dataProcessorConnections_) {
        disconnect(itm);
    }

    dataProcessorConnections_.clear();
}
