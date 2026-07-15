#include "bottom_track.h"
#include "scene3d_view.h"
#include "epoch_event.h"
#include "boat_track.h"
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QHash>

BottomTrack::BottomTrack(GraphicsScene3dView* view, QObject* parent) : SceneObject(new BottomTrackRenderImplementation, view, parent),
    datasetPtr_(nullptr)
{
    setPrimitiveType(GL_LINE_STRIP);
}

BottomTrack::~BottomTrack()
{

}

SceneObject::SceneObjectType BottomTrack::type() const
{
    return SceneObject::SceneObjectType::BottomTrack;
}

bool BottomTrack::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    if (m_view->m_mode == GraphicsScene3dView::ActiveMode::Idle) {
        return false;
    }
    if (event->type() == EpochSelected2d) {
        auto epochEvent = static_cast<EpochEvent*>(event);
        resetVertexSelection();
        m_view->m_mode = GraphicsScene3dView::ActiveMode::BottomTrackVertexSelectionMode;
        selectEpoch(epochEvent->epochIndex(), epochEvent->channel().channelId_);
        m_view->update();
    }
    return false;
}

void BottomTrack::setDatasetPtr(Dataset* datasetPtr) {
    datasetPtr_ = datasetPtr;
}

void BottomTrack::setDataProcessorPtr(DataProcessor* dataProcessorPtr)
{
    dataProcessorPtr_ = dataProcessorPtr;
}

void BottomTrack::actionEvent(ActionEvent actionEvent)
{
    auto minMaxFunc = [this](bool isMin) -> void {
        const auto indices{ RENDER_IMPL(BottomTrack)->selectedVertexIndices_ };
        if (!indices.isEmpty()) {
            QVector<int> sequenceVector;
            sequenceVector.reserve(indices.size());

            for (const auto& verticeIndex : indices) {
                const auto epochIndex{ vertex2Epoch_.value(verticeIndex) };
                if (auto epoch{ datasetPtr_->fromIndex(epochIndex) }; epoch) {
                    sequenceVector.push_back(epochIndex);

                    const float coeff = isMin ? 1.1f : 0.9f;
                    const auto channels = datasetPtr_->channelsList();
                    for (const auto& channel : channels) {
                        if (!isMin) {
                            epoch->setMaxDistProc(channel.channelId_, epoch->distProccesing(channel.channelId_) * coeff);
                        }
                        else {
                            epoch->setMinDistProc(channel.channelId_, epoch->distProccesing(channel.channelId_) * coeff);
                        }
                    }
                }
            }

            sequenceVector.shrink_to_fit();
            const auto subArraysVec{ getSubarrays(sequenceVector) };
            const auto channels = datasetPtr_->channelsList();
            const auto btP = datasetPtr_->getBottomTrackParam();

            for (auto& itm : subArraysVec) {
                if (auto* btp = datasetPtr_->getBottomTrackParamPtr(); btp) {
                    btp->indexFrom = itm.first;
                    btp->indexTo = itm.second;
                    qDebug() << "55555 btp->indexFrom:..." <<  btp->indexFrom << " " <<btp->indexTo;
                    if (channels.size() >= 2) {
                        qDebug() << "channels.size() >= 2.....";
                        QMetaObject::invokeMethod(dataProcessorPtr_, "bottomTrackProcessing", Qt::QueuedConnection,
                                    Q_ARG(DatasetChannel, channels[0]), Q_ARG(DatasetChannel, channels[1]),
                                    Q_ARG(BottomTrackParam, btP), Q_ARG(bool, true),/*manual*/
                                    Q_ARG(bool, false)/*redraw all*/);
                    }
                    else if (channels.size() == 1) {
                        qDebug() << "channels.size() ==1....";
                        QMetaObject::invokeMethod(dataProcessorPtr_, "bottomTrackProcessing", Qt::QueuedConnection,
                                    Q_ARG(DatasetChannel, channels[0]), Q_ARG(DatasetChannel, DatasetChannel()),
                                    Q_ARG(BottomTrackParam, btP), Q_ARG(bool, true),/*manual*/
                                    Q_ARG(bool, false));
                    }
                }
                else {
                    break;
                }
            }

            updateRenderData(0, 0, false, true);
            emit datasetPtr_->dataUpdate();
        }
    };

    switch (actionEvent) {
        case ActionEvent::Undefined: {
            break;
        }
        case ActionEvent::ClearDistProc: {
            const auto indices{ RENDER_IMPL(BottomTrack)->selectedVertexIndices_ };

            if (!indices.isEmpty()) {
                bool isSomethingDeleted{ false };
                for (const auto& verticeIndex : indices) {
                    const auto epochIndex{ vertex2Epoch_.value(verticeIndex) };
                    if (auto epoch{ datasetPtr_->fromIndex(epochIndex) }) {
                        const auto channels = datasetPtr_->channelsList();
                        for (const auto& itm : channels) {
                            epoch->clearDistProcessing(itm.channelId_);
                        }

                        Q_EMIT epochErased(epochIndex);
                        isSomethingDeleted = true;
                    }
                }
                if (isSomethingDeleted) {
                    RENDER_IMPL(BottomTrack)->selectedVertexIndices_.clear();
                    updateRenderData(0, 0, false, true);
                    emit datasetPtr_->dataUpdate();
                }
            }

            break;
        }
        case ActionEvent::MaxDistProc: {
            minMaxFunc(false);
            break;
        }
        case ActionEvent::MinDistProc: {
            minMaxFunc(true);
            break;
        }
        default:
            break;
    }
}

