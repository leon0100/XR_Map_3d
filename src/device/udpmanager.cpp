#include "udpmanager.h"



UdpManager::UdpManager(QObject *parent) : QObject{ parent }
{
    m_heartbeatCnt = 0;
    m_remotePort   = 8535;
    m_remoteIp     = "192.168.4.1";

    m_udpSocket = new QUdpSocket(this);
    if(m_udpSocket == nullptr) {
        return;
    }
    m_udpSocket->bind(QHostAddress::AnyIPv4, 8535, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    connect(m_udpSocket, &QUdpSocket::readyRead, this, &UdpManager::onReadyRead);

    m_heartbeatTimer = new QTimer(this);

    connect(m_heartbeatTimer, &QTimer::timeout, this, &UdpManager::onHeartbeatTimeout);

    m_heartbeatTimer->start(8000);
}

UdpManager::~UdpManager()
{
    qDebug() << "UdpManager::~UdpManager()............";
    disConnectUdp();
}


void UdpManager::disConnectUdp()
{
    qDebug() << "UdpManager::disConnectUdp()..........";
    if (m_heartbeatTimer) {
        m_heartbeatTimer->stop();
        m_heartbeatTimer->deleteLater();
        m_heartbeatTimer = nullptr;
    }

    if (m_udpSocket) {
        disconnect(m_udpSocket, &QUdpSocket::readyRead, this, &UdpManager::onReadyRead);
        m_udpSocket->abort();
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
        qDebug() << "  - UDP socket已关闭并释放";
    }
}


void UdpManager::stopHeartbeat()
{
    m_heartbeatTimer->stop();
}

void UdpManager::onHeartbeatTimeout()
{
    tmodemSn_++;
    QByteArray payload = buildXrmapActivePayload(1, QStringLiteral("T"),
                            102400u, 0x55AA, 0x11223344u, 512, 200, 1732000000u);
    QByteArray frame = buildTModemFrame_xrmap(0, tmodemSn_, true, 0x3E, payload);

    m_udpSocket->writeDatagram(frame, QHostAddress(m_remoteIp), m_remotePort);

    // qDebug() << "[UDP Heartbeat]" << packet << "len=" << len;
}

void UdpManager::onReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(m_udpSocket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        m_udpSocket->readDatagram(data.data(), data.size(), &sender, &senderPort);
        parseTModemFrame(data);

        // qDebug() << "recv:" << data << "from" << sender.toString() << senderPort;
    }
}


// --------------------------------------- CRC Helpers -------------------------------------------
// CRC8 (poly 0x07, init 0x00) - bitwise implementation
uint8_t UdpManager::crc8_poly7(const uint8_t *data, int len)
{
    uint8_t crc = 0x00;
    for (int i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x80) crc = (uint8_t)((crc << 1) ^ 0x07);
            else crc <<= 1;
        }
    }
    return crc;
}

// CRC16-MODBUS (reflected implementation)  returns 16-bit CRC (standard Modbus)
uint16_t UdpManager::crc16_modbus(const uint8_t *data, int len)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001; // 0xA001 is reflected 0x8005
            else
                crc = (crc >> 1);
        }
    }
    return crc;
}


