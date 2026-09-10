/*******************************************************************************
* File name:  tsl3.h
* Author   :  Oliver Miao
* Date     :  2017/3/26
* Version  :  Ver2.0
*
* ===========================================================================
*
* (c) TOSLON 2014-2024
* www.toslon.com
********************************************************************************/
#include <QList>

#ifndef _TSL3_H
#define _TSL3_H

#define _GPS_ENABLE
#define _MEMS_ENABLE  // Micro-ElectroMechanical System

#define TSL3_HDR_OFFSET 512

/*************************  DONNT CHANGE DEFINE BELOW   ************************/
#ifndef __LITTLE_ENDIAN__  // defined in compiler, Little Endian Mode
//#error "must be in __LITTLE_ENDIAN__ "
#endif

typedef enum
{
    ff_tsl = 0,
    ff_txt,
    ff_csv,
} fformat_e;

typedef enum
{
    logM_auto = 0,
    logM_manual,
    logM_rtkMd,
} logMode_e;

typedef enum
{
    dp_txRate_1Hz = 20, ///< ��ʾ: 1Hz
    dp_txRate_2Hz = 10,
    dp_txRate_3Hz = 6,
    dp_txRate_4Hz = 5,
    dp_txRate_5Hz = 4,
} DpTxRate_e;

typedef enum
{
    datP_DBT = 0,         /// ������ʾΪ: DBT
    datP_SDH_13D,     /// SDH_13D
    datP_ODOM,        /// ODOM
    datP_DESo,        /// DESo
} datP_TxFmt_e;

typedef struct
{
    quint8        diskRdy;
    quint8        gpsConnected;
    quint8        fileOpened;
    quint8        en;

    quint32       radioBrate;     ////< ���ݴ��ڲ�����/Data Port baudrate: 1200~115200

    //fformat_e format;
    quint8        format;
    quint8        zz1;
    quint8        zz2;
    quint8        zz3;

    quint32       gpsBrate;       ///< GPSD���ڲ����� / GPS Port baudrate: 1200~115200

    qint8        timeZone;       ///< �û�ʱ��/Time Zone: -12~+12
    //logMode_e logMode;        ///< ��¼��ģʽ/logger Mode: auto/manual/rtkFix(rtkMd)
    quint8 logMode;
    quint8 zz4;
    quint8 zz5;

    quint32       fileSize;       ///< �ļ���С/File Size: 1000~50000

    quint8        wrStep;
    quint8        dataPortEn;     ///< ���ݶ˿�/Data Port: 1=On; 0= Off.
    quint8        depthUnit;      ///< ���ȵ�λ/Depth Unit: 0= meter; 1= feet
    quint8        dataPrtPwEn;    ///< ���⹩��/ DataP Power: 1=On; 0= Off.

    //DpTxRate_e    dataP_TxRate;     ///<  ��������Ƶ��/Depth Update Rate:
    quint8  dataP_TxRate;
    //datP_TxFmt_e  tx_depth_DT_DBT;   ///< ����������ʽ/ Depth OutPut Format.
    quint8  tx_depth_DT_DBT;
    quint8        tx_GPS_gga;        ///< ת��GGA/ Forward GGA: 1= enable
    quint8        tx_GPS_rmc;        ///< ת��RMC/ Forward RMC: 1= enable

    quint16 chkSum;
} loggerCfg_t;


typedef enum
{
    md_manual = 0,
    md_auto = 1,
} mode_type_e;

typedef enum _SNR_GAIN_e
{
    snrGain_1x = 0,
    snrGain_2x,
    snrGain_4x,
    snrGain_8x,
} SNR_GAIN_e;

typedef enum _SNR_FRQ_e
{
    snrFrq455 = 0,
    snrFrq200,
    snrFrq115,
    snrFrq83,
    snrFrq50,

    //snrDualFrq=100,
} SNR_FRQ_e;



