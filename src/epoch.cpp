#include "epoch.h"

#include <QPainterPath>
#include "core.h"
extern Core core;


Epoch::Epoch()
{
    charts_.clear();
    flags.distAvail = false;
}

void Epoch::setEvent(int timestamp, int id, int unixt)
{
    _eventTimestamp_us = timestamp;
    _eventUnix = unixt;
    _eventId = id;
    _time = DateTime(unixt, timestamp*1000);
    flags.eventAvail = true;
}

void Epoch::setChart(const ChannelId& channelId, const QVector<QVector<uint8_t>>& data, float resolution, float offset)
{
    auto& echograms = charts_[channelId];

    for (int i = 0; i < data.size(); ++i) {
        if (i >= echograms.size()) {
            echograms.resize(i + 1);
        }

        auto& echogram = charts_[channelId][i];
        echogram.amplitude  = data[i];
        echogram.resolution = resolution;
        echogram.offset     = offset;
        echogram.type       = 1;
    }
}

void Epoch::setChartBySubChannelId(const ChannelId &channelId, uint8_t subChannelId, const QVector<uint8_t>& chartData, float resolution, float offset)
{
    if (!charts_.contains(channelId)) {
        return;
    }

    auto& allChartsByChannelId = charts_[channelId];
    if (subChannelId >= allChartsByChannelId.size()) {
        return;
    }

    auto& charts = allChartsByChannelId[subChannelId];
    charts.amplitude = chartData;
    charts.resolution = resolution;
    charts.offset = offset;
    charts.type = 1;
}

void Epoch::setRecParameters(const ChannelId& channelId, const RecordParameters& recParams)
{
    if (charts_.contains(channelId)) {
        auto& echograms =  charts_[channelId];

        for (auto& iEchogram : echograms) {
            iEchogram.recordParameters_ = recParams;
        }
    }
}

void Epoch::setChartParameters(const ChannelId& channelId, const ChartParameters& chartParams)
{
    if (charts_.contains(channelId)) {
        auto& echograms =  charts_[channelId];

        for (auto& iEchogram : echograms) {
            iEchogram.chartParameters_ = chartParams;
        }
    }
}

void Epoch::setDist(const ChannelId& channelId, int dist)
{
    rangefinders_[channelId] = dist * 0.001;
    flags.distAvail = true;
}

void Epoch::setRangefinder(const ChannelId& channelId, float distance)
{
    rangefinders_[channelId] = distance;
}

// void Epoch::setDopplerBeam(IDBinDVL::BeamSolution *beams, uint16_t cnt)
// {
//     for(uint16_t i = 0; i < cnt; i++) {
//         _dopplerBeams[i] = beams[i];
//     }
//     //    setDist(beams[0].distance*1000.0f);
//     _dopplerBeamCount = cnt;
// }

// void Epoch::setDVLSolution(IDBinDVL::DVLSolution dvlSolution)
// {
//     _dvlSolution = dvlSolution;
//     flags.isDVLSolutionAvail = true;
// }

void Epoch::setPositionLLA(double lat, double lon, LLARef* ref, uint32_t unix_time, int32_t nanosec)
{
    Q_UNUSED(ref);

    _positionGNSS.time = DateTime(unix_time, nanosec);
    _positionGNSS.lla.latitude = lat;
    _positionGNSS.lla.longitude = lon;

    flags.posAvail = true;
}

void Epoch::setPositionLLA(Position position)
{
    _positionGNSS = position;
    flags.posAvail = true;
}

void Epoch::setPositionLLA(const LLA &lla)
{
    _positionGNSS.lla = lla;
}

void Epoch::setSonarPosition(Position val)
{
    sonarPosition_ = val;
    flags.sonarPosAvail = true;
}

void Epoch::setPositionNED(const North_East_Down &ned)
{
    _positionGNSS.ned = ned;
}

void Epoch::setExternalPosition(Position position) {
    _positionExternal = position;
}

