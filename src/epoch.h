#pragma once

#include <math.h>
#include <stdint.h>
#include <time.h>
#include <QPixmap>
#include <QRectF>
#include <QVector>

#include "dataset_defs.h"
#include "id_binnary.h"


class Epoch
{
public:
    struct Contact {
        bool isValid() const {
            return !info.isEmpty() &&
                   cursorX != -1 &&
                   cursorY != -1;
        }
        void clear() {
            info.clear();
            lat      = 0.0f;
            lon      = 0.0f;
            echogramDistance = 0.0f;
            depth    = 0.0f;
            nedX     = 0.0f;
            nedY     = 0.0f;
            cursorX  = -1;
            cursorY  = -1;
            rectEcho = QRectF();
        }        

        QString info;
        float   lat = 0.0f;
        float   lon = 0.0f;
        float   echogramDistance = 0.0f;
        float   depth = 0.0f;
        float   nedX = 0.0f;
        float   nedY = 0.0f;
        int     cursorX = -1;
        int     cursorY = -1;
        QRectF  rectEcho;
    };

    struct DistProcessing {
        enum class DistanceSource {
            DistanceSourceNone = 0,
            DistanceSourceProcessing,
            DistanceSourceLoad,
            DistanceSourceConstrainHand,
            DistanceSourceDirectHand,
        };

        float distance = NAN;
        float min = NAN;
        float max = NAN;
        DistanceSource source = DistanceSource::DistanceSourceNone;

        Position bottomPoint;

        bool isDist() const { return qIsFinite(distance); }
        void setDistance(float dist, DistanceSource src = DistanceSource::DistanceSourceNone) { distance = dist; source = src; }
        void clearDistance(DistanceSource src = DistanceSource::DistanceSourceNone) { distance = NAN; source = src; }
        void resetDistance() { distance = NAN; source = DistanceSource::DistanceSourceNone; }
        float getDistance() const { return distance; }

        void setMin(float val, DistanceSource src = DistanceSource::DistanceSourceNone) {
            min = val;
            if(max != NAN && val + 0.05 > max) {
                max = val + 0.05;
            }
            source = src;
        }
        void setMax(float val, DistanceSource src = DistanceSource::DistanceSourceNone) {
            max = val;
            if(min != NAN && val - 0.05 < min) {
                min = val - 0.05;
            }
            source = src;
        }

        float getMax() const { return max; }
        float getMin() const { return min; }
    };

    struct Echogram {
        QVector<uint8_t> amplitude;
        float resolution = 0; // m
        float offset = 0; // m
        int type = 0;

        QVector<uint8_t> compensated;

        // void updateCompesated() {
        //     int raw_size = amplitude.size();
        //     if(compensated.size() != raw_size) {
        //         compensated.resize(raw_size);
        //     }

        //     const uint8_t* src = amplitude.constData();
        //     uint8_t* procData = compensated.data();

        //     const float resol = resolution;

        //     float avrg = 255;
        //     for(int i = 0; i < raw_size; i ++) {
        //         float val = src[i];

        //         avrg += (val - avrg)*(0.05f + avrg*0.0006);
        //         val = (val - avrg*0.55f)*(0.85f +float(i*resol)*0.006f)*2.f;

        //         if(val < 0) { val = 0; }
        //         else if(val > 255) { val = 255; }

        //         procData[i] = val;
        //     }
        // }

        DistProcessing bottomProcessing;
        Position sensorPosition;
        RecordParameters recordParameters_;
        ChartParameters chartParameters_;

        float range() const {
            return amplitude.size() * resolution;
        }

        bool isValid() const {
            return !amplitude.empty();
        }
    };

    Epoch();

    bool isValid() const {
        if (flags.eventAvail) return true;
        if (!charts_.isEmpty()) return true;
        if (flags.posAvail) return true;
        if (flags.tempAvail) return true;
        if (flags.distAvail) return true;

        return false;
    }

    bool operator==(const Epoch& other) const {
        return _eventId == other._eventId &&
               _eventTimestamp_us == other._eventTimestamp_us &&
               _eventUnix == other._eventUnix;
    }

