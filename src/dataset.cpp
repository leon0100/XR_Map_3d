#include "dataset.h"

#include "core.h"
extern Core* corePtr;


Dataset::Dataset() :
    interpolator_(this),
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

#if defined(FAKE_COORDS)
void Dataset::setActiveZeroing(bool state)
{
    activeZeroing_ = state;
}
#endif

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

void Dataset::addEvent(int timestamp, int id, int unixt) {
    qDebug() << "Dataset::addEvent................";
    lastEventTimestamp = timestamp;
    lastEventId = id;

    // addNewEpoch();

    {
        QWriteLocker wl(&poolMtx_);
        pool_[endIndex()].setEvent(timestamp, id, unixt);
    }

    emit dataUpdate();
}

void Dataset::addEncoder(float angle1_deg, float angle2_deg, float angle3_deg) {
    Epoch* last_epoch = last();
    if (!last_epoch) {
        return;
    }
    if(last_epoch->isEncodersSeted()) {
        // last_epoch = addNewEpoch();
    }

    last_epoch->setEncoders(angle1_deg, angle2_deg, angle3_deg);
    qDebug("Encoder was added");
    emit dataUpdate();
}

void Dataset::addTimestamp(int timestamp) {
    Q_UNUSED(timestamp);
}

void Dataset::setTranscSetup(const ChannelId& channelId, uint16_t freq, uint8_t pulse, uint8_t boost)
{
    usingRecordParameters_[channelId].freq  = freq;
    usingRecordParameters_[channelId].pulse = pulse;
    usingRecordParameters_[channelId].boost = boost;
}

void Dataset::setSoundSpeed(const ChannelId& channelId, uint32_t soundSpeed)
{
    usingRecordParameters_[channelId].soundSpeed  = soundSpeed;
}

void Dataset::setSonarOffset(float x, float y, float z)
{
    sonarOffset_ = QVector3D(x, y, z);
}

void Dataset::setChartSetup(const ChannelId& channelId, uint16_t resol, uint16_t count, uint16_t offset)
{
    usingRecordParameters_[channelId].resol  = resol;
    usingRecordParameters_[channelId].count = count;
    usingRecordParameters_[channelId].offset = offset;

    channelsToResizeEthData_.insert(channelId);
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

    // TODO:不只是迭代次数的回溯窗口，而是海图层面的最后一个未变更版本
    if(enableRender) {
        emit dataUpdate();
    }
    // emit chartAdded(lastIndx);
}


void Dataset::addDist(const ChannelId& channelId, int dist)
{
    int pool_index = endIndex();

    if (pool_index < 0 || pool_[pool_index].distAvail() == true) {
        qDebug() << "Dataset::addDist...................";
        addNewEpoch();
        pool_index = endIndex();
    }

    pool_[pool_index].setDist(channelId, dist);

    setLastDepth(dist);

    emit dataUpdate();
}

void Dataset::addRangefinder(const ChannelId& channelId, float distance)
{
    Epoch* epoch = last();
    if (!epoch) {
        return;
    }
    if (epoch->distAvail()) {
        // epoch = addNewEpoch();
    }

    setLastDepth(distance);

    epoch->setDist(channelId, distance * 1000);

    emit dataUpdate();
}

void Dataset::addUsblSolution(IDBinUsblSolution::UsblSolution data) {
    int pool_index = endIndex();
    if(pool_index < 0 || pool_[pool_index].isUsblSolutionAvailable() == true) {
        // addNewEpoch();
        pool_index = endIndex();
    }

    tracks[-1].data_.append(QVector3D());
    tracks[-1].objectColor_ = QColor(0, 255, 255);

    Position pos;
    pos.lla = LLA(data.usbl_latitude, data.usbl_longitude);

    static float dist_save = NAN;
    static float angl_save = NAN;

    Q_UNUSED(dist_save);
    Q_UNUSED(angl_save);

    float angl_usbl = data.azimuth_deg;
    float dist = data.distance_m;

    if(pos.lla.isCoordinatesValid()) {
        setLlaRef(LLARef(pos.lla), getCurrentLlaRefState());

        pos.LLA2NED(&_llaRef);
        // qDebug("usbl x %f, y %f", pos.ned.n, pos.ned.e);

        tracks[-2].data_.append(QVector3D(pos.ned.n, pos.ned.e, 0));
        tracks[-2].objectColor_ = QColor(0, 200, 0);

        float beacon_n = data.beacon_n;
        float beacon_e = data.beacon_e;

        if(pos.ned.isCoordinatesValid()) {
            beacon_n += pos.ned.n;
            beacon_e += pos.ned.e;
        }

        tracks[-4].data_.append(QVector3D(beacon_n, beacon_e, 0));
        tracks[-4].objectColor_ = QColor(200, 0, 0);
        tracks[-4].lineWidth_ = 5;

    } else {
         tracks[-4].data_.append(QVector3D(NAN, NAN, 0));
    }
    dist_save = dist;
    angl_save = angl_usbl;

    std::shared_ptr<UsblView> view = scene3dViewPtr_->getUsblViewPtr();
    view->setTrackRef(tracks);

    pool_[endIndex()].setAtt(data.usbl_yaw, data.usbl_pitch, data.usbl_roll);
    pool_[endIndex()].set(data);
    emit dataUpdate();
}