QByteArray UdpManager::buildXrmapActivePayload(uint16_t map_ver,  const QString &map_name,
                                               uint32_t map_size,  uint16_t all_map_CRC16,
    uint32_t all_map_CRC32,  uint16_t pkt_bytes,  uint16_t MAP_PKT_NUM,  uint32_t unix_sec)
{
    Xrmap_Active_t xr;
    xr.HDR[0] = 0x81; xr.HDR[1] = 0x7E; // SUB_CMD & ~SUB_CMD (示例)
    xr.SIZE = 0; // 占位, 后面回填

    // 填字段（均为 little-endian）
    xr.map_ver = qToLittleEndian<uint16_t>(map_ver);

    // map_name: 固定10字节，超长截断，不足补0
    QByteArray name = map_name.toUtf8();
    name = name.left(10);
    memset(xr.map_name, 0, sizeof(xr.map_name));
    memcpy(xr.map_name, name.constData(), name.size());

    xr.map_size = qToLittleEndian<uint32_t>(map_size);
    xr.all_map_CRC16 = qToLittleEndian<uint16_t>(all_map_CRC16);
    xr.all_map_CRC32 = qToLittleEndian<uint32_t>(all_map_CRC32);
    xr.pkt_bytes = qToLittleEndian<uint16_t>(pkt_bytes);
    xr.MAP_PKT_NUM = qToLittleEndian<uint16_t>(MAP_PKT_NUM);
    xr.unix_sec = qToLittleEndian<uint32_t>(unix_sec);
    xr.CRC16 = 0; // 占位，稍后计算

    // 暂把 xrmap struct 写入 QByteArray，计算 SIZE 与 CRC16
    QByteArray xrba(reinterpret_cast<const char*>(&xr), sizeof(xr));
    // SIZE 回填: 文档与你之前示例一致，SIZE = total_len - 3 (去除 HDR(2)+SIZE(1) ) .
    // 例如 xrba.size() = sizeof(xr) ; then SIZE = xrba.size() - 3
    int sizeVal = xrba.size() - 3;
    if (sizeVal < 0) sizeVal = 0;
    xrba[2] = static_cast<char>(sizeVal & 0xFF);

    // 重新计算 CRC16（对 struct 从头开始到 CRC16 字段之前的所有字节计算）
    // CRC 计算区间 = xrba[0 .. crc16_pos-1]
    int crc16_pos = sizeof(xr) - 2; // CRC16 field 的起始位置（最后两个字节）
    uint16_t xr_crc = crc16_modbus(reinterpret_cast<const uint8_t*>(xrba.constData()), crc16_pos);
    // 回填 CRC16 (little-endian)
    xrba[crc16_pos] = static_cast<char>(xr_crc & 0xFF);
    xrba[crc16_pos + 1] = static_cast<char>((xr_crc >> 8) & 0xFF);

    return xrba;
}


// -------------------- 构造完整 TMODEM 帧 --------------------
/*
 dev_addr: 目标设备地址 (0~15)
 sn: 流水号
 needAck: 是否置需 ACK 位
 commandByte: 直接把 0x3E 放入 header.command 字段（高4位命令, 低4位 subcmd/参数）
*/
QByteArray UdpManager::buildTModemFrame_xrmap(uint8_t dev_addr, uint8_t sn, bool needAck, uint8_t commandByte, const QByteArray &payload)
{
    StructFrameTM frame;
    // 填 header
    frame.header.sync1 = 0xAA;
    frame.header.sync2 = 0xBB;
    // route = [dev_addr(低4)] [version(2bit)在bit4..5] [needAck bit6] [ackMark bit7]
    quint8 route = (dev_addr & 0x0F);
    if (needAck) route |= 0x40;
    // version=0   // 默认
    frame.header.route = route;

    frame.header.command = commandByte; // 例如 0x3E
    frame.header.sn = sn;

    // length 字段是 u16，小端序，低12位为payload len
    quint16 lenField = static_cast<quint16>(payload.size() & 0x0FFF);
    frame.header.length = qToLittleEndian<uint16_t>(lenField); // 注意：解析端在读取后会 qFromLittleEndian
    frame.header.xorChk = 0; // 先置0，计算头部CRC8后再写入
    frame.payload = payload;

    // 计算头部 CRC8：对 header 前 sizeof(TModemHeader)-1 字节计算（即不含 xorChk）
    // 由于 header.length 存的是小端值，我们必须临时构造 contiguous bytes 来计算 CRC8
    QByteArray hdrBytes(reinterpret_cast<const char*>(&frame.header), sizeof(TModemHeader));
    // hdrBytes 最后一个字节是 xorChk (currently 0)
    uint8_t hdr_crc8 = crc8_poly7(reinterpret_cast<const uint8_t*>(hdrBytes.constData()), sizeof(TModemHeader) - 1);
    // 填回 header.xorChk
    frame.header.xorChk = hdr_crc8;

    // 重新生成最终 header bytes（包含 xorChk）
    // 计算整帧 CRC16（header + payload），不包含后面放的 check1/check2
    QByteArray allForCrc;
    allForCrc.append(reinterpret_cast<const char*>(&frame.header), sizeof(TModemHeader));
    allForCrc.append(frame.payload);

    uint16_t final_crc = crc16_modbus(reinterpret_cast<const uint8_t*>(allForCrc.constData()), allForCrc.size());
    frame.check1 = static_cast<uint8_t>(final_crc & 0xFF);
    frame.check2 = static_cast<uint8_t>((final_crc >> 8) & 0xFF);

    // 返回最终字节流
    return frame.toByteArray();
}



