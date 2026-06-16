#include "udpmanager.h"

#include <QProcess>
#include <QRegularExpression>


#include "minilzo.h"
#include "console.h"


UdpManager::UdpManager(QObject *parent) : QObject{ parent }
{
    // getCurrentWifiName();
}

UdpManager::~UdpManager()
{
    disConnectUdp();
}

QString UdpManager::getCurrentWifiName()
{
    QProcess process;
    process.start("netsh", QStringList() << "wlan" << "show" << "interfaces");
    process.waitForFinished();

    QString result = QString::fromLocal8Bit(process.readAllStandardOutput());
    QStringList lines = result.split('\n');
    for(QString line : lines) {
        line = line.trimmed();
        if(line.startsWith("SSID") && !line.startsWith("BSSID"))
        {
            int pos = line.indexOf(':');

            if(pos > 0)
            {
                return line.mid(pos + 1).trimmed();
            }
        }
    }

    return QString();
}

void UdpManager::disConnectUdp()
{
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
    }

    m_heartbeatCnt = 0;
}

void UdpManager::clearRealData()
{
    m_heartbeatCnt = 0;
    tmodemSn_ = 0;
    m_tsl3Buffer.clear();
    nowIndex_  = 0;
    tslIndex_ = 0;
    depthHistory_.clear();
    minDepth_ = 0.0;
    maxDepth_ = 0.0;
}