void Dataset::addDopplerBeam(IDBinDVL::BeamSolution *beams, uint16_t cnt) {
    int pool_index = endIndex();

    if(pool_index < 0 || (pool_[pool_index].isDopplerBeamAvail() == true)) {
        // addNewEpoch();
    }

    pool_index = endIndex();

    pool_[pool_index].setDopplerBeam(beams, cnt);
    emit dataUpdate();
}

void Dataset::addDVLSolution(IDBinDVL::DVLSolution dvlSolution) {
    int pool_index = endIndex();

    if(pool_index < 0 || (pool_[pool_index].isDopplerBeamAvail() == false)) {
        // addNewEpoch();
        pool_index = endIndex();
    }

    pool_[pool_index].setDVLSolution(dvlSolution);
    emit dataUpdate();
}

void Dataset::addAtt(float yaw, float pitch, float roll)
{
    uint64_t lastIndx = pool_.size() - 1;

    Epoch* last_epoch = last();
    if (!last_epoch) {
        return;
    }
    if(last_epoch->isAttAvail()) {
        // last_epoch = addNewEpoch();
    }

    last_epoch->setAtt(yaw, pitch, roll);

    _lastYaw = yaw;
    _lastPitch = pitch;
    _lastRoll = roll;

    interpolator_.interpolateAtt(false);

    emit attitudeAdded(lastIndx);
    emit dataUpdate();
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
        interpolator_.interpolatePos(false);

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

        if (isValidActiveContactIndx()) {
            if (auto* ep = fromIndex(activeContactIndx_); ep) {
                const double latTarget = ep->contact_.lat;
                const double lonTarget = ep->contact_.lon;
                const double latBoat   = pos.lla.latitude;
                const double lonBoat   = pos.lla.longitude;
                distToActiveContact_ = distanceMetersLLA(latBoat, lonBoat, latTarget, lonTarget);

                const double yawDeg = _lastYaw;
                if (qIsFinite(yawDeg)) {
                    angleToActiveContact_ = angleToTargetDeg(latBoat, lonBoat, latTarget, lonTarget, yawDeg);
                }
            }
        }
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
        interpolator_.interpolatePos(false);

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
        lastEp->setDistProcesing_CSV(depth);
    }

    // qDebug() << "pool_size().............................. " << pool_.size();
    uint64_t poolCnt = pool_.size();
    if (!getLlaRef().isInit) {
        LlaRefState llaState = state_ == DatasetState::kUndefined ? LlaRefState::kFile :
                    (state_ == DatasetState::kFile ? LlaRefState::kFile :  LlaRefState::kConnection);
        setLlaRef(LLARef(pos.lla), llaState);
    }

    lastEp->setPositionLLA(pos);
    lastEp->setPositionRef(&_llaRef); //在这里将LLA坐标转化成本地NED坐标
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
            // North_East_Down curNed  = lastEp->getPositionGNSS().ned;
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

void Dataset::addPositionRTK(Position position) {
    Epoch* last_epoch = last();
    if (!last_epoch) {
        return;
    }
    last_epoch->setExternalPosition(position);
}

void Dataset::addDepth(float depth) {
    Epoch* last_epoch = last();
    if (!last_epoch) {
        return;
    }
    if(last_epoch->isDepthAvail()) {
        // last_epoch = addNewEpoch();
    }

    setLastDepth(depth);

    last_epoch->setDepth(depth);
}

void Dataset::addGnssVelocity(double h_speed, double course) {
    // qDebug() << "Dataset::addGnssVelocity............";
    int pool_index = endIndex();
    if(pool_index < 0) {
        // addNewEpoch();
        pool_index = endIndex();
    }

    pool_[pool_index].setGnssVelocity(h_speed, course);
    emit dataUpdate();
}