void BottomTrack::isEpochsChanged(int lEpoch, int rEpoch, bool manual, bool redrawAll)
{
    // qDebug() << "BottomTrack::isEpochsChanged lEpoch..." << lEpoch << "  " << rEpoch;
    if(!datasetPtr_) {
        return;
    }

    // updateRenderData(lEpoch, rEpoch, redrawAll, manual);
    auto* r = RENDER_IMPL(BottomTrack);
    r->selectedVertexIndices_.clear();
    int rSize = r->cdata().size();

    DataProcessorType currDataType = datasetPtr_->getDataProcessorState();
    if (currDataType == DataProcessorType::bletoothTrack || currDataType == DataProcessorType::wifiTrack) {
        lEpoch = rEpoch > 1 ? (rEpoch-1) : 0;
    }

    // auto datasetChannels = datasetPtr_->channelsList();
    // if (!datasetChannels.isEmpty()) {
    //     visibleChannel_ = datasetChannels.first();
    // }
    // else {
    //     visibleChannel_ = DatasetChannel();
    // }


    QVector<QVector3D> prepData;
    // qDebug() << "lEpoch....." << lEpoch << "   rEpoch...." << rEpoch;
    for (int epIndx = lEpoch; epIndx < rEpoch; ++epIndx) {
        auto vIt = epoch2Vertex_.find(epIndx);
        if (vIt != epoch2Vertex_.end()) {
            // 情况A: 已存在点，更新Z坐标
            if (auto* ep = datasetPtr_->fromIndex(epIndx); ep) {
                if (auto pos = ep->getSonarPosition().ned; pos.isCoordinatesValid()) {
                    auto vIndx = *vIt;
                    const float dist = -1.f * static_cast<float>(ep->distProccesing(visibleChannel_.channelId_));
                    r->m_data[vIndx].setZ(dist);

                    epIndxUpdated_.push_back(epIndx);
                    vertIndxUpdated_.push_back(vIndx);
                }
            }
        }
        else { // 情况B: 新点，创建3D坐标
            if (auto* ep = datasetPtr_->fromIndex(epIndx); ep) {
                if (auto pos = ep->getSonarPosition().ned; pos.isCoordinatesValid()) {
                    // float dist = -1.f * static_cast<float>(ep->distProccesing(visibleChannel_.channelId_));
                    float dist = datasetPtr_->getDistProccesing_CSV(epIndx);
                    prepData.push_back(QVector3D(pos.n, pos.e, dist));
                    epIndxUpdated_.push_back(epIndx);
                    vertIndxUpdated_.push_back(rSize);

                    vertex2Epoch_.insert(rSize, epIndx);
                    epoch2Vertex_.insert(epIndx, rSize);
                    rSize++;
                }
            }
        }
    }

    // qDebug() << "epIndxUpdated_.size():" << epIndxUpdated_.size() << "  " << vertIndxUpdated_.size();
    emit updatedPoints(epIndxUpdated_, vertIndxUpdated_, manual);  //这句绘制等高线

    SceneObject::appendData(prepData);

    Q_EMIT epochListChanged();
}

