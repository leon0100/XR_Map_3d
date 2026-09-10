# Toslon Sonar Log File Protocol

- Ver 1.1 @2025/8.24
- v1.0 创建 @Date: 2025/8.16

----

## 1）类型

- *.tsl 保留

- *.tslx:  tsl3升级版格式，包含Sonar, GNSS, MEMS 等全面信息，适用于专业声纳，侧扫及声纳阵列，用于有线传输， 数据存储；帧CRC16校验

- *.tsly: 精简版声纳数据，仅包括声纳数据, 适合无线传输等低速通讯应用; 帧XOR校验

- *.tslz 保留

- *.tslv 保留

  ----

  > **曾经使用格式**
  >
  > - *.tsl3  测深仪使用， 被tslx替代； 不再用于新设计
  > - *.tslw TF740 XR500 TF520 使用， 被 tsly替代； 不再用于新设计
  > - *.tslw2: xr320 使用，极致压缩30字节版本， 专属定制化版本， 不再使用于其他产品



## 2）*.tslx(草稿版本)

```c
/*  TSLX Frame: 36 bytes + pixels[]********************************************/
#pragma pack(push, 1)
typedef struct
{
    uint8_t   hdr[2];       /* '%', 'X'*/
    uint16_t  length;       /* length of this frame, not include hdr[] &length*/

    uint32_t  depth;        // cm
    uint16_t  up_rng;       /* cm， pixels[0]像素的深度值. */
    uint16_t  lo_rng;       /* cm,  pixels[max]像素的深度值 */
    uint16_t  ping_size;    /* pixels[]像素数量 */

    uint32_t  Unix_sec;     /* Unix 秒 */
    uint32_t  ticks;        /* time ticks@ mS */
    uint16_t  sn;           /* 帧流水序号 */
    uint8_t   beam_ID;      /* 0~255: 波束代号 */
    uint8_t   reg[7];       /* reg[0].bit[1:0] 压缩标志 (0=raw,1=miniLZO, 2=LZ4); 未用的填充0x00*/

    uint16_t  HDR_CRC16;    /* CRC16 cal, from hdr[0] to reg[8] */
    uint16_t  PIX_CRC16;    /* CRC16 cal, from pixels[0] to pixels[max]*/
    uint8_t   pixels[];
} tslx_t;
#pragma pack(pop)

/* NAVI INFO frame: 64 bytes*/
#pragma pack(push, 1)
typedef struct
{
    uint8_t   hdr[2];        /* '%', 'G'*/
    uint16_t  lenght;        /* length of this frame, not include hdr[] &length*/

    double   longitude;     ///< dddmm.mmmmmmm-
    double   latitude;      ///< ddmm.mmmmmmm , for example: 3218.1234 is 32.30305666 degree-
    float    alt1;          // Altitude, Meters, above mean sea level, in meters(天线离海平面的高度)-
    float    HAE_WG84;      // 原来的: alt2; [HAE_WG84]Height of geoid (mean sea level) above WGS84-

    uint32_t time;          /* Unix 秒*/
    float    heading;       // x10, 0~3600, for example: 2734 means the heading is 273.4 degree-
    float    speed;         // x100, in knot-
    float    temperature;   // temperature in Fahrenheit-
    float    voltage;       // voltage-

    // MEMS SENSOR
    float    pitch;         /* 取值范围：-90° ～ +90°, 定义：船头向上为正，向下为负*/
    float    roll;          /* 取值范围：-90°～ +90°定义：船体右倾为正，左倾为负*/
  
    uint8_t  fix_sta;       // 0=(FIX NOT valid)，1=(GPS FIX)，2=(DGPS)，3=无效PPS，4=（RTK FIX），5=RTK FLOAT，6=正在估算.-
    uint8_t  reg[5];          // RESERVED-
    uint16_t NAVI_CRC16;
} navix_info_t;
#pragma pack(pop)

```



### reg[9]

