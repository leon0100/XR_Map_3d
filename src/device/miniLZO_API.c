/*!
******************************************************************************
* @file    miniLZO_tst.c
* @brief   none.
* @author  Oliver Miao
* @date    2023/12/29
* @version Ver1.00
* @note    none.
*
* 测试工程见: TSBUS_tester
******************************************************************************
*/
//#include "bsp_mcu.h"
#include "minilzo.h"
#include "type.h"
#include "tslcompress_c.h"
/*-----------------------------[   Constant Define  ]--------------------------*/
//#define TEST_BUF_BYTES_NUM   (1024 * 2)
#define WRKMEM_BUF_SIZE      (1 << (11 + 1))   /*4096 Byte(u8) */

/*-----------------------------[    Micro Define    ]--------------------------*/
//#define BUF_UNIT_NUM         (TEST_BUF_BYTES_NUM/4)
#define WRKMEM_UNIT_NUM      (WRKMEM_BUF_SIZE/4)
//#define DECOMPRESS_EXT_SIZE  (TEST_BUF_BYTES_NUM / 16 + 64 + 3)

/*-----------------------------[    Type Define     ]--------------------------*/
/*-----------------------------[ Local Variables    ]--------------------------*/
//static u32 WRK_MEM_BUF_x[WRKMEM_UNIT_NUM];    /* 压缩算法用buffer, 可以用echo_buf[] 复用*/
//u32 *pMiniLZO_wkBuf = WRK_MEM_BUF_x;

//extern u8 echo_buf[];
u8 echo_buf[4096];
u32 *pMiniLZO_wkBuf = (u32 *)echo_buf;

/*-----------------------------[  Gloable Variables ]--------------------------*/
//u32 usr_raw_datBuf[BUF_UNIT_NUM];
//u32 buf_2_compress[BUF_UNIT_NUM + (sizeof(usr_raw_datBuf) / 16 + 64 + 3) / 4];
//u32 buf_2_compress[BUF_UNIT_NUM + DECOMPRESS_EXT_SIZE / 4];
//u32 buf_4_decompress[BUF_UNIT_NUM];

/*-----------------------------[Function Declaration]--------------------------*/

/*!
*******************************************************************************
* @brief       u32 compress_tsl(void *src, u32 len, u8 *dst)
* @param [in]  None.
* @param [out] None.
* @retval      None.
* @date        2023/12/30
* @note        None.
*******************************************************************************/
u32 compress_tsl(void *inBuf, u32 len_byte, u8 *out)
{
    s32 res;
    u32 dwCompressLen;

    //压缩, compressed 1024 bytes into 519 bytes, time = 363 us
    res = lzo1x_1_compress((const lzo_bytep)inBuf, len_byte, (lzo_bytep)out, (lzo_uintp)&dwCompressLen, pMiniLZO_wkBuf);
    if (res != LZO_E_OK)
    {
        /* this should NEVER happen */
        printf("internal error - compression failed: %d\r\n", res);
        return 0;
    }
    return dwCompressLen;
}
/*!
*******************************************************************************
* @brief       u32 decompress_tsl()
* @param [in]  None.
* @param [out] None.
* @retval      None.
* @date        2023/12/30
* @note        None.
*******************************************************************************/
u32 decompress_tsl(void *src, u32 len_byte, void *dst)
{
    s32 res;
    u32 dwDecompressLen;

    //解压, decompressed 519 bytes back into 1024 bytes, time = 159 us
    res = lzo1x_decompress((const lzo_bytep)src, len_byte, (lzo_bytep)dst, (lzo_uintp)&dwDecompressLen, NULL);
    if (res != LZO_E_OK)
    {
        /* this should NEVER happen */
        printf("internal error - decompression failed: %d\r\n", res);
        return 0;
    }

    return dwDecompressLen;
}
/*!
 * @brief initialize the LZO library
 */
void miniLZO_init(void)
{
    if (lzo_init() != LZO_E_OK)
    {
        printf("internal error - lzo_init() failed !!!\n");
        printf("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics)\n");
    }
    //DBG_PRT("miniLZO work buffer size: %d Bytes\n", WRKMEM_BUF_SIZE);
    //DBG_PRT("!!WARNING: echo_buf[] is used when compress the tsl3 frame, raw data was destroied!\r\n");
}
/**->>[END OF FILE]********  (c)Copyright Toslon-Marine  ***********************/

