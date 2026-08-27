#include "dataset.h"

#include "core.h"
extern Core* corePtr;


Dataset::Dataset() :
    // interpolator_(this),
    lastBottomTrackEpoch_(0),
    sonarPosIndx_(0)
{
    qRegisterMetaType<ChannelId>("ChannelId");
    resetDataset();
    pool_.clear();
}

Dataset::~Dataset()
{

}

void Dataset::setState(DatasetState state)
{
    state_ = state;
}

void Dataset::setDataProcessorState(DataProcessorType dataProcessorState)
{
    dataProcessorState_ = dataProcessorState;
}

DataProcessorType Dataset::getDataProcessorState()
{
    return dataProcessorState_;
}

Dataset::DatasetState Dataset::getState() const
{
    return state_;
}

void Dataset::getMaxDistanceRange(float *from, float *to, const ChannelId& channel1,
                        uint8_t subAddressCh1, const ChannelId& channel2, uint8_t subAddressCh2)
{
    const int sz = size();
    float channel1_max = 0;
    float channel2_max = 0;
    for(int iepoch = 0; iepoch < sz; iepoch++) {
        Epoch* epoch = fromIndex(iepoch);
        if (epoch != NULL) {
            if (epoch->chartAvail(channel1, subAddressCh1)) {
                float range = epoch->chart(channel1, subAddressCh1)->range();
                if (channel1_max < range) {
                    channel1_max = range;
                }
            }

            if (epoch->chartAvail(channel2, subAddressCh2)) {
                float range = epoch->chart(channel2, subAddressCh2)->range();
                if (channel2_max < range) {
                    channel2_max = range;
                }
            }
        }
    }

    if (channel1_max > 0) {
        if (channel2_max > 0) {
            *from = -channel1_max;
            *to = channel2_max;
        }
        else {
            *from = 0;
            *to = channel1_max;
        }

    }
    else {
        *from = NAN;
        *to = NAN;
    }
}

int Dataset::getLastBottomTrackEpoch() const
{
    return lastBottomTrackEpoch_;
}

LLARef Dataset::getLlaRef() const
{
    return _llaRef;
}

void Dataset::setLlaRef(const LLARef &val, LlaRefState state)
{
    _llaRef = val;

    llaRefState_ = state;
    emit updatedLlaRef();
}


Dataset::LlaRefState Dataset::getCurrentLlaRefState() const
{
    LlaRefState retVal = llaRefState_;

    switch (state_) {
        case DatasetState::kConnection: { retVal = LlaRefState::kConnection; break; }
        case DatasetState::kFile:       { retVal = LlaRefState::kFile;       break; }
        default: break;
    }

    return retVal;
}

void Dataset::setSonarOffset(float x, float y, float z)
{
    sonarOffset_ = QVector3D(x, y, z);
}

void Dataset::addChart(const ChannelId& channelId, const ChartParameters& chartParams,
                       const QVector<QVector<uint8_t>>& data, bool enableRender)
{
    if (data.empty()) {
        return;
    }

    uint8_t numSubChannels = data.size();
    if (shouldAddNewEpoch(channelId, numSubChannels)) {
        addNewEpoch();
    }

    updateEpochWithChart(channelId, chartParams, data, 0.2, 0.1);
    const int endIndx = endIndex();

    lastAddChartEpochIndx_[channelId] = endIndx;

    for (int i = 0; i < numSubChannels; ++i) {
        validateChannelList(channelId, i);
    }

    if(enableRender) {
        emit dataUpdate();
    }
}

void Dataset::addChartMeta(const ChannelId& channelId, const ChartParameters& chartParams, bool enableRender)
{
    uint8_t numSubChannels = 1;
    if (shouldAddNewEpoch(channelId, numSubChannels)) {
        addNewEpoch();
    }

    Epoch* lastEp = last();
    if (!lastEp) {
        return;
    }

    lastEp->setChartParameters2(channelId, chartParams);

    const int endIndx = endIndex();
    lastAddChartEpochIndx_[channelId] = endIndx;
    validateChannelList(channelId, 0);
    if(enableRender) {
        emit dataUpdate();
    }
}