void Dataset::addTemp(float temp_c) {
    //qDebug() << "Dataset::addTemp" << temp_c;
    lastTemp_ = temp_c;
    Epoch* last_epoch = last();
    if (!last_epoch) {
        return;
    }
    last_epoch->setTemp(temp_c);
}

void Dataset::mergeGnssTrack(QList<Position> track) {
    const int64_t max_difference_ns = 1e9;
    const int psize = size();
    const int tsize = track.size();
    int track_pos_save = 0;
    volatile int sync_count = 0;

    for(int iepoch = 0; iepoch < psize; iepoch++) {
        Epoch* epoch =  fromIndex(iepoch);
        Position boatPos = epoch->getPositionGNSS();

        DateTime time_epoch = *epoch->time();
        if(time_epoch.sec > 0) {
            boatPos.time = *epoch->time();
            boatPos.time.sec -= 18;
        }

        int64_t internal_ns  = boatPos.time.sec*1e9+boatPos.time.nanoSec;


        if(internal_ns > 0) {
            int64_t min_dif_ns = max_difference_ns;
            int min_ind = -1;
            for(int track_pos = track_pos_save; track_pos < tsize;track_pos++) {
                int64_t track_ns  = track[track_pos].time.sec*1e9+track[track_pos].time.nanoSec;
                if(track_ns > 0) {
                    int64_t dif_ns = track_ns - internal_ns;
                    if(min_dif_ns > abs(dif_ns)) {
                        min_dif_ns = abs(dif_ns);
                        min_ind = track_pos;
                    }

                    if(dif_ns > max_difference_ns) { break; }
                }
            }

            if(min_ind > 0) {
                track_pos_save = min_ind;
                epoch->setExternalPosition(track[min_ind]);
                sync_count++;
            }
        }
    }
    emit dataUpdate();

    // emit positionComplete(track.last().lla.latitude, track.last().lla.longitude, "", "");

}


void Dataset::resetDataset()
{
    {
        QWriteLocker locker(&lock_);
        channelsSetup_.clear();
        firstChannelId_ = DatasetChannel();
    }

    resetRenderBuffers();
    resetDistProcessing();

    usingRecordParameters_.clear();
    lastAddChartEpochIndx_.clear();
    channelsToResizeEthData_.clear();
    polygonOutlineNED_.clear();
    autoBoundary_.clear();

    activeContactIndx_    = -1;
    boatLatitute_         = 0.0f;
    boatLongitude_        = 0.0f;
    distToActiveContact_  = 0.0f;
    angleToActiveContact_ = 0.0f;
    lastDepth_            = 0.0f;
    speed_                = 0.0f;
    sonarPosIndx_         = 0;
    minX_ = std::numeric_limits<float>::max();
    maxX_ = std::numeric_limits<float>::lowest();
    minY_ = std::numeric_limits<float>::max();
    maxY_ = std::numeric_limits<float>::lowest();

    _llaRef.isInit        = false;

    emit lastDepthChanged();
    emit channelsUpdated();
    emit dataUpdate();
    emit lastPositionChanged();
    emit activeContactChanged();
}