void UdpManager::parseTModemFrame(const QByteArray& rawData)
{
    QList<StructFrameTM> frames;

    const quint8 HEAD1 = 0xAA;
    const quint8 HEAD2 = 0xBB;
    const int HEADER_LEN = sizeof(TModemHeader); // 8 字节
    const int MAX_PAYLOAD = 4096;

    int pos = 0;
    int dataLen = rawData.size();
    int frameCount = 0;

    while (pos <= dataLen - HEADER_LEN)
    {
        // 1. 查找包头 0xAA 0xBB
        if (!(static_cast<quint8>(rawData.at(pos)) == HEAD1 && static_cast<quint8>(rawData.at(pos + 1)) == HEAD2)) {
            pos++;
            continue;
        }


        // 2. 读取并校验头部
        TModemHeader header{};
        memcpy(&header, rawData.constData() + pos, HEADER_LEN);

        if (!header.verifyXorChk()) {
            pos++;
            continue;
        }

        // 3. 小端序转换长度字段
        header.length = qFromLittleEndian(header.length);

        // 4. 长度检查
        quint16 payloadLen = header.getLength();
        if (payloadLen > MAX_PAYLOAD) {
            pos++;
            continue;
        }

        // 5. 检查整帧长度是否足够
        quint32 frameLen = HEADER_LEN + payloadLen + 2; // +2 是 check1/check2
        if (pos + frameLen > dataLen) {
            break; // 数据不足，等待更多数据
        }

        // 6. 读取整帧数据
        QByteArray packet = rawData.mid(pos, frameLen);

        // 7. 解析帧结构
        StructFrameTM tbbpFrame{};
        memcpy(&tbbpFrame.header, packet.constData(), HEADER_LEN);
        tbbpFrame.payload = packet.mid(HEADER_LEN, payloadLen);
        tbbpFrame.check1 = static_cast<quint8>(packet.at(packet.size() - 2));
        tbbpFrame.check2 = static_cast<quint8>(packet.at(packet.size() - 1));

        // 8. 校验整帧CRC
        if (!tbbpFrame.verifyChecks()) {
            pos++;
            continue;
        }

        qDebug() << "tbbpFrame.payload............." << tbbpFrame.payload;
        m_tsl3Buffer += tbbpFrame.payload;
        tsl_3 tsl3Struct;
        parsePayload_tsl3(m_tsl3Buffer, tsl3Struct);

        // 9. 添加到帧列表
        frames.append(tbbpFrame);
        frameCount++;

        // 10. 移动到下一帧
        pos += frameLen;
    }

    // parseTsl3FromTModem();
}