QVector3D BottomTrack::convertLLAToNED(const LLA& lla) {
    // 这里需要实现 LLA（经纬度高度）到 NED（北东地）坐标系的转换
    // 具体实现取决于项目的坐标系统设置
    // 以下是一个简化的示例实现：

    // 假设参考点（原点）
    static const LLA originLLA(0.0, 0.0, 0.0);

    // 计算距离和方位角
    double distance = calculateDistance(originLLA, lla);
    double azimuth = calculateAzimuth(originLLA, lla);

    // 转换为 NED 坐标
    double north = distance * cos(azimuth);
    double east = distance * sin(azimuth);
    double down = -lla.altitude; // 高度取负值作为深度

    return QVector3D(north, east, down);
}

double BottomTrack::calculateDistance(const LLA& lla1, const LLA& lla2) {
    // 实现两点之间的距离计算（如 Haversine 公式）
    // 简化示例
    return sqrt(pow(lla1.latitude - lla2.latitude, 2) + pow(lla1.longitude - lla2.longitude, 2)) * 111000; // 粗略估算
}

double BottomTrack::calculateAzimuth(const LLA& lla1, const LLA& lla2) {
    // 实现方位角计算
    // 简化示例
    double dLon = lla2.longitude - lla1.longitude;
    double dLat = lla2.latitude - lla1.latitude;
    return atan2(dLon, dLat);
}



void BottomTrack::setData(const QVector<QVector3D> &data, int primitiveType)
{
    qDebug() << "BottomTrack::setData................";
    if (m_filter) {
        QVector <QVector3D> filteredData;
        m_filter->apply(data, filteredData);
        SceneObject::setData(filteredData, primitiveType);
        return;
    }

    SceneObject::setData(data, primitiveType);
}

void BottomTrack::clearData()
{
    vertex2Epoch_.clear();
    epoch2Vertex_.clear();
    visibleChannel_ = DatasetChannel();

    epIndxUpdated_.clear();
    vertIndxUpdated_.clear();
    // minX_ = std::numeric_limits<float>::max();
    // maxX_ = std::numeric_limits<float>::lowest();
    // minY_ = std::numeric_limits<float>::max();
    // maxY_ = std::numeric_limits<float>::lowest();

    auto r = RENDER_IMPL(BottomTrack);
    r->selectedVertexIndices_.clear();

    SceneObject::clearData();
}

void BottomTrack::resetVertexSelection()
{
    RENDER_IMPL(BottomTrack)->selectedVertexIndices_.clear();
}

void BottomTrack::selectEpoch(int epochIndex, const ChannelId& channelId)
{
    if (m_view->m_mode != GraphicsScene3dView::BottomTrackVertexSelectionMode)
        return;


    if (!datasetPtr_->isContainsChannelInChannelSetup(channelId) || channelId != visibleChannel_.channelId_)
        return;

    if (epochIndex < 0 || epochIndex >= datasetPtr_->size())
        return;

    auto* epoch = datasetPtr_->fromIndex(epochIndex);
    North_East_Down nedPos = epoch->getSonarPosition().ned;

    auto indxFromMap = vertex2Epoch_.key(epochIndex);

    if (!epoch ||
        !nedPos.isCoordinatesValid() ||
        (epochIndex && !indxFromMap)) {
        //qDebug() << "invalid pos on bottom track" << epochIndex << indxFromMap;
        return;
    }

    auto r = RENDER_IMPL(BottomTrack);

    r->selectedVertexIndices_.clear();
    r->selectedVertexIndices_.append(indxFromMap);

    Q_EMIT changed();
}

void BottomTrack::setVisibleState(bool state)
{
    SceneObject::setVisible(state);

    m_view->getBoatTrackPtr()->setBottomTrackVisibleState(state);
}

void BottomTrack::mouseMoveEvent(Qt::MouseButtons buttons, qreal x, qreal y)
{
    Q_UNUSED(buttons)
    Q_UNUSED(x)
    Q_UNUSED(y)

    if (!m_view)  return;

    if (m_view->m_mode == GraphicsScene3dView::BottomTrackVertexSelectionMode) {
        if (buttons.testFlag(Qt::LeftButton)) {
            auto hits = m_view->m_ray.hitObject(shared_from_this(), Ray::HittingMode::Vertex);
            if (!hits.isEmpty()) {
                RENDER_IMPL(BottomTrack)->selectedVertexIndices_ = {hits.first().indices().first};
                auto epochIndex = vertex2Epoch_.value({hits.first().indices().first});

                auto epochEvent = new EpochEvent(EpochSelected3d, datasetPtr_->fromIndex(epochIndex),epochIndex, visibleChannel_);

                QCoreApplication::postEvent(this, epochEvent);
            }
        }
    }

    if(m_view->m_mode == GraphicsScene3dView::BottomTrackVertexComboSelectionMode) {
        RENDER_IMPL(BottomTrack)->selectedVertexIndices_.clear();
        for (int i = 0; i < RENDER_IMPL(BottomTrack)->m_data.size(); i++) {
            auto p = RENDER_IMPL(BottomTrack)->m_data.at(i);
            auto p_screen = p.project(m_view->camera().lock()->viewMatrix()*m_view->m_model,
                            m_view->m_projection, m_view->boundingRect().toRect());

            if (m_view->m_comboSelectionRect.contains(p_screen.x(), p_screen.y()))
                RENDER_IMPL(BottomTrack)->selectedVertexIndices_.append(i);
        }
    }
}