| reg[] | 数值  |                   含义                   |      |
| :---: | :---: | :--------------------------------------: | ---- |
|   0   |  0~2  | pixels[]压缩标志: 0=raw,1=miniLZO, 2=LZ4 |      |
|   1   | 0~255 |                   NDF                    |      |
|   2   | 0~255 |                   NDF                    |      |
|   3   | 0~255 |                   NDF                    |      |
|   4   | 0~255 |                   NDF                    |      |
|   5   | 0~255 |                   NDF                    |      |
|  ...  | 0~255 |                   NDF                    |      |




## 3）*.tsly

- 数据类型定义，本结构体最大 14+pixels number(4096 max)

  ```c
  /* 文件-3: [tsly 帧] 格式定义: 14 bytes + pixels[] ***************************************************/
  #pragma pack(push, 1)
  typedef struct _tsly_t
  {
      uint8_t  hdr[2];     /* '%', 'Y'*/
      uint16_t length;     /* bit[10:0]: length; bit[15:11]: see below table*/
  
      uint16_t reg0;       /* 动态参数： temperature, voltage, depthTrust_ratio, */
      uint16_t depth;      /* 深度，单位 cm */
      uint16_t ping_size;  /* bit[10:0]: ping_size 0~2047;  bit[15:11]: see below table  */
      uint8_t  up_Rng;     /* meters： max 255meters */
      uint8_t  lo_Rng;     /* meters： max 255meters*/
      uint8_t  HDR_CRC8;   /* 由hdr[0]到 lo_Rng， CRC8 */
  
      uint8_t PIX_CRC8;    /* 仅对pixels[]计算CRC8*/
      uint8_t pixels[];    /* ping_size 0~2047*/
  } tsly_t;
  #pragma pack(pop)
  ```

### length 详解

- length 包括hdr[] length等全部字节，length= sizeof(tsly_t)的实际值= ping_size+14

| bit[15:14]                 | bit[13:11] |                      bit[10:0]                      |
| :------------------------- | :--------: | :-------------------------------------------------: |
| reg0含义: 0~3              |  RESRVED   |                  本帧长度: 0~2047                   |
| 0: reserved                |            | Length of *tsly* Frame, not include hdr[2] & length |
| 1: reg0 is temperature     |            |                                                     |
| 2: reg0 is voltage         |            |                                                     |
| 3: reg0 is dethTrust ratio |            |                                                     |
|                            |            |                                                     |

### reg0 详解

> - 依据 length.bit[15:14] 确定本数值的含义
> - 本数值适用于低速变化的传感器值.




### ping_size 详解
| bit[15:13]            | bit[12:11]         |          bit[10:0]           |
| :-------------------- | :----------------- | :--------------------------: |
| 频率/波束代号: 0~7    | 像素分辨率: 0~3    |       像素数量: 0~2047       |
| 0: (Higher)           | 0 :pixels is 8 bit | pixel number of the pixels[] |
| 1: (Lower）           | 1: pixel is 2bit   |                              |
| 2: (NDF)              | 2: pixel is 4bit   |                              |
| 3: (NDF)              | 3: pixel is 16 bit |                              |
| 4: (NDF)              |                    |                              |
| 5: ( sidescan-Center) |                    |                              |
| 6: (sidescan-Right)   |                    |                              |
| 7: (sidescan-Left)    |                    |                              |


- ping_size 是图像像素数量
- 像素数量可以动态调整: 48, 120, 240, 360, 480, 1000



### HDR_CRC8

- 包括 0xA5, 'Y' 标识符 至  lo_Rng（包含）
- 

### PIX_CRC8

- 仅对pixels[ping_size] 数组校验， pixels[] 为可变长度
- CRC8错误检出率： 随机错误（均匀分布） 检出率99.61% ； 突发8bit以上错误无法检测
- 考虑到TMODEM协议已经有CRC16, 此处用CRC8够用， 绝对优于XOR-CHK

----



## 4) 记录文件格式（文件头512字节）

| 区块序号 | 名称                 | 长度（Byte)        | 功能说明                                                    |
| :------: | -------------------- | ------------------ | ----------------------------------------------------------- |
|   4.1    | ASCII 明文描述字符串 | 48                 | ASCII 描述文件类型, 不解析                                  |
|   4.2    | 文件参数结构体       | 128                | 和文件版本相关信息，格式锁定。                              |
|   4.3    | 产品参数结构体       | 256                | 和具体产品及应用相关信息，格式和TSL_FIL_PARM_t.version 有关 |
|   4.4    | 预留                 | 80=512-48-48-256   | 预留                                                        |
|   4.5    | Sonar & GNSS帧数据   | 最高5000帧记录数据 | 约 5000*1K=5MB 数据                                         |
|   4.6    | 文件尾               | 24字节             | 非必须                                                      |

