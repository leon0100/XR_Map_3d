#ifndef IDBINNARY_H
#define IDBINNARY_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QVector>
#include <QTimer>
#include "dataset_defs.h"


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


struct RawData {
#pragma pack(push, 1)
    struct RawDataHeader {
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
    };
    #pragma pack(pop)

    RawDataHeader header;
    QByteArray data;
};


#endif // IDBINNARY_H