///@brief ���û�����, ϵͳ�����Զ����ĵĲ���.
typedef struct _typSntCtrl
{
    quint8  secPeak_filter;      ///< ���λز��˲���/Sec. Peak Filter: 1=ON ; 0=OFF
    quint8  reg0;
    quint8  trigger;           /* ������ֵ/Trigger:  0=auto; >1: manual. ֱ����ʾ��ֵ, ������ΪADC������ֵ  */
    //mode_type_e gain_mode;         /* ����ģʽ/Gain Mode: 1=auto **/
    quint8  gain_mode;

    //SNR_GAIN_e  gain;              /* ����/Gain: gain_mode=0ʱ, ��ʾ�ֶ�Ӳ������ֵ */
    quint8  gain;
    //mode_type_e rngMode;           /* �Զ����ȷ�Χ/Auto Depth Range: 1=auto, 0=manual */
    quint8  rngMode;
    quint8          pingSpd;
    quint8          dualFrqEn;         ///<˫Ƶʹ��/Dual Beam:  1=Dual Beam ; 0=Single Beam

    //SNR_FRQ_e   snrFrq;            ///<����Ƶ��/Sonar Freq.: ��λKHz
    quint8 snrFrq;
    quint8 zz6;
    quint16         upRng;             ///<��Χ����/ Up Range:  rngMode=1ʱ��ʾ��ֵ, ��λmeter; rngMode=0ʱ:--

    quint16         loRng;            ///<��Χ����/ Low Range:  rngMode=1ʱ��ʾ��ֵ, ��λmeter; rngMode=0ʱ:--
    //mode_type_e soundSpdMd;       ///<�Զ�����/auto sound speed: 0=manual; 1= auto
    quint8 soundSpdMd;
    quint8 zz7;


    float       depthOffset;      ///< ��ˮ����/ depth offset: ��λcm

    float       soundSpd;         ///<����/sound speed: if((soundSpdMd==0)&& (soundSpd>0)): ���õ�����.��λm/s; else --.-

    quint8  rngIdx;
    quint8  fishID;
    quint8  simulator;
    quint8  transudcerVaild;        /// ������״̬/Transducer: 0: �޺Ϸ�̽ͷ /Invalid; 1=�кϷ�̽ͷ/ Valid

    quint16 chkSum;

} typSnrCtrl;



#define SET_TSL_GAIN(x)     (0x07 & x)
#define SET_TSL_GAINMD(x)   ((0x01 & x)<<3)
#define SET_TSL_FREQ(x)     ((0x03 & x)<<4)

#pragma pack(push)
#pragma pack(1)

typedef struct
{
    quint8 reg0;             ///< bit[2:0] hardware gain: 1x 2x ~6x; bit[3]  gain mode: 1-- auto gain; bit[5:4] frq
    quint8 frequency;        ///< for Dual beam transducer, indicate which beam working, please reference snrBeam_e.
    quint8 channelNumber;    ///< channel of the reciver(sonar sensor),????????  [0...255]

    quint32 number;         // the serial number of ping,example: 0,1,2.... N-1, N, N+1
    quint32 timeOffset;     // start form 0, time get from system-ticks, each file start from 0

    quint32 upRng;          // in unit cm
    quint32 loRng;          // in unit cm
    quint16 size;           // vaild sonar ping size in *rawData buffer, <= rawDat[], ??????????????????? **/
} ping_info_t3;


typedef struct
{
    double latitude;   ///< ddmm.mmmmmmm , for example: 3218.1234 is 32.30305666 degree
    double longitude;  ///< dddmm.mmmmmmm

    quint32  time;         // simplified GPS time
    /* format of time
                            + 2000=year [31:26] 2000~2063, after year of 2063, file hearder would define the offset of year.
                            month       [25:22]
                            day         [21:17]
                            hour        [16:12]
                            minute      [11: 6]
                            second      [ 5: 0]
                            */
    quint16 heading;        // x10, 0~3600, for example: 2734 means the heading is 273.4 degree
    quint16 speed;          // x100, in knot
    float  alt1;            // Altitude, Meters, above mean sea level, in meters(?????????????)
    float  alt2;            // [2019-01-22]Height of geoid (mean sea level) above WGqint84(??????????????????????)
    quint8 fixQuality;      //0=??????(FIX NOT valid)??
    //1=????��(GPS FIX)??
    //2=?????��(DGPS)??
    //3=??��PPS??
    //4=???????��??RTK FIX????
    //5=RTK FLOAT??
    //6=???????.S
#ifdef _MEMS_ENABLE
    float  pitch;       //  real-time pitch angle of the boat //
    float  roll;        //  real-time roll angle of the boat //
#endif
} navi_info_t3;


typedef struct
{
    quint16 voltage;     // x10, volt
    qint16 temperature; // x10, temperature in Fahrenheit
    quint32 depth;       // unit is cm
} aux_info_t3;


typedef struct
{
    quint8 a;
    quint8 b;
    quint8 c;
}pack_head_t3;

typedef struct
{
    pack_head_t3 head;
    ping_info_t3 ping;
    navi_info_t3 boat;
    aux_info_t3 auxInfo;

    QList<quint8> rawDat;  // ??  is it better to be a variable length buffer or a fixed length buffer like rawData[2048]?
    quint8 chkSum;                // xor calculate all data in byte
} tsl_3;



#pragma pack(pop)

/*********************************    END OF tsl2 define   *********************/
#endif // end of _TSL3_H