void Dataset::addPosition(double lat, double lon, uint32_t unix_time, int32_t nanosec)
{
    Epoch* lastEp = last();
    if (!lastEp) {
        return;
    }

    Position pos;
    pos.lla = LLA(lat, lon);
    pos.time = DateTime(unix_time, nanosec);
    const bool oneHzNoTimestamp = (unix_time == 0 && nanosec == 0);

    if (pos.lla.isCoordinatesValid()) {
        if (lastEp->getPositionGNSS().lla.isCoordinatesValid()) {
            lastEp = addNewEpoch();
        }
        uint64_t lastIndx = pool_.size() - 1;
        if (!getLlaRef().isInit) {
            LlaRefState llaState = state_ == DatasetState::kUndefined ? LlaRefState::kFile :
                        (state_ == DatasetState::kFile ? LlaRefState::kFile :  LlaRefState::kConnection);
            setLlaRef(LLARef(pos.lla), llaState);
        }
        lastEp->setPositionLLA(pos);
        lastEp->setPositionRef(&_llaRef);

        lastEp->setPositionDataType(DataType::kRaw);

        if (Epoch* prevEp = lastlast(); prevEp) {
            const auto& prev = prevEp->getPositionGNSS();
            if (prev.lla.isCoordinatesValid()) {
                const double dist = distanceMetersLLA(prev.lla.latitude, prev.lla.longitude, pos.lla.latitude,  pos.lla.longitude);

                if (oneHzNoTimestamp) {
                    speed_ = (dist / 0.1) * 3.6; // TODO: kostyl
                }
                else {
                    const auto& c = pos.time;
                    const auto& p = prev.time;

                    int64_t dsec  = int64_t(c.sec)     - int64_t(p.sec);
                    int64_t dnano = int64_t(c.nanoSec) - int64_t(p.nanoSec);
                    if (dnano < 0) {
                        dsec -= 1;
                        dnano += 1000000000;
                    }

                    double dt = double(dsec) + double(dnano) * 1e-9;
                    if (dt <= 0.0) {
                        dt = 1.0;
                    }

                    speed_ = (dist / dt) * 3.6;
                }

                emit speedChanged();
            }
        }

        //qDebug() << "add pos for" << lastIndx;

        boatLatitute_  = pos.lla.latitude;
        boatLongitude_ = pos.lla.longitude;

        emit positionAdded(lastIndx);
        emit dataUpdate();
        emit lastPositionChanged();
    }
}

void Dataset::addPosition_realTime(double lat, double lon, double depth, bool isRead)
{
    Epoch* lastEp = last();
    if (!lastEp) {
        return;
    }

    Position pos;
    pos.lla = LLA(lat, lon);

    if (pos.lla.isCoordinatesValid()) {
        if (lastEp->getPositionGNSS().lla.isCoordinatesValid()) {
            lastEp = addNewEpoch();
        }
        uint64_t lastIndx = pool_.size() - 1;
        if (!getLlaRef().isInit) {
            LlaRefState llaState = state_ == DatasetState::kUndefined ? LlaRefState::kFile :
                                (state_ == DatasetState::kFile ? LlaRefState::kFile :  LlaRefState::kConnection);
            setLlaRef(LLARef(pos.lla), llaState);
        }
        lastEp->setPositionLLA(pos);
        lastEp->setPositionRef(&_llaRef);
        // qDebug() << "_llaRef: longitude:" << _llaRef.refLla.longitude << "  latitude:" <<
        //     _llaRef.refLla.latitude << "  " << _llaRef.refLla.altitude;
        lastEp->setPositionDataType(DataType::kRaw);
        // interpolator_.interpolatePos(false);

        //qDebug() << "add pos for" << lastIndx;
        boatLatitute_  = pos.lla.latitude;
        boatLongitude_ = pos.lla.longitude;
        if(isRead) {
            emit positionAdded(lastIndx);
        }
    }
}


