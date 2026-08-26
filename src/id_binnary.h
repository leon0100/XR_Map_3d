#ifndef IDBINNARY_H
#define IDBINNARY_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QVector>
#include <QTimer>
#include "dataset_defs.h"
// #include "proto_binnary.h"

// using namespace Parsers;
using Segment = QPair<uint16_t, uint16_t>; // first - begin, second - end

typedef enum {
    BoardNone = -1,
    BoardEnhanced = 1,
    BoardBase = 3,
    BoardNBase = 4,
    BoardChirp = 5,
    BoardAssist = 6,
    BoardNEnhanced = 7,
    BoardSideEnhanced = 8,
    BoardRecorderMini = 9,
    BoardDVL = 10,
    BoardBasic2D = 12,
    BoardUSBL = 15,
    BoardUSBLBeacon = 16,
    BoardNanoSSS = 17,
    BoardPULSEred_2D = 128,
    BoardPULSEblue_DSS = 129
} BoardVersion;

// struct LastReadInfo {
//     LastReadInfo() : version(), checkSum(0), address(0), isReaded(true) {};
//     LastReadInfo(Version _version, uint16_t _checkSum, uint8_t _address, bool _isReaded) :
//         version(_version), checkSum(_checkSum), address(_address), isReaded(_isReaded) {};

//     Version  version;
//     uint16_t checkSum;
//     uint8_t  address;
//     bool     isReaded;
// };

using ListLinkedChannels = QList<QPair<ChannelId, uint8_t>>;

struct ChartParameters {
    ChartParameters()
        : boardVersion(BoardNone), loRng(0), upRng(0), depth(0), sspd(0),
        pingSize(0), sfEnd(0), btStart(0), draft(0), heading(0), speed(0), temperature(0),
        latitude(0.0), longitude(0.0)
    {};

    ChartParameters(BoardVersion _boardVersion, QList<Segment> _errList)
        : boardVersion(_boardVersion),  errList(_errList),
          loRng(0), upRng(0), depth(0), sspd(0), pingSize(0), sfEnd(0), btStart(0), draft(0),
          heading(0), speed(0), temperature(0), latitude(0.0), longitude(0.0)
    {};

    BoardVersion boardVersion;
    // Version version;
    QList<Segment> errList;

    float loRng;
    float upRng;
    float depth;
    float sspd;
    int pingSize;
    int sfEnd;
    int btStart;
    int draft;
    quint16  heading;
    quint16  speed;
    quint16  temperature;
    quint32  time;
    double   latitude;
    double   longitude;
};

// class IDBin : public QObject
// {
//     Q_OBJECT
// public:
//     explicit IDBin(QObject *parent = nullptr);
//     ~IDBin();

//     // Resp  parse(FrameParser &proto);

//     virtual ID id() = 0;
//     virtual bool isSettable() { return false; }
//     virtual bool isSettup() { return false; }
//     virtual bool isRequestable() { return true; }

//     // Type lastType() { return m_lastType; }
//     // Version lastVersion() { return m_lastVersion; }
//     // Resp lastResp() { return m_lastResp; }

//     virtual void simpleRequest(Version ver);
//     virtual void requestAll() { simpleRequest(v0); }
//     virtual void startColdStartTimer() {};

//     void setAddress(uint8_t addr) { m_address = addr; }
//     void setConsoleOut(bool is_console) { isConsoleOut = is_console; }

// signals:
//     void updateContent(Parsers::Type type, Parsers::Version ver, Parsers::Resp resp, uint8_t address);
//     void dataSend(QByteArray data);
//     void binFrameOut(Parsers::ProtoBinOut &proto_out);
//     void notifyDevDriver(bool state);

// protected:
//     const U4 m_key = 0xC96B5D4A;

//     Type m_lastType;
//     Version m_lastVersion;
//     Resp m_lastResp;
//     uint8_t _lastAddress = 0;
//     QList<Version> availableVer;
//     uint8_t m_address = 0;
//     bool isConsoleOut = false;

//     virtual Resp parsePayload(FrameParser &proto) = 0;
//     virtual void requestSpecific(ProtoBinOut &proto_out) { Q_UNUSED(proto_out) }

//     bool checkKeyConfirm(U4 key) { return (key == m_key); }
//     void appendKey(ProtoBinOut &proto_out);

//     void hashBinFrameOut(ProtoBinOut &proto);
//     void interExecColdStartTimer();

// private:
//     /*methods*/
//     bool checkResponse(FrameParser& proto);
//     void onExpiredColdStartTimer();
//     void onExpiredSetTimer();
//     /*data*/
//     static const uint8_t repeatingCount_ = 7;
//     static const int timerPeriodMsec_ = 1500;
//     QTimer setTimer_;
//     QTimer coldStartTimer_;
//     LastReadInfo hashLastInfo_;
//     uint8_t setTimerCount_;
//     uint8_t coldStartTimerCount_;
//     bool isColdStart_;
//     bool needToCheckSetResp_;
// };



// class IDBinTimestamp : public IDBin
// {
//     Q_OBJECT
// public:
//     explicit IDBinTimestamp() : IDBin() {
//     }

//     ID id() override { return ID_TIMESTAMP; }
//     Resp  parsePayload(FrameParser &proto) override;

//     uint32_t timestamp() { return m_timestamp; }
// protected:
//     uint32_t m_timestamp;
// };



// class IDBinDist : public IDBin
// {
//     Q_OBJECT
// public:
//     explicit IDBinDist() : IDBin() {
//     }

//     ID id() override { return ID_DIST; }
//     Resp  parsePayload(FrameParser &proto) override;

//     uint32_t dist_mm() { return m_dist_mm; }
// protected:
//     uint32_t m_dist_mm;
// };



struct RawData {
#pragma pack(push, 1)
    struct RawDataHeader {
        // struct  __attribute__((packed)) {
        //     uint16_t dataType : 5; //
        //     uint16_t dataSize : 6; // +1 bytes
        //     uint16_t dataTrigger : 2;
        //     uint16_t channelGroup : 3;
        // };
#pragma pack(push, 1)
        struct BitFieldHeader {
            uint16_t dataType     : 5;
            uint16_t dataSize     : 6;
            uint16_t dataTrigger  : 2;
            uint16_t channelGroup : 3;
        };
#pragma pack(pop)

        uint8_t  channelCount = 0;
        uint32_t globalOffset = 0;
        uint32_t localOffset = 0;
        float    sampleRate = 0;
    // } __attribute__((packed));
    };
    #pragma pack(pop)

    RawDataHeader header;
    QByteArray data;

    uint32_t samplesPerChannel() {
        // return data.size()/(header.dataSize + 1)/header.channelCount;
    }
};



#endif // IDBINNARY_H
