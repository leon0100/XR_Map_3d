#include "core.h"

#include <ctime>
#include "bottom_track.h"
#include "hotkeys_manager.h"
#ifdef Q_OS_WINDOWS
#include <Windows.h>
#endif


Core::Core() : QObject(),
    dataProcessor_(nullptr),
    dataProcThread_(nullptr),
    dataHorizon_(std::make_unique<DataHorizon>()),
    qmlAppEnginePtr_(nullptr),
    datasetPtr_(new Dataset),
    scene3dViewPtr_(nullptr),
    openedfilePath_(),
    filePath_(),
    isFileOpening_(false)
{
    qRegisterMetaType<uint8_t>("uint8_t");
    createControllers();
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
    qmlAppEnginePtr_->rootContext()->setContextProperty("BottomTrackControlMenuController",     bottomTrackControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("IsobathsViewControlMenuController",    isobathsViewControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("MosaicViewControlMenuController",      mosaicViewControlMenuController_.get());
    qmlAppEnginePtr_->rootContext()->setContextProperty("ImageViewControlMenuController",       imageViewControlMenuController_.get());

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

Dataset* Core::getDatasetPtr()
{
    return datasetPtr_;
}

DataProcessor* Core::getDataProcessorPtr() const
{
    return dataProcessor_;
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
            plot2dList_.at(i)->installEventFilter(scene3dViewPtr_->bottomTrack().get());
            plot2dList_.at(i)->installEventFilter(scene3dViewPtr_->getBoatTrackPtr().get());
        }
    }

    scene3dViewPtr_->setQmlRootObject(object);

    boatTrackControlMenuController_->setQmlEngine(object);
    boatTrackControlMenuController_->setGraphicsSceneView(scene3dViewPtr_);

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

    onChannelsUpdated();

    createMapTileManagerConnections();
    createScene3dConnections();

    QMetaObject::invokeMethod(dataProcessor_, "setBottomTrackPtr", Qt::QueuedConnection,
                              Q_ARG(BottomTrack*, scene3dViewPtr_->bottomTrack().get()));
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

void Core::onFitAllInViewButtonClicked()
{
    if(scene3dViewPtr_) {
        scene3dViewPtr_->fitAllInView();
    }
}

void Core::onIsNorthLocationButtonChanged(bool state)
{
    if(scene3dViewPtr_) {
        scene3dViewPtr_->setIsNorth(state);
    }
}

void Core::onNavigationArrowVisibleChanged(bool checked)
{
    if (scene3dViewPtr_) {
        if (auto nAPtr = scene3dViewPtr_->getNavigationArrowPtr(); nAPtr) {
            nAPtr->setVisible(checked);
        }
    }
}

void Core::openFileFromMenu()
{
    QString defaultPath = openedfilePath_.isNull() ? qApp->applicationDirPath() + "/data/" : openedfilePath_;
    QFileDialog dialog(nullptr, tr("Open"), defaultPath,
        "Toslon Sonar Log(*.tsl3);;"
        "Toslon Sonar Log(*.tslw);;"
        "Toslon Sonar(*.kml *.kmz);;"
        "Toslon Sonar Log(*.csv)");
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
        qint64 totalFileSize = 0;
        for(int i = 0; i < fileCnt; i++) {
            QFile file(fileNames.at(i));

            qint64 fileSize = file.size();
            size_sum += fileSize / 1024 / 1024;
            if(size_sum > 1024) {
                GIF->dialogInfo(Dialog_OK, tr("File size is too large!"));
                return;
            }
            totalFileSize += fileSize;

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

        // datasetPtr_->preallocatePool(static_cast<int>(totalFileSize));
        const int estimatedEpochs = fileCnt * 5000;
        datasetPtr_->preallocatePool(estimatedEpochs);
        deviceManager_->resetFileAndChannel(fileCnt);

        //读取内容并调用相应的处理函数
        fileNames.sort();
        for(int i = 0; i < fileCnt; i++) {
            /*-按照已选择的文件名路径打开文件，给下一步做铺垫-*/
            QString nowFileName = fileNames.at(i);

            if(currentFileType_ == filetype_tslw) {
                deviceManager_->openFile_tsl(nowFileName, filetype_tslw, i, fileCnt);
            }
            else if(currentFileType_ == filetype_tsl3) {
                deviceManager_->openFile_tsl(nowFileName, filetype_tsl3, i, fileCnt);
            }
            else if(currentFileType_ == filetype_CSV) {
                deviceManager_->openFile_CSV(nowFileName, i, fileCnt);
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
                    if(datasetPtr_->size() > 0) {
                        bleManager_->clearRealData();
                        udpManager_->clearRealData();
                        serialPortManager_->clearRealData();
                        datasetPtr_->resetDataset();
                        dataHorizon_->clear();
                        if (scene3dViewPtr_) {
                            scene3dViewPtr_->clear(true);
                            scene3dViewPtr_->getNavigationArrowPtr()->resetPositionAndAngle();
                        }
                    }
                    const int numPlots = plot2dList_.size();
                    for(int i = 0; i < numPlots; i++) {
                        qPlot2D* plot2d = plot2dList_.at(i);
                        if(plot2d){
                            plot2d->clearPlotData();
                        }
                    }

                    // emit isobathsViewControlMenuController_->edgeLimitChanged(100);
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

int Core::poolSize()
{
    if(!datasetPtr_) {
        return -1;
    }

    return datasetPtr_->size();
}


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

void Core::onFileStopsOpening(QVector<float>& depthVec, double minZ, double maxZ)
{
    if(isAutoRenderSpan_) {
        int vecSize = depthVec.size();
        if(vecSize > 200 && vecSize <= 400) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(80);
        }
        else if(vecSize > 400 && vecSize <= 600) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(60);
        }
        else if(vecSize > 600 && vecSize <= 800) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(50);
        }
        else if(vecSize > 800) {
            isobathsViewControlMenuController_->setEdgeLimitChanged(40);
        }
    }
    // qDebug() << "onFileStopsOpening.............." << minZ << "    " << maxZ;
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

void Core::createControllers()
{
    boatTrackControlMenuController_     = std::make_shared<BoatTrackControlMenuController>();
    bottomTrackControlMenuController_   = std::make_shared<BottomTrackControlMenuController>();
    isobathsViewControlMenuController_  = std::make_shared<IsobathsViewControlMenuController>();
    mosaicViewControlMenuController_    = std::make_shared<MosaicViewControlMenuController>();
    imageViewControlMenuController_     = std::make_shared<ImageViewControlMenuController>();

    deviceManager_                      = std::make_shared<DeviceManager>(datasetPtr_);
    bleManager_                         = std::make_shared<BLEManager>();
    udpManager_                         = std::make_shared<UdpManager>();
    serialPortManager_                  = std::make_shared<SerialPortManager>();
    locations_                          = std::make_shared<Locations>();
}

void Core::createDeviceManagerConnections()
{
    Qt::ConnectionType directionConnection = Qt::ConnectionType::DirectConnection;
    QObject::connect(deviceManager_.get(), &DeviceManager::chartComplete, datasetPtr_,   &Dataset::addChart,              directionConnection);
    // QObject::connect(deviceManager_.get(), &DeviceManager::positionComplete, datasetPtr_,&Dataset::addPosition,           directionConnection);
    QObject::connect(bleManager_.get(), &BLEManager::positionComplete, datasetPtr_, &Dataset::addPosition_realTime,               directionConnection);
    QObject::connect(udpManager_.get(), &UdpManager::positionComplete, datasetPtr_, &Dataset::addPosition_realTime,               directionConnection);
    QObject::connect(serialPortManager_.get(), &SerialPortManager::positionComplete, datasetPtr_, &Dataset::addPosition_realTime, directionConnection);
    QObject::connect(serialPortManager_.get(), &SerialPortManager::chartComplete,    datasetPtr_, &Dataset::addChart,             directionConnection);

    QObject::connect(deviceManager_.get(), &DeviceManager::positionComplete_file, datasetPtr_, &Dataset::addPosition_file, directionConnection);

    QObject::connect(deviceManager_.get(), &DeviceManager::fileStopsOpening, this, &Core::onFileStopsOpening,            directionConnection);

    QObject::connect(bleManager_.get(), &BLEManager::signal_drawRealtimeContour, this, &Core::slot_RealtimeDrawContourBle,  directionConnection);
    QObject::connect(udpManager_.get(), &UdpManager::signal_drawRealtimeContour, this, &Core::slot_RealtimeDrawContourWifi, directionConnection);
    QObject::connect(serialPortManager_.get(), &SerialPortManager::signal_drawRealtimeContour, this, &Core::slot_RealtimeDrawContourSerialPort, directionConnection);
}

QHash<QUuid, QString> Core::getLinkNames() const
{
    QHash<QUuid, QString> retVal;

    return retVal;
}

bool Core::isOpenedFile() const
{
    return !openedfilePath_.isEmpty();
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
}

int Core::getCurrMapLevel() const
{
    return currMapLevel_;
}

int Core::getDataProcessorState() const
{
    return static_cast<int>(dataProcessorState_);
}

QObject* Core::progress() const
{
    return progress_;
}
void Core::setProgress(QObject* dialog)
{
    if (progress_ != dialog) {
        progress_ = dialog;
        if (deviceManager_) {
            deviceManager_->setProgressDialog(dialog);
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

    dataProcessorConnections_.append(QObject::connect(dataProcessor_, &DataProcessor::lastBottomTrackEpochChanged, datasetPtr_, &Dataset::onLastBottomTrackEpochChanged, connType));
}

void Core::resetDataProcessorConnections()
{
    for (auto& itm : dataProcessorConnections_) {
        disconnect(itm);
    }

    dataProcessorConnections_.clear();
}