void Epoch::setPositionRef(LLARef* ref) {
    if(ref != NULL && ref->isInit) {
        _positionGNSS.LLA2NED(ref);
    }
}

void Epoch::setDepth(float depth)
{
    depth_ = depth;
}

float Epoch::getDepth()
{
    return depth_;
}

void Epoch::setPositionDataType(DataType dataType)
{
    _positionGNSS.dataType = dataType;
}

void Epoch::setSonarPositionDataType(DataType dataType)
{
    sonarPosition_.dataType = dataType;
}

void Epoch::setGnssVelocity(double h_speed, double course) {
    _GnssData.hspeed = h_speed;
    _GnssData.course = course;
}

void Epoch::setTime(DateTime time) {
    Q_UNUSED(time);
}

void Epoch::setTime(int year, int month, int day, int hour, int min, int sec, int nanosec) {
    Q_UNUSED(year);
    Q_UNUSED(month);
    Q_UNUSED(day);
    Q_UNUSED(hour);
    Q_UNUSED(min);
    Q_UNUSED(sec);
    Q_UNUSED(nanosec);
}


void Epoch::setTemp(float temp_c) {
    m_temp_c = temp_c;
    flags.tempAvail = true;
}

void Epoch::setEncoders(float enc1, float enc2, float enc3) {
    _encoder.e1 = enc1;
    _encoder.e2 = enc2;
    _encoder.e3 = enc3;
}

void Epoch::setDistProcessing(const ChannelId& channelId, float dist) {
    if (charts_.contains(channelId)) {
        auto& charts = charts_[channelId];
        for (auto& iEchogram : charts) {
            iEchogram.bottomProcessing.setDistance(dist, DistProcessing::DistanceSource::DistanceSourceDirectHand);
        }
    }
}

void Epoch::clearDistProcessing(const ChannelId& channelId) {
    if (charts_.contains(channelId)) {
        auto& charts = charts_[channelId];
        for (auto& iEchogram : charts) {
            iEchogram.bottomProcessing.clearDistance(DistProcessing::DistanceSource::DistanceSourceDirectHand);
        }
    }
}

void Epoch::setMinDistProc(const ChannelId& channelId, float dist) {
    if (charts_.contains(channelId)) {
        auto& charts = charts_[channelId];
        for (auto& iEchogram : charts) {
            iEchogram.bottomProcessing.setMin(dist, DistProcessing::DistanceSource::DistanceSourceConstrainHand);
        }
    }
}

void Epoch::setMaxDistProc(const ChannelId& channelId, float dist) {
    if (charts_.contains(channelId)) {
        auto& charts = charts_[channelId];
        for (auto& iEchogram : charts) {
            iEchogram.bottomProcessing.setMax(dist, DistProcessing::DistanceSource::DistanceSourceConstrainHand);
        }
    }
}

void Epoch::setMinMaxDistProc(const ChannelId& channelId, int min, int max,  bool isSave)
{
    if (charts_.contains(channelId)) {
        auto& charts = charts_[channelId];
        for (auto& iEchogram : charts) {
            float minsave = iEchogram.bottomProcessing.getMin();
            float maxsave = iEchogram.bottomProcessing.getMax();

            iEchogram.bottomProcessing.setMin(min);
            iEchogram.bottomProcessing.setMax(max);
            iEchogram.bottomProcessing.resetDistance();

            if (!isSave) {
                iEchogram.bottomProcessing.setMin(minsave);
                iEchogram.bottomProcessing.setMax(maxsave);
            }
        }
    }
}


int Epoch::chartSize(const ChannelId &channelId, uint8_t subChannelId)
{
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return -1;
    }

    const auto& v = it.value();
    return hasIndex(v, subChannelId) ? v.at(subChannelId).amplitude.size() : -1;
}

bool Epoch::chartAvail()
{
    return charts_.size() > 0;
}

bool Epoch::chartAvail(const ChannelId &channelId, uint8_t subChannelId) const
{
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return false;
    }

    const auto& echograms = it.value();
    return hasIndex(echograms, subChannelId) && !echograms.at(subChannelId).amplitude.isEmpty();
}