void BottomTrack::mousePressEvent(Qt::MouseButtons buttons, qreal x, qreal y)
{
    Q_UNUSED(buttons)
    Q_UNUSED(x)
    Q_UNUSED(y)

    if (!m_view)
        return;

    if (m_view->m_mode == GraphicsScene3dView::BottomTrackVertexSelectionMode) {
        if (buttons.testFlag(Qt::LeftButton)) {
            auto hits = m_view->m_ray.hitObject(shared_from_this(), Ray::HittingMode::Vertex);
            if (!hits.isEmpty()) {
                auto* r = RENDER_IMPL(BottomTrack);

                r->selectedVertexIndices_ = {hits.first().indices().first};
                auto epochIndex = vertex2Epoch_.value({hits.first().indices().first});
                m_view->getBoatTrackPtr()->selectEpoch(epochIndex);
                auto epochEvent = new EpochEvent(EpochSelected3d, datasetPtr_->fromIndex(epochIndex),epochIndex, visibleChannel_);
                QCoreApplication::postEvent(this, epochEvent);
            }
        }
        /*
        if(!RENDER_IMPL(BottomTrack)->m_selectedVertexIndices.isEmpty()){
            auto epochIndex = m_epochIndexMatchingMap.value(
                        RENDER_IMPL(BottomTrack)->m_selectedVertexIndices.first());

            Q_EMIT epochPressed(epochIndex);
        }
        */
    }
}

void BottomTrack::mouseReleaseEvent(Qt::MouseButtons buttons, qreal x, qreal y)
{
    Q_UNUSED(buttons)
    Q_UNUSED(x)
    Q_UNUSED(y)

    if(!m_view) return;

    auto epochEvent = new EpochEvent(EpochSelected3d, datasetPtr_->fromIndex(-1),-1, visibleChannel_);
    QCoreApplication::postEvent(this, epochEvent);
}

void BottomTrack::keyPressEvent(Qt::Key key)
{
    if (!m_view || !visibleChannel_.channelId_.isValid())
        return;

    if (m_view->m_mode == GraphicsScene3dView::BottomTrackVertexSelectionMode && key == Qt::Key_Delete) {
        const auto indices{ RENDER_IMPL(BottomTrack)->selectedVertexIndices_ };
        bool isSomethingDeleted{ false };
        for (const auto& verticeIndex : indices) {
            const auto epochIndx{ vertex2Epoch_.value(verticeIndex) };
            if (auto epoch{ datasetPtr_->fromIndex(epochIndx) }) {
                const auto channels = datasetPtr_->channelsList(); // TODO
                for (const auto& itm : channels) {
                    epoch->clearDistProcessing(itm.channelId_);
                }

                epoch->clearDistProcessing(visibleChannel_.channelId_);
                Q_EMIT epochErased(epochIndx);
                isSomethingDeleted = true;
            }
        }
        if (isSomethingDeleted) {
            RENDER_IMPL(BottomTrack)->selectedVertexIndices_.clear();
            updateRenderData(0, 0, false, true);
            emit datasetPtr_->dataUpdate();
        }
    }

    if (key == Qt::Key_Delete) {
        const auto indices{ RENDER_IMPL(BottomTrack)->selectedVertexIndices_ };
        if (!indices.isEmpty()) {
            bool isSomethingDeleted{ false };
            for (const auto& verticeIndex : indices) {
                const auto epochIndx{ vertex2Epoch_.value(verticeIndex) };
                if (auto epoch{ datasetPtr_->fromIndex(epochIndx) }) {
                    const auto channels = datasetPtr_->channelsList(); // TODO
                    for (const auto& itm : channels) {
                        epoch->clearDistProcessing(itm.channelId_);
                    }
                    Q_EMIT epochErased(epochIndx);
                    isSomethingDeleted = true;
                }
            }
            if (isSomethingDeleted) {
                RENDER_IMPL(BottomTrack)->selectedVertexIndices_.clear();
                updateRenderData(0, 0, false, true);
                emit datasetPtr_->dataUpdate();
            }
        }
    }
}