### 4.1.  ASCII 文件头 (48字节)

- 描述字符串 "TOLSON sonar log file\0"
- 

### 4.2） 文件参数Struct（128字节）

```c
/* 128 bytes: 文件头-1 [tslx /tsly自身参数]. 30+ reserved[]*/
#pragma pack(push, 1)
typedef struct
{
    char     file_typ_str[16];    /* 文件标识符（如`"tslx"`），用于快速识别文件类型。*/
    char     ver_str[16];         /* 文件格式版本（如`0x01`）， 本参数也决定了产品参数可能是不同定义*/
    uint16_t header_size;         /* 文件头总长度（如`512字节），便于后续扩展。*/
    uint8_t  frame_dat_format;    /* 数据帧格式ID: (0: RESERVED; 1: tsl3; 2: tslx; 3: tsly) */
    uint32_t frame_size_max;      /* 每帧数据大小MAX（本例为`1200`字节）, 安全解压缩时要用到*/
    uint32_t file_frame_max;      /* 总帧数MAX（本例为`5000`）*/
    uint32_t time_stamp_type;     /* 时间戳类型（如`0x00`=无，`0x01`=Unix时间戳，`0x02`=相对起始时间等）*/
    uint32_t creat_time;          /* Unix second, file creat time stamp */
    uint16_t CRC16;
    uint8_t  reserved[75];
} TSL_FIL_Parm_t;
#pragma pack(pop)

```



### 4.3) 产品参数Struct(256字节)

```c
/* 256byte: 文件头-2 [tslx /tsly设备参数部分].194+reserved3[] */
#pragma pack(push, 1)
typedef struct
{
    // -- 设备信息字段 (64字节) --
    char     device_name[32];       // 设备型号字符串 (如"Toslon XR530")
    char     firmware_ver_str[16];  // device 固件版本 (如"1.10a")
    uint32_t serial_number;         // 设备序列号
    uint8_t  gps_datum;             // GPS基准面 (0=WGS84)
    int8_t   tim_zone;              // 时区: -12~+12, 文件中时间都是 UTC-8 或unix秒-
    uint8_t  reserved1[10];         // 填充对齐

    // -- 数据配置字段 (128字节) --
    uint16_t sonar_frequency;       // 声呐频率 (kHz，如200)
    uint8_t  sonar_channels;        // 通道数 (如2=双频)
    uint8_t  depth_unit;            // 深度单位 (0=米，1=英尺)
    uint16_t depth_range_max;       // 最大量程 (米)
    uint8_t  reserved2[58];         // 预留扩展

    // -- 校验字段   --
    uint16_t header_crc16;           // 文件头的CRC16校验值
    uint8_t  reserved3[126];         // 保留字段
} TSL_DEV_Parm_t;
#pragma pack(pop)
```

### 4.4） 文件头预留

- 预留80字节未用

  

### 4.5) 声纳 & GNSS数据记录

- tslx_t, tsly_t : 声呐帧数据

- naiv_info_t : GNSS 导航帧数据

- 声呐帧数据更新率远高于GNSS

  

### 4.6) 文件结束标记(32字节, 非必须)

```c
// tslx tsly 文件结束标记.(32字节, 非必须)
#pragma pack(push, 1)
typedef struct {
    char     end_mark[6];    /* "<END>\0" */
    uint32_t total_frame_num;
    uint32_t end_time_stamp;  /* Unix time */
    char     reserved[14];
}TSL_FIL_end_t;
#pragma pack(pop)

```



## 5）tsly文件HEX示例

![image-20250824162322673](Toslon Sonar Log File protocol.assets/image-20250824162322673.png)