QList<ChannelId> Epoch::chartChannels()
{
    return charts_.keys();
}

float Epoch::getMaxRange(const ChannelId& channel, const ChannelId& channel2)
{
    float maxRange = NAN;

    if (channel == CHANNEL_NONE && channel2 == CHANNEL_NONE) {
        for (auto it = charts_.cbegin(), end = charts_.cend(); it != end; ++it) {
            const auto &echogramList = it.value();
            for (const auto& ech : echogramList) {
                float r = ech.range();
                if (qIsFinite(r) && (!qIsFinite(maxRange) || r > maxRange)) {
                    maxRange = r;
                    break;
                }
            }
        }
    }
    else {
        auto extractMaxFromChannel = [this](const ChannelId& ch) -> float {
            float result = NAN;
            if (charts_.contains(ch)) {
                const auto& chChartsCRef = charts_[ch];
                for (const auto& ech : chChartsCRef) {
                    float r = ech.range();
                    if (qIsFinite(r) && (!qIsFinite(result) || r > result)) {
                        result = r;
                        break;
                    }
                }
            }
            return result;
        };

        const float r1 = extractMaxFromChannel(channel);
        const float r2 = extractMaxFromChannel(channel2);

        if (qIsFinite(r1)) {
            maxRange = r1;
        }
        if (qIsFinite(r2) && (!qIsFinite(maxRange) || r2 > maxRange)) {
            maxRange = r2;
        }
    }

    if (!rangefinders_.isEmpty()) {
        float r3 = rangefinders_.first();
        if (qIsFinite(r3) && (!qIsFinite(maxRange) || r3 > maxRange)) {
            maxRange = r3;
        }
    }

    return maxRange;
}

double Epoch::distProccesing(const ChannelId& channelId)
{
    if (channelId == CHANNEL_NONE) {
        for (auto it = charts_.cbegin(); it != charts_.cend(); ++it) {
            for (const auto& iEchogram : it.value()) {
                double distance = iEchogram.bottomProcessing.getDistance();
                if (qIsFinite(distance)) {
                    return distance;
                }
            }
        }
    } else if (charts_.contains(channelId)) {
        const auto& chart = charts_[channelId];
        for (const auto& ech : chart) {
            double distance = ech.bottomProcessing.getDistance();
            if (qIsFinite(distance)) {
                return distance;
            }
        }
    }

    return NAN;
}

Epoch::Echogram *Epoch::chart(const ChannelId &channelId, uint8_t subChannelId)
{
    if (chartAvail(channelId, subChannelId)) {
        return &charts_[channelId][subChannelId];
    }

    return nullptr;
}

Epoch::Echogram Epoch::chartCopy(const ChannelId &channelId, uint8_t subChannelId) const
{
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return {};
    }

    const auto& v = it.value();
    return hasIndex(v, subChannelId) ? v.at(subChannelId) : Epoch::Echogram{};
}

void Epoch::setAtt(float yaw, float pitch, float roll, DataType dataType) {
    _attitude.yaw = yaw;
    _attitude.pitch = pitch;
    _attitude.roll = roll;

    _attitude.dataType = dataType;
}

void Epoch::setGNSSSec(time_t sec)
{
    _positionGNSS.time.sec = sec;
}

void Epoch::setGNSSNanoSec(int nanoSec)
{
    _positionGNSS.time.nanoSec = nanoSec;
}

void Epoch::doBottomTrack2D(Echogram &chart, bool is_update_dist) {
    Q_UNUSED(chart);
    Q_UNUSED(is_update_dist);
}

void Epoch::doBottomTrackSideScan(Echogram &chart, bool is_update_dist) {
    Q_UNUSED(chart);
    Q_UNUSED(is_update_dist);
}