void Dataset::addPosition_file(double lat, double lon, int depth, bool enableRender)
{
    Epoch* lastEp = last();
    if (!lastEp) {
        return;
    }

    Position pos;
    pos.lla = LLA(lat, lon);
    if (!pos.lla.isCoordinatesValid()) {
        return;
    }

    if (lastEp->getPositionGNSS().lla.isCoordinatesValid()) {
        lastEp = addNewEpoch();  //不断累加帧数的下标index
    }

    // qDebug() << "pool_size()............... " << pool_.size();
    uint64_t poolCnt = pool_.size();
    if (!getLlaRef().isInit) {
        LlaRefState llaState = state_ == DatasetState::kUndefined ? LlaRefState::kFile :
                    (state_ == DatasetState::kFile ? LlaRefState::kFile :  LlaRefState::kConnection);
        // qDebug() << "llaState........" << (int)llaState << "  " << (int)state_;
        setLlaRef(LLARef(pos.lla), llaState);
    }

    lastEp->setPositionLLA(pos);
    lastEp->setPositionRef(&_llaRef); //将LLA坐标转化成本地NED坐标
    lastEp->setPositionDataType(DataType::kRaw);

    North_East_Down curNed = lastEp->getPositionGNSS().ned;
    QVector3D new3DData = QVector3D(curNed.n, curNed.e, 0);
    minX_ = std::min(minX_, new3DData.x());
    maxX_ = std::max(maxX_, new3DData.x());
    minY_ = std::min(minY_, new3DData.y());
    maxY_ = std::max(maxY_, new3DData.y());

    if (poolCnt >= 2) {
        Epoch* prevEp = fromIndex(poolCnt - 2);
        if (prevEp && prevEp->getPositionGNSS().ned.isCoordinatesValid()) {
            North_East_Down prevNed = prevEp->getPositionGNSS().ned;
            double dn    = curNed.n - prevNed.n;
            double de    = curNed.e - prevNed.e;
            double dist  = dn * dn + de * de;
            if (dist > 1000.0) {
                lastEp->isRegionStart_ = true;
                return;
            }
        }
    }

    boatLatitute_  = pos.lla.latitude;
    boatLongitude_ = pos.lla.longitude;

    if(enableRender) {
        emit positionAdded(poolCnt-1);
    }

}

void Dataset::positionAddedDone()
{
    uint64_t poolCnt = pool_.size();
    if(poolCnt > 2) {
        qDebug() << "222222222222";
        emit positionAdded(poolCnt-1);
    }

    emit dataUpdate();
    qDebug() << "111111111111";
}

void Dataset::location(double lat, double lon)
{
    LLA lla = LLA(lat, lon);
    if (lla.isCoordinatesValid()) {
        _llaRef = LLARef(lla);
        emit locationToDest(lla);
    }
    else {
        GIF->dialogInfo(Dialog_OK, tr("Invalid Coordinates!"));
    }
}

void Dataset::resetDataset()
{
    {
       QWriteLocker locker(&lock_);
       channelsSetup_.clear();
       firstChannelId_ = DatasetChannel();
    }

    resetRenderBuffers();

    usingRecordParameters_.clear();
    lastAddChartEpochIndx_.clear();
    channelsToResizeEthData_.clear();
    polygonOutlineNED_.clear();

    boatLatitute_         = 0.0f;
    boatLongitude_        = 0.0f;
    lastDepth_            = 0.0f;
    speed_                = 0.0f;
    sonarPosIndx_         = 0;
    _llaRef.isInit        = false;

    emit lastDepthChanged();
    emit channelsUpdated();
    emit dataUpdate();
    emit lastPositionChanged();
}

