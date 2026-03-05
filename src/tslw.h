/*!
 ******************************************************************************
 * @file    wsl.h
 * @brief   无线传输专业sonar log 文件
 * @author  Oliver Miao
 * @date    2017/7/19
 * @version Ver1.00
 * @note    none.
 ******************************************************************************
*/
#ifndef  _WSL_H
#define  _WSL_H

/*-----------------------------[   Constant Define  ]--------------------------*/
#define _PIXEL_NUM_WSL_V10   240
#define _COORDINATE_SCALE    (10000000)   ///<精度到0.011m

/*-----------------------------[    Micro Define    ]--------------------------*/
/*-----------------------------[    Type Define     ]--------------------------*/
#pragma pack(push)
#pragma pack(1)


#include <QList>


typedef struct
{
    quint8 reg0;            ///<bit[1:0] hardware gain: 1x 2x 4x; bit[2]  gain mode: 1-- auto gain
    quint32 loRng;     ///<in unit cm
} ping_info_w;

typedef struct
{
    qint32 latitude;      ///<放大 x10^7
    qint32 longitude;     ///<放大 x10^7

    quint32  time;        ///<simplified GPS time

    quint16 heading;       ///<unit=0.1 degree
    quint16 HDOP;          ///<放大 x100
    quint8  satellites;
    quint8  speed;         ///<unit=0.1 m/s, max: 91km/h
} navi_info_w;          // 20Byte

typedef struct
{
    quint8  reg0;         ///<bit[7:4]指示油门状态: 0 / 1)(min) / 2(max);  bit[3:0] 指示自动驾驶状态 **/
    quint8  reg1;

    quint8 fishIdx;       ///<index 0~240 in img[]
    quint8 fishSize;      ///<0~255
    quint8 surfaceEnd;

    quint8 voltage;       ///<unit=0.1V, volt: 0~25.5V

    quint8 temperature;   ///<temperature in Fahrenheit

    quint16 depth;        ///<unit is cm
} aux_info_w;

/*! @brif 仅在解析和打包的时候使用, size = 7+240+20=267Byte */
typedef struct
{
    ping_info_w ping;
    navi_info_w boat;
    aux_info_w auxInfo;
} tsl_digitalInfo_w;

typedef struct
{
    quint8 a;
    quint8 b;
    quint8 c;
}pack_head_w;


typedef struct
{
    pack_head_w packhead;
    ping_info_w ping;
    navi_info_w boat;
    aux_info_w auxInfo;
    QList<quint8> rawDat;
    quint8 chkSum;              // xor calculate all data in byte
} tsl_w;


typedef struct
{
    quint32 loRng_m;       ///<in unit meter
    qint32 latitude;           ///<放大 x10^7
    qint32 longitude;          ///<放大 x10^7
    quint32 time;               ///<simplified GPS time
    float heading;            ///<unit=0.1 degree
    quint8 satellites;
    quint16 hdop;
    float speed;               ///<unit=0.1 m/s, max: 91km/h
    quint8 fishIdx;             ///<index 0~240 in img[]
    quint8 fishSize;            ///<0~255
    float voltage;             ///<unit=0.1V, volt: 0~25.5V
    float temperature_f;       ///<temperature in F
    float temperature_c;       ///<temperature in C
    float depth_m;            ///<unit is meter
}StructLatestTslwData;






#pragma pack(pop)

/*-----------------------------[ Local Variables    ]--------------------------*/
/*-----------------------------[  Gloable Variables ]--------------------------*/
/*-----------------------------[Function Declaration]--------------------------*/

#endif // end of _WSL_H