void BottomTrack::updateRenderData(int lEpIndx, int rEpIndx, bool redraw, bool manually)
{
    bool redrawAll = redraw;
    if ((!lEpIndx && !rEpIndx) || (lEpIndx == 0 && rEpIndx == datasetPtr_->size())) {
        redrawAll = true;
    }

    if (redrawAll) {
        clearCache();
    }

    const int toIndx   = redrawAll ? datasetPtr_->getLastBottomTrackEpoch() : rEpIndx;
    const int fromIndx = redrawAll ? 0 : lEpIndx;
    if (!redrawAll && fromIndx >= toIndx) {
        return;
    }

    const int need = toIndx - fromIndx;

    auto* r = RENDER_IMPL(BottomTrack);
    r->selectedVertexIndices_.clear();
    int rSize = r->cdata().size();

    QVector<QVector3D> prepData;
    QVector<int>       epIndxUpdated;
    QVector<int>       vertIndxUpdated;
    prepData.reserve(need);
    epIndxUpdated.reserve(need);
    vertIndxUpdated.reserve(need);

    for (int epIndx = fromIndx; epIndx < toIndx; ++epIndx) {
        auto vIt = epoch2Vertex_.find(epIndx);
        if (vIt != epoch2Vertex_.end()) {
            // 情况A: 已存在点，更新Z坐标
            if (auto* ep = datasetPtr_->fromIndex(epIndx); ep) {
                if (auto pos = ep->getSonarPosition().ned; pos.isCoordinatesValid()) {
                    auto vIndx = *vIt;
                    const float dist = -1.f * static_cast<float>(ep->distProccesing(visibleChannel_.channelId_));
                    r->m_data[vIndx].setZ(dist);

                    epIndxUpdated.push_back(epIndx);
                    vertIndxUpdated.push_back(vIndx);
                }
            }
        }
        else { // 情况B: 新点，创建3D坐标
            if (auto* ep = datasetPtr_->fromIndex(epIndx); ep) {
                if (auto pos = ep->getSonarPosition().ned; pos.isCoordinatesValid()) {
                    float dist = datasetPtr_->getDistProccesing_CSV(epIndx);
                    prepData.push_back(QVector3D(pos.n, pos.e, dist));

                    epIndxUpdated.push_back(epIndx);
                    vertIndxUpdated.push_back(rSize);

                    vertex2Epoch_.insert(rSize, epIndx);
                    epoch2Vertex_.insert(epIndx, rSize);

                    rSize++;
                }
            }
        }
    }

    emit updatedPoints(epIndxUpdated, vertIndxUpdated, manually); // for dataHorizon -> dataProcessor

    SceneObject::appendData(prepData);
}

QVector<QPair<int, int>> BottomTrack::getSubarrays(const QVector<int>& sequenceVector)
{
    QVector<QPair<int, int>> retVal;

    if (sequenceVector.isEmpty()) {
        return retVal;
    }

    int start = sequenceVector[0];
    int end = sequenceVector[0];

    for (int i = 1; i < sequenceVector.size(); ++i) {
        if (sequenceVector[i] == end + 1) {
            end = sequenceVector[i];
        } else {
            retVal.append(qMakePair(start, end));
            start = sequenceVector[i];
            end = sequenceVector[i];
        }
    }

    retVal.append(qMakePair(start, end));

    return retVal;
}

void BottomTrack::clearCache()
{
    Q_ASSERT(QThread::currentThread() == thread());
    auto* r = RENDER_IMPL(BottomTrack);
    r->m_data.resize(0);
    // r->m_data.clear();
    vertex2Epoch_.clear();
    epoch2Vertex_.clear();
}

//-----------------------RenderImplementation-----------------------------//
BottomTrack::BottomTrackRenderImplementation::BottomTrackRenderImplementation()
{}

BottomTrack::BottomTrackRenderImplementation::~BottomTrackRenderImplementation()
{}

void BottomTrack::BottomTrackRenderImplementation::render(QOpenGLFunctions *ctx,
                                                          const QMatrix4x4 &mvp,
                                                          const QMap<QString,
                                                          std::shared_ptr<QOpenGLShaderProgram> > &shaderProgramMap) const
{
    Q_UNUSED(ctx);
    Q_UNUSED(mvp);
    Q_UNUSED(shaderProgramMap);
}