void Dataset::resetRenderBuffers()
{
    vec_CSV_.clear();
    {
        //加poolMtx_写锁，等待ComputeWorker读完再清空
        QWriteLocker wl(&poolMtx_);
        pool_.clear();
        pool_.shrink_to_fit();
    }
    autoBoundary_.clear();
    minX_ = std::numeric_limits<float>::max();
    maxX_ = std::numeric_limits<float>::lowest();
    minY_ = std::numeric_limits<float>::max();
    maxY_ = std::numeric_limits<float>::lowest();
    _lastYaw   = 0;
    _lastPitch = 0;
    _lastRoll  = 0;
    // interpolator_.clear();
    _llaRef = LLARef();
    lastBottomTrackEpoch_ = 0;
}

void Dataset::resetPolygonOutline()
{
    polygonOutline_.clear();
    polygonOutlineNED_.clear();
}

void Dataset::clearBoundary()
{
    autoBoundary_.clear();
}


void Dataset::triggerRenderUpdate()
{
    emit dataUpdate();
    const int endIndx = endIndex();
    if (endIndx >= 0) {
        emit positionAdded(endIndx);
    }
}

void Dataset::preallocatePool(int capacity)
{
    if (capacity <= 0) return;
    QWriteLocker wl(&poolMtx_);
    pool_.reserve(capacity);
}


void Dataset::removeFrames(int startIndex, int endIndex)
{
    if(startIndex > endIndex) {
        std::swap(startIndex, endIndex);
    }
    const int sz = pool_.size();
    if(startIndex < 0) {
        startIndex = 0;
    }
    if(endIndex >= sz) {
        endIndex = sz - 1;
    }
    if((startIndex > endIndex) ||(startIndex >= sz)) {
        return;
    }
    QWriteLocker wl(&poolMtx_);
    pool_.remove(startIndex, endIndex-startIndex+1);
}

void Dataset::setChannelOffset(const ChannelId& channelId, float x, float y, float z)
{
    QWriteLocker locker(&lock_);

    // write to all on ChannelId
    for (int16_t i = 0; i < channelsSetup_.size(); ++i) {
        if (channelsSetup_.at(i).channelId_ == channelId) {
            channelsSetup_[i].localPosition_.x = x;
            channelsSetup_[i].localPosition_.y = y;
            channelsSetup_[i].localPosition_.z = z;
        }
    }
}

void Dataset::spatialProcessing()
{
    auto ch_list = channelsList();
    for (auto it = ch_list.cbegin(); it != ch_list.cend(); ++it) {
        ChannelId ich = it->channelId_;

        for(int iepoch = 0; iepoch < size(); iepoch++) {
            Epoch* epoch = fromIndex(iepoch);
            if(epoch == NULL) { continue; }

            Position ext_pos = epoch->getExternalPosition();

            if(epoch->chartAvail(ich)) {
                Epoch::Echogram* data = epoch->chart(ich);

                if(data == NULL) { continue; }

                if(ext_pos.ned.isValid()) {
                    ext_pos.ned.d += it->localPosition_.z;
                }

                if(ext_pos.lla.isValid()) {
                    ext_pos.lla.altitude -= it->localPosition_.z;
                }

                data->sensorPosition = ext_pos;

                if(ext_pos.ned.isValid()) {
                    ext_pos.ned.d += data->bottomProcessing.getDistance();
                }

                if(ext_pos.lla.isValid()) {
                    ext_pos.lla.altitude -= data->bottomProcessing.getDistance();
                }

                data->bottomProcessing.bottomPoint = ext_pos;
            }
        }
    }
}

void Dataset::setRefPosition(int epoch_index)
{
    qDebug() << "Dataset::setRefPosition000000.................";
    Epoch*  ref_epoch = fromIndex(epoch_index);
    setRefPosition(ref_epoch);
}

void Dataset::setRefPosition(Epoch* epoch)
{
    qDebug() << "Dataset::setRefPosition1111111..............";
    if(epoch == NULL) { return; }

    setRefPosition(epoch->getPositionGNSS());
}