bool Epoch::chartTo(const ChannelId& channelId, uint8_t subChannelId, float start, float end, int16_t* dst, int dstLen, int imageType, bool reverse)
{
    if (dst == nullptr) {
        return false;
    }

    ChannelId localChannelId = channelId;

    if (!charts_.contains(localChannelId)) {
        memset(dst, 0, dstLen * 2);
        return false;
    }

    if (charts_[localChannelId][subChannelId].resolution == 0) {
        memset(dst, 0, dstLen * 2);
        return false;
    }

    int rawSize = charts_[localChannelId][subChannelId].amplitude.size();
    qDebug() << "rawSize............." << rawSize;
    if (rawSize == 0) {
        memset(dst, 0, dstLen * 2);
        return false;
    }

    uint8_t* src = charts_[localChannelId][subChannelId].amplitude.data();

    if (imageType == 1) {
        if (charts_[localChannelId][subChannelId].compensated.size() == 0) {
            charts_[localChannelId][subChannelId].updateCompesated();
        }
        src = charts_[localChannelId][subChannelId].compensated.data();
    }

    if (rawSize == 0) {
        for (int iTo = 0; iTo < dstLen; iTo++) {
            dst[iTo] = 0;
        }
    }

    start -= charts_[localChannelId][subChannelId].offset;
    end   -= charts_[localChannelId][subChannelId].offset;
    qDebug() << "start...." << start << "  " << end;

    float rawRangeF    = charts_[localChannelId][subChannelId].range();
    float targetRangeF = static_cast<float>(end - start);
    float scaleFactor  = (static_cast<float>(rawSize) / static_cast<float>(dstLen)) * (targetRangeF / rawRangeF);
    int offset         = start / charts_[localChannelId][subChannelId].resolution;
    int srcStart = offset;
    int dir = reverse ? -1 : 1;
    int off = reverse ? (dstLen-1) : 0;
    if (scaleFactor >= 0.8f) {
        for (int iTo = 0; iTo < dstLen; iTo++) {
            int srcEnd = static_cast<float>(iTo + 1) * scaleFactor + offset;

            int32_t val = 0;
            if (srcStart >= 0 && srcStart < rawSize) {
                if (srcEnd > rawSize) {
                    srcEnd = rawSize;
                }

                val = src[srcStart];
                for (int i = srcStart; i < srcEnd; i++) {
                    val += src[i];
                }
                val /= 1 + (srcEnd - srcStart);
            }

            srcStart = srcEnd;
            dst[off + dir * iTo] = val;
        }
    }
    else {
        for (int iTo = 0; iTo < dstLen; iTo++) {
            float cellOffset = static_cast<float>(iTo) * scaleFactor + static_cast<float>(offset) + 0.5f;
            int srcStart = static_cast<int>(cellOffset);
            int srcEnd = srcStart + 1;

            int32_t val = 0;
            if (srcStart >= 0 && srcStart < rawSize) {
                if (srcEnd >= rawSize) {
                    srcEnd = rawSize - 1;
                }

                float coef = cellOffset - floorf(cellOffset);
                val = static_cast<float>(src[srcStart]) * (1 - coef) + static_cast<float>(src[srcEnd]) * coef;
            }

            dst[off + dir*iTo] = val;
        }
    }

    return true;
}

void Epoch::getSonarFramePixel(const ChannelId& channelId, uint8_t subChannelId, QVector<uint8_t>& pixelVec)
{
    // Echogram& chart = charts_[channelId][subChannelId];
    auto it = charts_.find(channelId);
    if(it == charts_.end() || subChannelId >= it.value().size()) {
        return;
    }
    Echogram& chart = it.value()[subChannelId];
    pixelVec = chart.amplitude;
}

uint8_t Epoch::getChartsSizeByChannelId(const ChannelId& channelId) const
{
    if (charts_.contains(channelId)) {
        return static_cast<uint8_t>(charts_[channelId].size());
    }
    return 0;
}

// write to all
void Epoch::setResolution(const ChannelId& channelId, uint16_t resolution)
{
    if (charts_.contains(channelId)) {
        auto& echograms = charts_[channelId];
        for (auto& iEchogram : echograms) {
            iEchogram.recordParameters_.resol = resolution;
        }
    }
}