void UdpManager::onHeartbeatTimeout()
{
    tmodemSn_++;
    QByteArray payload = buildXrmapActivePayload(1, QStringLiteral("T"),
                         102400u, 0x55AA, 0x11223344u, 512, 200, 1732000000u);
    QByteArray frame = buildTModemFrame_xrmap(0, tmodemSn_, true, 0x3E, payload);

    m_udpSocket->writeDatagram(frame, QHostAddress(m_remoteIp), m_remotePort.toUShort());

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

QString UdpManager::remoteIp() const
{
    return m_remoteIp;
}
void UdpManager::setRemoteIp(const QString& ip)
{
    m_remoteIp = ip;
    emit remoteIpChanged();
}

QString UdpManager::remotePort() const
{
    return m_remotePort;
}
void UdpManager::setRemotePort(QString port)
{
    m_remotePort = port;
    emit remotePortChanged();
}

QString UdpManager::wifiName() const
{
    return m_wifi;
}


void UdpManager::openUdp(bool open)
{
    if(open) {
        if(m_remoteIp.isEmpty()) {
            GIF->dialogInfo(Dialog_OK, tr("Remote IP is Empty!"));
            emit signalCancelUdpOn(false);
            return;
        }

        if(m_remotePort.isEmpty()) {
            GIF->dialogInfo(Dialog_OK, tr("Port is Empty!"));
            emit signalCancelUdpOn(false);
            return;
        }

        m_heartbeatCnt = 0;
        m_udpSocket = new QUdpSocket(this);
        m_udpSocket->bind(QHostAddress::AnyIPv4, m_remotePort.toUShort(),
                          QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

        connect(m_udpSocket, &QUdpSocket::readyRead, this, &UdpManager::onReadyRead);

        m_heartbeatTimer = new QTimer(this);
        connect(m_heartbeatTimer, &QTimer::timeout, this, &UdpManager::onHeartbeatTimeout);
        m_heartbeatTimer->start(8000);
        emit signalCancelUdpOn(true);
    }
    else {
        disConnectUdp();
        emit signalCancelUdpOn(false);
    }


}

void UdpManager::setDataReading(bool isReading)
{
    readingDrawTrack_ = isReading;
    emit dataReadingChanged();
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

    xr.map_size      = qToLittleEndian<uint32_t>(map_size);
    xr.all_map_CRC16 = qToLittleEndian<uint16_t>(all_map_CRC16);
    xr.all_map_CRC32 = qToLittleEndian<uint32_t>(all_map_CRC32);
    xr.pkt_bytes     = qToLittleEndian<uint16_t>(pkt_bytes);
    xr.MAP_PKT_NUM   = qToLittleEndian<uint16_t>(MAP_PKT_NUM);
    xr.unix_sec      = qToLittleEndian<uint32_t>(unix_sec);
    xr.CRC16         = 0; // 占位，稍后计算

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
    // qDebug() << "rawData.size().... " << rawData.size();
    QList<StructFrameTM> frames;

    const quint8 HEAD1       = 0xAA;
    const quint8 HEAD2       = 0xBB;
    const int    HEADER_LEN  = sizeof(TModemHeader); // 8字节
    const int    MAX_PAYLOAD = 4096;

    int pos = 0;
    int dataLen = rawData.size();
    while (pos <= (dataLen - HEADER_LEN))
    {
        // 1. 查找包头 0xAA 0xBB
        if (!(static_cast<quint8>(rawData.at(pos)) == HEAD1 && static_cast<quint8>(rawData.at(pos + 1)) == HEAD2)) {
            pos++;
            continue;
        }

        // 2. 读取并校验头部
        TModemHeader header{};
        memcpy(&header, rawData.constData() + pos, HEADER_LEN);
        QByteArray hdr = rawData.left(HEADER_LEN);
        if (!header.verifyXorChk()) {
            pos++;
            continue;
        }

        // 3.小端序转换长度字段
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
        tbbpFrame.check1  = static_cast<quint8>(packet.at(packet.size() - 2));
        tbbpFrame.check2  = static_cast<quint8>(packet.at(packet.size() - 1));

        // 8. 校验整帧CRC
        if (!tbbpFrame.verifyChecks()) {
            pos++;
            continue;
        }

        // qDebug() << "tbbpFrame.payload............." << tbbpFrame.payload;
        m_tsl3Buffer += decompressTsl3(tbbpFrame.payload);

        // 9. 添加到帧列表
        frames.append(tbbpFrame);

        // 10. 移动到下一帧
        pos += frameLen;
    }

    parseTsl3FromTModem();

}

void UdpManager::parseTsl3FromTModem()
{
    QList<QByteArray> tslByteList;
    int byteCount = 0;
    int maxCount = m_tsl3Buffer.count();
    while(nowIndex_ < (maxCount-100))
    {
        if('#' == m_tsl3Buffer.at(nowIndex_)) {
            byteCount = sizeof(pack_head_t3)+sizeof(ping_info_t3)+sizeof(navi_info_t3)+sizeof(aux_info_t3)+
                        U8_TO_16(m_tsl3Buffer.at(nowIndex_+22),m_tsl3Buffer.at(nowIndex_+23))+1;
            if((byteCount >= 100) && (byteCount <= 2048)) {
                if((nowIndex_ + byteCount) > maxCount) {
                    nowIndex_++;
                    continue;
                }
                else {
                    QByteArray tslByteArrayXor = m_tsl3Buffer.mid(nowIndex_,byteCount);
                    quint8 chk = 0;
                    for(int i = 3;i < (tslByteArrayXor.count()-1);i++) {
                        chk ^= tslByteArrayXor.at(i);
                    }

                    if(chk == (quint8)tslByteArrayXor.at(tslByteArrayXor.count()-1)) {
                        tslByteList.append(m_tsl3Buffer.mid(nowIndex_,byteCount));
                        nowIndex_ += byteCount;
                    }
                    else {
                        nowIndex_++;
                        continue;
                    }
                }
            }
            else {
                nowIndex_++;
                continue;
            }
        }
        else {
            nowIndex_++;
            continue;
        }
    }

    int idx = sizeof(pack_head_t3)+sizeof(ping_info_t3)+sizeof(navi_info_t3)+sizeof(aux_info_t3);
    for(auto tslData : tslByteList) {
        QByteArray tslDataTemp = tslByteList.at(tslIndex_);

        tsl_3 tslSingleStruct;
        memcpy(&tslSingleStruct, tslData, idx);
        LLA lla;
        lla.latitude  = dm_to_dd(tslSingleStruct.boat.latitude);
        lla.longitude = dm_to_dd(tslSingleStruct.boat.longitude);
        lla.altitude  = tslSingleStruct.auxInfo.depth * 0.01f;
        // qDebug() << "lla.latitude " << lla.latitude << "  " << lla.longitude << "  " << lla.altitude;

        emit positionComplete(lla.latitude, lla.longitude, lla.altitude, readingDrawTrack_);

        depthHistory_.append(static_cast<float>(lla.altitude));
        minDepth_ = std::min(minDepth_, lla.altitude);
        maxDepth_ = std::max(maxDepth_, lla.altitude);

        emit signal_drawRealtimeContour(depthHistory_, minDepth_, maxDepth_, readingDrawTrack_);
    }

}

double UdpManager::dm_to_dd(double ddmmmmmmm)
{
    double dm = (double)ddmmmmmmm / 100.0f;
    int dd = (int)dm;
    double mm = (dm-dd) / 0.6f;

    return ((double)dd+mm);
}



QByteArray UdpManager::decompressTsl3(const QByteArray &compressed)
{
    if (compressed.isEmpty())
    {
        qWarning() << "[TslCompressor] Compressed data is empty.";
        return QByteArray();
    }

    int maxDecompressedSize = 4096;
    QByteArray result;
    result.resize(maxDecompressedSize);

    lzo_uint decompressedLen = result.size();
    int res = lzo1x_decompress_safe((const lzo_bytep)compressed.constData(), compressed.size(),
        (lzo_bytep)result.data(),&decompressedLen, nullptr);

    if (res != LZO_E_OK)
    {
        qDebug() << "Decompression failed with error code:" << res;
        return QByteArray();
    }

    result.resize(decompressedLen);
    return result;
}