void Dataset::resetRenderBuffers()
{
    tracks.clear();
    vec_CSV_.clear();
    {
        //加poolMtx_写锁，等待ComputeWorker读完再清空
        QWriteLocker wl(&poolMtx_);
        pool_.clear();
        pool_.shrink_to_fit();
    }
    minX_ = std::numeric_limits<float>::max();
    maxX_ = std::numeric_limits<float>::lowest();
    minY_ = std::numeric_limits<float>::max();
    maxY_ = std::numeric_limits<float>::lowest();
    _lastYaw   = 0;
    _lastPitch = 0;
    _lastRoll  = 0;
    lastTemp_  = NAN;
    interpolator_.clear();
    _llaRef = LLARef();
    // bSProc_->clear();
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

void Dataset::resetDistProcessing() {
//     int pool_size = size();
//     for(int i = 0; i < pool_size; i++) {
// //        Epoch* dataset = fromIndex(i);
// //        dataset->resetDistProccesing();
//     }
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

void Dataset::spatialProcessing() {
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

void Dataset::usblProcessing() {
    const int to_size = size();
    int from_index = 0;

    _beaconTrack.clear();
    _beaconTrack1.clear();

    for(int i = from_index; i < to_size; i+=1) {
        Epoch* epoch = fromIndex(i);
        Position boatPos = epoch->getPositionGNSS();

        if(boatPos.ned.isCoordinatesValid() && epoch->isAttAvail() && epoch->isUsblSolutionAvailable()) {
            double n = boatPos.ned.n, e = boatPos.ned.e;
            Q_UNUSED(n);
            Q_UNUSED(e);
            double yaw = epoch->yaw();
            double azimuth = epoch->usblSolution().azimuth_deg-180;
            double dist = epoch->usblSolution().distance_m;
            double dir = ((yaw + azimuth) + 120);
            double rel_n = dist*cos(qDegreesToRadians(dir));
            double rel_e = dist*sin(qDegreesToRadians(dir));
            Q_UNUSED(rel_n);
            Q_UNUSED(rel_e);

        }
    }
}

void Dataset::setRefPosition(int epoch_index) {
    qDebug() << "Dataset::setRefPosition000000.................";
    Epoch*  ref_epoch = fromIndex(epoch_index);
    setRefPosition(ref_epoch);
}

void Dataset::setRefPosition(Epoch* epoch) {
    qDebug() << "Dataset::setRefPosition1111111..............";
    if(epoch == NULL) { return; }

    setRefPosition(epoch->getPositionGNSS());
}

void Dataset::setRefPosition(Position ref_pos) {
    qDebug() << "Dataset::setRefPosition2222222222222222...............";
    if(ref_pos.lla.isCoordinatesValid()) {
        setLlaRef(LLARef(ref_pos.lla), getCurrentLlaRefState());
        qDebug() << "Dataset::setRefPosition.size() " << size();
        for(int iepoch = 0; iepoch < size(); iepoch++) {
            Epoch* epoch = fromIndex(iepoch);
            if(epoch == NULL) { continue; }
            epoch->setPositionRef(&_llaRef);
            // qDebug() << "Dataset::setRefPosition _llaRef " << _llaRef.refLla.longitude << " " << _llaRef.refLla.latitude
            //          <<"   " << _llaRef.refLla.altitude;
        }
    }
}

void Dataset::setRefPositionByFirstValid() {
    qDebug() << "Dataset::setRefPositionByFirstValid..................";
    Epoch* epoch = getFirstEpochByValidPosition();
    if(epoch == NULL) { return; }

    setRefPosition(epoch);
}

Epoch *Dataset::getFirstEpochByValidPosition() {
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

void Dataset::interpolateData(bool fromStart)
{
    interpolator_.interpolatePos(fromStart);
    interpolator_.interpolateAtt(fromStart);
}


void Dataset::onDistCompleted(int epIndx, const ChannelId& channelId, float dist)
{
    // qDebug() << "dist1 is " << dist;
    Epoch* epPtr = fromIndex(epIndx);
    if (!epPtr) {
        return;
    }

    bool settedChart = false;

    // for all sub ch
    auto numSubChs = epPtr->getChartsSizeByChannelId(channelId);
    // qDebug() << "numSubChs........................" << numSubChs;
    numSubChs = 1;
    for (int subChId = 0; subChId < numSubChs; ++subChId) {
        if (epPtr->chartAvail(channelId, subChId)) {
            Epoch::Echogram* chart = epPtr->chart(channelId, subChId);
            if (chart) {
                chart->bottomProcessing.setDistance(dist, Epoch::DistProcessing::DistanceSource::DistanceSourceProcessing);
                settedChart = true;
            }
        }
    }

    settedChart = true;
    if (settedChart) {
        setLastDepth(dist);

        if (firstChannelId_.channelId_ != channelId) { // only if first channel updated
            return;
        }

        int guardInterval = bottomTrackParam_.windowSize; // bottomTrack will proceed epIndx - guardInterval in next iteration
        int compIndx = epIndx > guardInterval ? epIndx - guardInterval : epIndx;
        // qDebug() << "guardInterval:"<<guardInterval << "  epIndx" <<epIndx;
        emit bottomTrackAdded(compIndx);
    }
}

void Dataset::onLastBottomTrackEpochChanged(const ChannelId& channelId, int val, const BottomTrackParam& btP, bool manual, bool redrawAll)
{
    // qDebug() << "Dataset::onLastBottomTrackEpochChanged.............";
    bottomTrackParam_     = btP;
    lastBottomTrackEpoch_ = val;

    // emit dataUpdate();
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

void Dataset::setActiveContactIndx(int64_t indx)
{
    activeContactIndx_ = indx;
    emit activeContactChanged();
    emit dataUpdate();
}

int64_t Dataset::getActiveContactIndx() const
{
    return activeContactIndx_;
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