void Dataset::setRefPosition(Position ref_pos)
{
    // qDebug() << "Dataset::setRefPosition2222222222222222...............";
    if(ref_pos.lla.isCoordinatesValid()) {
        setLlaRef(LLARef(ref_pos.lla), getCurrentLlaRefState());
        for(int iepoch = 0; iepoch < size(); iepoch++) {
            Epoch* epoch = fromIndex(iepoch);
            if(epoch == NULL) { continue; }
            epoch->setPositionRef(&_llaRef);
            // qDebug() << "Dataset::setRefPosition _llaRef " << _llaRef.refLla.longitude << " " << _llaRef.refLla.latitude
            //          <<"   " << _llaRef.refLla.altitude;
        }
    }
}

void Dataset::setRefPositionByFirstValid()
{
    // qDebug() << "Dataset::setRefPositionByFirstValid..................";
    Epoch* epoch = getFirstEpochByValidPosition();
    if(epoch == NULL) { return; }

    setRefPosition(epoch);
}

Epoch *Dataset::getFirstEpochByValidPosition()
{
    for(int iepoch = 0; iepoch < size(); iepoch++) {
        Epoch* epoch = fromIndex(iepoch);
        if(epoch == NULL) { continue; }
        if(epoch->getPositionGNSS().lla.isCoordinatesValid()) {
            return epoch;
        }
    }

    return NULL;
}

QStringList Dataset::channelsNameList()
{
    channelsNames_.clear();
    channelsIds_.clear();
    subChannelIds_.clear();

    QStringList result;

    result << QString(tr("None"));

    channelsNames_ << QString(tr("None"));
    channelsIds_   << ChannelId();
    subChannelIds_ << 0x00;

    const QVector<DatasetChannel> chList = channelsList();

    for (const auto& channel : chList) {

        const ChannelId& chId = channel.channelId_;
        uint8_t sub = channel.subChannelId_;

        QString name = QString("%1|%2|%3").arg(channel.portName_, QString::number(channel.channelId_.address), QString::number(sub));

        result << name;

        channelsNames_ << name;
        channelsIds_   << chId;
        subChannelIds_ << sub;
    }

    return result;
}

void Dataset::onLastBottomTrackEpochChanged(const ChannelId& channelId, int val, const BottomTrackParam& btP, bool manual, bool redrawAll)
{
    // qDebug() << "Dataset::onLastBottomTrackEpochChanged.............";
    bottomTrackParam_     = btP;
    lastBottomTrackEpoch_ = val;

    emit bottomTrackUpdated(channelId, bottomTrackParam_.indexFrom, bottomTrackParam_.indexTo, manual, redrawAll);
}

void Dataset::validateChannelList(const ChannelId &channelId, uint8_t subChannelId)
{
    int16_t indx = -1;

    {
        QWriteLocker locker(&lock_);

        if (channelsSetup_.empty()) {
            firstChannelId_ = DatasetChannel(channelId, subChannelId);
        }

        for (int16_t i = 0; i < channelsSetup_.size(); ++i) {
            if (channelsSetup_.at(i).channelId_ == channelId && channelsSetup_.at(i).subChannelId_ == subChannelId) {
                indx = i;
                break;
            }
        }

        if (indx != -1) {
            if (channelsSetup_[indx].portName_.isEmpty()) {
                auto links = corePtr->getLinkNames();
                if (links.contains(channelId.uuid)) {
                    channelsSetup_[indx].portName_ = links[channelId.uuid];
                }
            }

            channelsSetup_[indx].counter();
        }
        else {
            auto newDCh = DatasetChannel(channelId, subChannelId);
            auto links = corePtr->getLinkNames();

            if (links.contains(channelId.uuid)) {
                newDCh.portName_ = links[channelId.uuid];
            }
            else {
                newDCh.portName_ = "None";
            }

            channelsSetup_.push_back(newDCh);
        }
    }

    if (indx == -1) {
        emit channelsUpdated();
    }
}