bool UdpManager::parsePayload_tsl3(QByteArray &payload, tsl_3 &tsl3Struct)
{
    QByteArray tslByteArray = payload;

    int nowIndex=0;
    /*-当前序号是#代表可能是帧头，要进一步进行数量检测-*/
    if('#' == tslByteArray.at(nowIndex))
    {
        /*-算一下按照当前#是帧头的情况下，读取到的像素的个数，看是不是在100~1000范围内-*/
        int byteCount = sizeof(pack_head_t3) +sizeof(ping_info_t3) +sizeof(navi_info_t3) +sizeof(aux_info_t3) +U8_TO_16(tslByteArray.at(nowIndex+22),tslByteArray.at(nowIndex+23)) +1;

        /*-检查一下当前序号再加上获取的像素个数是不是已经超过整体长度了，超过的话再次查找下一个#-*/
        if((nowIndex + byteCount) == tslByteArray.count())
        {
            /*-这里取出来不会超过最大长度了-*/
            QByteArray tslByteArray_xor = tslByteArray.mid(nowIndex,byteCount);

            quint8 chk=0;
            for(int i=3;i<tslByteArray_xor.count()-1;i++)
            {
                chk ^= tslByteArray_xor.at(i);
            }

            /*-如果连异或校验也通过了，那么可以放进list里了-*/
            if(chk == (quint8)tslByteArray_xor.at(tslByteArray_xor.count()-1))
            {
                QByteArray tsl3ByteArray = tslByteArray.mid(nowIndex,byteCount);
                //tsl_3 tsl3Struct;
                memcpy(&tsl3Struct, tsl3ByteArray, sizeof(pack_head_t3)+sizeof(ping_info_t3)+sizeof(navi_info_t3)+sizeof(aux_info_t3));

                // 获取 QByteArray 的一部分
                QByteArray part = tsl3ByteArray.mid(sizeof(pack_head_t3) + sizeof(ping_info_t3) + sizeof(navi_info_t3) + sizeof(aux_info_t3), tsl3Struct.ping.size);

                // 将 QByteArray 转换为 QList<quint8>
                QList<quint8> listPart;
                for (char byte : part) {
                    listPart.append(static_cast<quint8>(byte));
                }

                // 将转换后的 QList<quint8> 追加到 rawDat
                tsl3Struct.rawDat.append(listPart);
                qDebug() << "tsl3Struct........." << tsl3Struct.boat.latitude << "  " << tsl3Struct.boat.longitude;

                return true;
            }
        }
    }
    return false;
}