void BottomTrack::BottomTrackRenderImplementation::render(QOpenGLFunctions *ctx,
                    const QMatrix4x4 &model, const QMatrix4x4 &view, const QMatrix4x4 &projection,
                    const QMap<QString,std::shared_ptr<QOpenGLShaderProgram>> &shaderProgramMap) const
{
    if (!m_isVisible || !shaderProgramMap.contains("height") || !shaderProgramMap.contains("static")) {
        return;
    }

    //nie:test  这里是带颜色和高度的轨迹线
    // track
    // {
    //     QOpenGLShaderProgram* shaderProgram = nullptr;
    //     int posLoc = -1, maxZLoc = -1, minZLoc = -1, matrixLoc = -1;

    //     shaderProgram = shaderProgramMap["height"].get();
    //     shaderProgram->bind();

    //     int isPointLoc   = shaderProgram->uniformLocation("isPoint");
    //     int isTriangleLoc= shaderProgram->uniformLocation("isTriangle");
    //     shaderProgram->setUniformValue(isPointLoc,    true);
    //     shaderProgram->setUniformValue(isTriangleLoc, false);

    //     maxZLoc = shaderProgram->uniformLocation("max_z");
    //     minZLoc = shaderProgram->uniformLocation("min_z");
    //     shaderProgram->setUniformValue(maxZLoc, m_bounds.maximumZ());
    //     shaderProgram->setUniformValue(minZLoc, m_bounds.minimumZ());

    //     posLoc = shaderProgram->attributeLocation("position");
    //     matrixLoc = shaderProgram->uniformLocation("matrix");

    //     shaderProgram->setUniformValue(matrixLoc, projection * view * model);
    //     shaderProgram->enableAttributeArray(posLoc);
    //     shaderProgram->setAttributeArray(posLoc, m_data.constData());

    //     ctx->glLineWidth(4.0);
    //     ctx->glDrawArrays(m_primitiveType, 0, m_data.size());
    //     ctx->glLineWidth(1.0);

    //     shaderProgram->setUniformValue(isPointLoc,    false);
    //     shaderProgram->setUniformValue(isTriangleLoc, false);

    //     shaderProgram->disableAttributeArray(posLoc);
    //     shaderProgram->release();
    // }

    {
        QOpenGLShaderProgram* shaderProgram = nullptr;
        int colorLoc = -1, posLoc = -1, matrixLoc = -1;

        //------------->Drawing selected vertices<<---------------//
        shaderProgram = shaderProgramMap["static"].get();
        shaderProgram->bind();

        int isPointLoc   = shaderProgram->uniformLocation("isPoint");
        int isTriangleLoc= shaderProgram->uniformLocation("isTriangle");
        shaderProgram->setUniformValue(isPointLoc,    true);
        shaderProgram->setUniformValue(isTriangleLoc, false);

        colorLoc  = shaderProgram->uniformLocation("color");
        matrixLoc = shaderProgram->uniformLocation("matrix");
        posLoc    = shaderProgram->attributeLocation("position");
        int widthLoc  = shaderProgram->uniformLocation("width");

        QVector4D vertexColor(0.91f, 0.25f, 0.2f, 1.0f);

        //TODO: Needs to optimize data preparing
        QVector<QVector3D> selectedVertices;
        selectedVertices.reserve(selectedVertexIndices_.size());
        for (const auto& i : selectedVertexIndices_) {
            selectedVertices.append(m_data.at(i));
        }

        if (selectedVertices.isEmpty()) {
            shaderProgram->disableAttributeArray(posLoc);
            shaderProgram->setUniformValue(isPointLoc,    false);
            shaderProgram->setUniformValue(isTriangleLoc, false);
            shaderProgram->release();
            return;
        }

        shaderProgram->setUniformValue(colorLoc,vertexColor);
        shaderProgram->setUniformValue(matrixLoc, projection * view * model);
        shaderProgram->setUniformValue(widthLoc, 17.0f);
        shaderProgram->enableAttributeArray(posLoc);
        shaderProgram->setAttributeArray(posLoc, selectedVertices.constData());

        ctx->glEnable(34370);
        ctx->glEnable(34913);

        ctx->glDrawArrays(GL_POINTS, 0, selectedVertices.size());
        ctx->glDisable(34370);
        ctx->glDisable(34913);

        shaderProgram->setUniformValue(isPointLoc,    false);
        shaderProgram->setUniformValue(isTriangleLoc, false);

        shaderProgram->disableAttributeArray(posLoc);
        shaderProgram->release();
    }
}