Epoch *Dataset::addNewEpoch()
{
    bool beenAdded = false;
    int indxAdded = -1;
    Epoch* ptrAdded = nullptr;

    {
        QWriteLocker wl(&poolMtx_);

        uint64_t newSize = pool_.size() + 1;
        pool_.resize(newSize);
        ptrAdded = last();

        beenAdded = true;
        indxAdded = newSize;
    }

    if (beenAdded) {
        emit epochAdded(indxAdded);
    }

    return ptrAdded;
}
Epoch* Dataset::addNewEpochPolygonOutline()
{
    bool beenAdded = false;
    int indxAdded = -1;
    Epoch* ptrAdded = nullptr;

    {
        QWriteLocker wl(&polygonOutlineMtx_);

        uint64_t newSize = polygonOutline_.size() + 1;
        polygonOutline_.resize(newSize);
        ptrAdded = lastPolygonOutline();

        beenAdded = true;
        indxAdded = newSize;
    }

    if (beenAdded) {
        emit epochAdded(indxAdded);
    }

    return ptrAdded;
}

void Dataset::modifyPolygonOutlineEpoch(int index, const Epoch& epoch)
{
    QWriteLocker wl(&polygonOutlineMtx_);
    polygonOutline_[index] = epoch;
}

bool Dataset::shouldAddNewEpoch(const ChannelId &channelId, uint8_t numSubChannels) const
{
    const int lastIndx = endIndex();

    if (lastIndx == -1) {
        return true;
    }

    const auto& epoch = pool_[lastIndx];

    for (int i = 0; i < numSubChannels; ++i) {
        if (!epoch.chartAvail(channelId, i)) {
            return false;
        }
    }

    return true;
}

void Dataset::updateEpochWithChart(const ChannelId &channelId, const ChartParameters &chartParams,
                                const QVector<QVector<uint8_t>> &data, float resolution, float offset)
{
    const int indx = endIndex();
    auto& epoch = pool_[indx];

    RecordParameters recParam;
    if (usingRecordParameters_.contains(channelId)) {
        recParam = usingRecordParameters_[channelId];
    }

    epoch.setChart(channelId, data, resolution, offset);
    epoch.setRecParameters(channelId, recParam);
    epoch.setChartParameters(channelId, chartParams);
}

void Dataset::setLastDepth(float val)
{
    lastDepth_ = val;

    emit lastDepthChanged();
}

std::tuple<ChannelId, uint8_t, QString>  Dataset::channelIdFromName(const QString& name) const
{
    auto retVal = std::make_tuple(ChannelId(), 0x00, QString());

    if (name.isEmpty() || channelsNames_.isEmpty() || channelsIds_.size() != channelsNames_.size() ||
        subChannelIds_.size() != channelsNames_.size()) {

        return retVal;
    }

    int index = channelsNames_.indexOf(name);

    if (index >= 0 && index < channelsIds_.size()) {
        return std::make_tuple(channelsIds_[index], subChannelIds_[index], channelsNames_[index]);
    }

    return retVal;
}

void Dataset::onSonarPosCanCalc(uint64_t indx)
{
    for (uint64_t i = sonarPosIndx_ + 1; i <= indx; ++i) {
        if (auto* ep = fromIndex(i); ep) {
            if (sonarOffset_.isNull()) {
                ep->setSonarPosition(ep->getPositionGNSS());
            }
            else {
                Position boatPos = ep->getPositionGNSS();
                const North_East_Down d = fruOffsetToNed(sonarOffset_, ep->yaw());
                North_East_Down sonarNed(boatPos.ned.n + d.n, boatPos.ned.e + d.e, /*always zero*/0.0);
                LLA sonarLla(&sonarNed, &_llaRef, /*spherical=*/true);
                boatPos.lla      = sonarLla;
                boatPos.LLA2NED(&_llaRef); // ned
                ep->setSonarPosition(boatPos);
            }

            ep->setSonarPositionDataType(ep->getPositionDataType());
        }
    }

    sonarPosIndx_ = indx;
}