void Epoch::setChartCount(const ChannelId& channelId, uint16_t chartCount)
{
    if (charts_.contains(channelId)) {
        auto& echograms = charts_[channelId];
        for (auto& iEchogram : echograms) {
            iEchogram.recordParameters_.count = chartCount;
        }
    }
}

void Epoch::setOffset(const ChannelId& channelId, uint16_t offset)
{
    if (charts_.contains(channelId)) {
        auto& echograms = charts_[channelId];
        for (auto& iEchogram : echograms) {
            iEchogram.recordParameters_.offset = offset;
        }
    }
}

void Epoch::setFrequency(const ChannelId& channelId, uint16_t frequency)
{
    if (charts_.contains(channelId)) {
        auto& echograms = charts_[channelId];
        for (auto& iEchogram : echograms) {
            iEchogram.recordParameters_.freq = frequency;
        }
    }
}

void Epoch::setPulse(const ChannelId& channelId, uint8_t pulse)
{
    if (charts_.contains(channelId)) {
        auto& echograms = charts_[channelId];
        for (auto& iEchogram : echograms) {
            iEchogram.recordParameters_.pulse = pulse;
        }
    }
}

void Epoch::setBoost(const ChannelId& channelId, uint8_t boost)
{
    if (charts_.contains(channelId)) {
        auto& echograms = charts_[channelId];
        for (auto& iEchogram : echograms) {
            iEchogram.recordParameters_.boost = boost;
        }
    }
}

void Epoch::setSoundSpeed(const ChannelId& channelId, uint32_t soundSpeed)
{
    if (charts_.contains(channelId)) {
        auto& echograms = charts_[channelId];
        for (auto& iEchogram : echograms) {
            iEchogram.recordParameters_.soundSpeed = soundSpeed;
        }
    }
}

// get from first
uint16_t Epoch::getResolution(const ChannelId& channelId) const
{
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return 0;
    }

    const auto& echograms = it.value();

    for (const auto& iEchogram : echograms) {
        return iEchogram.recordParameters_.resol;
    }

    return 0;
}

uint16_t Epoch::getChartCount(const ChannelId& channelId) const
{
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return 0;
    }

    const auto& echograms = it.value();

    for (const auto& iEchogram : echograms) {
        return iEchogram.recordParameters_.count;
    }

    return 0;
}

uint16_t Epoch::getOffset(const ChannelId& channelId) const
{    
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return 0;
    }

    const auto& echograms = it.value();

    for (const auto& iEchogram : echograms) {
        return iEchogram.recordParameters_.offset ;
    }

    return 0;
}

uint16_t Epoch::getFrequency(const ChannelId& channelId) const
{
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return 0;
    }

    const auto& echograms = it.value();

    for (const auto& iEchogram : echograms) {
        return iEchogram.recordParameters_.freq ;
    }

    return 0;
}

uint8_t Epoch::getPulse(const ChannelId& channelId) const
{
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return 0;
    }

    const auto& echograms = it.value();

    for (const auto& iEchogram : echograms) {
        return iEchogram.recordParameters_.pulse ;
    }

    return 0;
}

uint8_t Epoch::getBoost(const ChannelId& channelId) const
{
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return 0;
    }

    const auto& echograms = it.value();

    for (const auto& iEchogram : echograms) {
        return iEchogram.recordParameters_.boost ;
    }

    return 0;
}

uint32_t Epoch::getSoundSpeed(const ChannelId& channelId) const
{
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return 0;
    }

    const auto& echograms = it.value();

    for (const auto& iEchogram : echograms) {
        return iEchogram.recordParameters_.soundSpeed ;
    }

    return 0;
}

ChartParameters Epoch::getChartParameters(const ChannelId& channelId) const
{
    auto it = charts_.constFind(channelId);
    if (it == charts_.cend()) {
        return {};
    }

    const auto& echograms = it.value();

    for (const auto& iEchogram : echograms) {
        return iEchogram.chartParameters_;
    }

    return {};
}