    bool operator!=(const Epoch& other) const {
        return !(*this == other);
    }

    void setEvent(int timestamp, int id, int unixt);
    void setChart(const ChannelId& channelId, const QVector<QVector<uint8_t>>& chartData, float resolution, float offset);
    void setChartBySubChannelId(const ChannelId& channelId, uint8_t subChannelId, const QVector<uint8_t>& chartData, float resolution, float offset);

    void setRecParameters(const ChannelId& channelId, const RecordParameters& recParams);
    void setChartParameters(const ChannelId& channelId, const ChartParameters& chartParams);
    void setDist(const ChannelId& channelId, int dist);
    void setPositionLLA(double lat, double lon, LLARef* ref = NULL, uint32_t unix_time = 0, int32_t nanosec = 0);
    void setPositionLLA(Position position);
    void setSonarPosition(Position val);
    void setPositionLLA(const LLA& lla);
    void setPositionNED(const North_East_Down& ned);
    void setExternalPosition(Position position);
    void setPositionRef(LLARef* ref);

    void setDepth(float depth);
    float getDepth();
    bool isDepthAvail() { return qIsFinite(depth_); }


    void setPositionDataType(DataType dataType);
    DataType getPositionDataType() const { return _positionGNSS.dataType; };

    void setSonarPositionDataType(DataType dataType);
    DataType getSonarPositionDataType() const { return sonarPosition_.dataType; };

    void setGnssVelocity(double h_speed, double course);

    void setTime(DateTime time);
    void setTime(int year, int month, int day, int hour, int min, int sec, int nanosec = 0);

    void setTemp(float temp_c);
    void setAtt(float yaw, float pitch, float roll, DataType dataType = DataType::kRaw);
    DataType getAttDataType() const { return _attitude.dataType; };

    void setEncoders(float enc1, float enc2, float enc3);
    bool isEncodersSeted() { return _encoder.isSeted();}
    float encoder1() { return _encoder.e1; }
    float encoder2() { return _encoder.e2; }
    float encoder3() { return _encoder.e3; }

    void setDistProcessing(const ChannelId& channelId, float dist);

    void clearDistProcessing(const ChannelId& channelId);
    void setMinDistProc(const ChannelId& channelId, float dist);
    void setMaxDistProc(const ChannelId& channelId, float dist);
    void setMinMaxDistProc(const ChannelId& channelId, int min, int max,  bool isSave = true);

    bool eventAvail() { return flags.eventAvail; }
    int  eventID() { return _eventId; }
    int  eventTimestamp() {return _eventTimestamp_us; }
    int  eventUnix() { return _eventUnix; }

    DateTime* time() { return &_time; }

    int chartSize(const ChannelId& channelId = CHANNEL_NONE, uint8_t subChannelId = 0);
    bool chartAvail();
    bool chartAvail(const ChannelId& channelId, uint8_t subChannelId = 0) const;
    Echogram* chart(const ChannelId& channelId = CHANNEL_NONE, uint8_t subChannelId = 0);
    Echogram chartCopy(const ChannelId &channelId = CHANNEL_NONE, uint8_t subChannelId = 0) const;

    QList<ChannelId> chartChannels();

    float getMaxRange(const ChannelId& channel = CHANNEL_NONE, const ChannelId& channel2 = CHANNEL_NONE);

    bool distAvail() const
    {
        return flags.distAvail;
    }

    double distProccesing(const ChannelId& channelId = CHANNEL_NONE);

    // float rangeFinder() const {
    //     if(rangefinders_.size() > 0) {
    //         return rangefinders_.first();
    //     }
    //     return NAN;
    // }

    float temperature() { return m_temp_c; }
    bool temperatureAvail() { return flags.tempAvail; }

    bool isAttAvail() { return _attitude.isAvail(); }
    float yaw() { return _attitude.yaw; }
    float pitch() { return _attitude.pitch; }
    float roll() { return _attitude.roll; }