void UdpManager::parseTsl3FromTModem()
{
    // if(tmodemSn_ == 1) {
    //     tslByteArray.remove(0, 512);  /*- 把文件头512字节的文件信息去掉，只保留声呐数据 -*/
    // }

    /*-将去掉文件头的所有剩下的声呐数据，按照一帧一帧的模式放入临时容器-*/
    // QList<QByteArray> tslByteList;
    // int nowIndex  = 0;
    int byteCount = 0;
    int maxCount = m_tsl3Buffer.count();
    while(nowIndex < (maxCount-100))
    {
        if('#' == m_tsl3Buffer.at(nowIndex)) {
            qDebug() << "'#' == m_tsl3Buffer.at(nowIndex)..........";
            byteCount = sizeof(pack_head_t3)+sizeof(ping_info_t3)+sizeof(navi_info_t3)+sizeof(aux_info_t3)+
                        U8_TO_16(m_tsl3Buffer.at(nowIndex+22),m_tsl3Buffer.at(nowIndex+23))+1;
            qDebug() << "byteCount......." << byteCount;
            if((byteCount >= 100) && (byteCount <= 2048)) {
                /*- 检查一下当前序号再加上获取的像素个数是不是已经超过整体长度了，超过的话进入下个循环 -*/
                if((nowIndex + byteCount) > maxCount) {
                    nowIndex++;
                    continue;
                }
                else {
                    QByteArray tslByteArray_xor = m_tsl3Buffer.mid(nowIndex,byteCount);
                    quint8 chk = 0;
                    for(int i = 3;i < tslByteArray_xor.count()-1;i++) {
                        chk ^= tslByteArray_xor.at(i);
                    }

                    if(chk == (quint8)tslByteArray_xor.at(tslByteArray_xor.count()-1)) {
                        tslByteList.append(m_tsl3Buffer.mid(nowIndex,byteCount));
                        nowIndex += byteCount;
                    }
                    else {
                        nowIndex++;
                        continue;
                    }
                }
            }
            else {
                qDebug() << "!!(byteCount >= 100) && (byteCount <= 2048)..........." << nowIndex;
                nowIndex++;
                continue;
            }
        }
        else {
            // qDebug() << "'#' != m_tsl3Buffer.at(nowIndex)..........." << nowIndex;
            nowIndex++;
            continue;
        }
    }


    int tsl3Cnt = tslByteList.count();
    int idx = sizeof(pack_head_t3)+sizeof(ping_info_t3)+sizeof(navi_info_t3)+sizeof(aux_info_t3);

    QList<LLA> track;
    QVector<float> vec_CSV;
    double minZ = 0.0, maxZ = 0.0;

    qDebug() << "tslWCnt.size()........." << tsl3Cnt;
    const int MEDIAN_WINDOW = 13;          // 窗口大小（奇数）
    const float SPIKE_THRESHOLD = 10.0f;   // 跳变阈值（米），超过用中值替代
    QList<LLA> buffer;
    int progressInterval = qMax(1, tsl3Cnt / 100);

    for(; tslIndex_ < tsl3Cnt; tslIndex_++)
    {
        QByteArray tslDataTemp = tslByteList.at(tslIndex_);

        tsl_3 tslSingleStruct;
        memcpy(&tslSingleStruct, tslDataTemp, idx);
        LLA lla;
        lla.latitude  = dm_to_dd(tslSingleStruct.boat.latitude);
        lla.longitude = dm_to_dd(tslSingleStruct.boat.longitude);
        lla.altitude = tslSingleStruct.auxInfo.depth / 100.f;

        buffer.append(lla);
        if (buffer.size() < MEDIAN_WINDOW) {
            // 窗口还没满的时候：为了保证轨迹点数量一致，将前 (W/2) 个点直接推入 track
            if (buffer.size() <= MEDIAN_WINDOW / 2) {
                minZ = std::min(minZ, lla.altitude);
                maxZ = std::max(maxZ, lla.altitude);
                vec_CSV.append(lla.altitude);
                track.append(lla);
            }
        } else {
            // 窗口超过设定大小，移除最旧的点，让窗口滑动
            if (buffer.size() > MEDIAN_WINDOW) {
                buffer.removeFirst();
            }

            // 获取窗口正中间的点
            int midIndex = MEDIAN_WINDOW / 2;
            LLA targetLla = buffer[midIndex];

            // 提取当前窗口内的所有深度，用于求中值
            QVector<float> depths;
            for (const LLA& item : buffer) {
                depths.append(item.altitude);
            }
            // 排序找中位数
            std::sort(depths.begin(), depths.end());
            float medianDepth = depths[MEDIAN_WINDOW / 2];

            // 异常值判断：与中位数偏差是否超过阈值
            if (std::abs(targetLla.altitude - medianDepth) > SPIKE_THRESHOLD) {
                targetLla.altitude = medianDepth; // 使用中值替代
                buffer[midIndex].altitude = medianDepth;
            }

            // 将过滤后的中心点加入最终轨迹
            minZ = std::min(minZ, targetLla.altitude);
            maxZ = std::max(maxZ, targetLla.altitude);
            vec_CSV.append(targetLla.altitude);
            track.append(targetLla);
        }

        qDebug() << "lla.latitude " << lla.latitude << "  " << lla.longitude << "  " << lla.altitude;
        bool enableRender = (tslIndex_ + 1) == tsl3Cnt ? true : false;
        // emit positionComplete_file(lla.latitude, lla.longitude, lla.altitude, enableRender);


    }


}

double UdpManager::dm_to_dd(double ddmmmmmmm)
{
    double dm = (double)ddmmmmmmm / 100.0f;

    int dd = (int)dm;

    double mm = (dm-dd) / 0.6f;

    return ((double)dd+mm);
}
