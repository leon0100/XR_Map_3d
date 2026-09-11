#ifndef TSLY_H
#define TSLY_H

#include <cstdint>


/* ping_size bit[12:11]: pixel resolution */
// enum TslyPixelRes
// {
//     Pixel8Bit  = 0,
//     Pixel2Bit  = 1,
//     Pixel4Bit  = 2,
//     Pixel16Bit = 3
// };


// /* length bit[15:14]: meaning of reg0 dynamic parameter */
// enum TslyReg0Type
// {
//     Reg0Temperature = 0,
//     Reg0Voltage     = 1,
//     Reg0DepthTrust  = 2
// };


/*
 * NAVI INFO frame
 * Size: 64 bytes
 */
#pragma pack(push, 1)

typedef struct
{
    uint8_t   hdr[2];        /* '%', 'G' */
    uint16_t  length;        /* length of this frame, not include hdr[] & length */

    double    longitude;     /* dddmm.mmmmmmm */
    double    latitude;      /* ddmm.mmmmmmm */

    float     alt1;          /* Altitude, meters, above mean sea level */
    float     HAE_WG84;      /* Height of geoid above WGS84 */

    uint32_t  time;          /* Unix seconds */

    float     heading;       /* x10, 0~3600 */
    float     speed;         /* x100, in knot */
    float     temperature;   /* temperature in Fahrenheit */
    float     voltage;       /* voltage */

    /* MEMS SENSOR */
    float     pitch;         /* -90 ~ +90 degrees */
    float     roll;          /* -90 ~ +90 degrees */

    uint8_t   fix_sta;
    uint8_t   reg[5];        /* RESERVED */

    uint16_t  NAVI_CRC16;

} navix_info_t;

#pragma pack(pop)



/*
 * TSLY frame
 * Header: 14 bytes
 * Followed by pixels[]
 */
#pragma pack(push, 1)

typedef struct _tsly_t
{
    uint8_t   hdr[2];        /* '%', 'Y' */

    uint16_t  length;        /*
                              * bit[10:0]: length
                              * bit[15:11]: reserved / type information
                              */

    uint16_t  reg0;          /*
                              * Dynamic parameter:
                              * temperature
                              * voltage
                              * depth trust ratio
                              */

    uint16_t  depth;         /* depth, unit: cm */

    uint16_t  ping_size;     /*
                              * bit[10:0]: ping_size, 0~2047
                              * bit[15:11]: pixel resolution
                              */

    uint8_t   up_Rng;        /* upper range, meters, max 255 */
    uint8_t   lo_Rng;        /* lower range, meters, max 255 */

    uint8_t   HDR_CRC8;      /*
                              * CRC8 calculated from hdr[0]
                              * through lo_Rng
                              */

    uint8_t   PIX_CRC8;      /* CRC8 calculated only for pixels[] */

    uint8_t   pixels[];      /* pixel data, size determined by ping_size */

} tsly_t;

#pragma pack(pop)


#endif /* TSLY_H */