    double lat() { return _positionGNSS.lla.latitude; }
    double lon() { return _positionGNSS.lla.longitude; }

    Position getPositionGNSS() { return _positionGNSS; }
    Position getExternalPosition() { return _positionExternal; }
    Position getSonarPosition() { return sonarPosition_; }

    uint32_t positionTimeUnix() { return _positionGNSS.time.sec; }
    uint32_t positionTimeNano() { return _positionGNSS.time.nanoSec; }
    DateTime* positionTime() {return &_positionGNSS.time; }

    void setGNSSSec(time_t sec);
    void setGNSSNanoSec(int nanoSec);

    double relPosN() { return _positionGNSS.ned.n; }
    double relPosE() { return _positionGNSS.ned.e; }
    double relPosD() { return _positionGNSS.ned.d; }

    bool isPosAvail() { return flags.posAvail; }

    double gnssHSpeed() { return _GnssData.hspeed; }

    void doBottomTrack2D(Echogram &chart, bool is_update_dist = false);
    void doBottomTrackSideScan(Echogram &chart, bool is_update_dist = false);
    bool chartTo(const ChannelId& channelId, uint8_t subChannelId, float start,
                 float end, int16_t* dst, int dstLen, int imageType, bool reverse = false);
    void getSonarFramePixel(const ChannelId& channelId, uint8_t subChannelId, QVector<uint8_t>& pixelVec);

    void setResolution      (const ChannelId& channelId, uint16_t resolution);
    void setChartCount      (const ChannelId& channelId, uint16_t chartCount);
    void setOffset          (const ChannelId& channelId, uint16_t offset);
    void setFrequency       (const ChannelId& channelId, uint16_t frequency);
    void setPulse           (const ChannelId& channelId, uint8_t pulse);
    void setBoost           (const ChannelId& channelId, uint8_t boost);
    void setSoundSpeed      (const ChannelId& channelId, uint32_t soundSpeed);
    uint16_t getResolution  (const ChannelId& channelId) const;
    uint16_t getChartCount  (const ChannelId& channelId) const;
    uint16_t getOffset      (const ChannelId& channelId) const;
    uint16_t getFrequency   (const ChannelId& channelId) const;
    uint8_t getPulse        (const ChannelId& channelId) const;
    uint8_t getBoost        (const ChannelId& channelId) const;
    uint32_t getSoundSpeed  (const ChannelId& channelId) const;
    ChartParameters getChartParameters(const ChannelId& channelId) const;
    Contact contact_;

    uint8_t getChartsSizeByChannelId(const ChannelId& channelId) const;

public:
    bool isRegionStart_ = false;

protected:
    QMap<ChannelId, QVector<Echogram>> charts_; // key - channelId, value - echograms for all addresses
    QMap<ChannelId, float> rangefinders_;

    int _eventTimestamp_us = 0;
    int _eventUnix = 0;
    int _eventId = 0;

    DateTime _time;

    struct {
        float yaw = NAN, pitch = NAN, roll = NAN;
        DataType dataType;
        bool isAvail() {
            return qIsFinite(yaw) && qIsFinite(pitch) && qIsFinite(roll);
        }
    } _attitude;

    Position _positionGNSS;
    Position _positionExternal;
    Position sonarPosition_;

    struct {
        double hspeed = NAN;
        double course = NAN;
    } _GnssData;

    float m_temp_c = NAN;

    struct {
        float e1 = NAN;
        float e2 = NAN;
        float e3 = NAN;
        bool isSeted() {
            return qIsFinite(e1) || qIsFinite(e2) || qIsFinite(e3);
        }
    } _encoder;

    struct {
        float velocityX = 0;
        float velocityY = 0;
        float velocityZ = 0;
        bool isAvai = false;
    } doppler;

    struct {
        bool encoderAvail = false;
        bool eventAvail = false;
        bool timestampAvail = false;
        bool distAvail = false;

        bool posAvail = false;
        bool sonarPosAvail = false;

        bool tempAvail = false;
        bool isDVLSolutionAvail = false;
    } flags;

    float depth_ = NAN;

};
