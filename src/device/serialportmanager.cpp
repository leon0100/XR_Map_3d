#include "serialportmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QProcess>

#include "minilzo.h"
#include "console.h"

SerialPortManager::SerialPortManager(QObject *parent) : QObject(parent)
{
    serialPort_ = new QSerialPort(this);
    serialPort_->setDataBits(QSerialPort::Data8);
    serialPort_->setParity(QSerialPort::NoParity);
    serialPort_->setStopBits(QSerialPort::OneStop);
    serialPort_->setFlowControl(QSerialPort::NoFlowControl);

    connect(serialPort_, &QSerialPort::readyRead, this, &SerialPortManager::handleReadyRead);

    scanPorts();
}

char SerialPortManager::calculateChecksum(const QByteArray &data)
{
    char checksum = 0;
    for (char c : data) {
        checksum ^= c;
    }
    return checksum;
}

bool SerialPortManager::verifyChecksum(const QByteArray &nmeaSentence)
{
    int checksumIndex = nmeaSentence.indexOf('*');
    if (checksumIndex == -1) {
        return false;
    }

    QByteArray data = nmeaSentence.mid(1, checksumIndex-1);

    // 提取校验和
    QByteArray checksumStr = nmeaSentence.mid(checksumIndex+1, 2);
    bool ok;
    int receivedChecksum = checksumStr.toInt(&ok, 16);
    if (!ok) {
        return false;
    }

    char calculatedChecksum = calculateChecksum(data);
    return calculatedChecksum == receivedChecksum;
}

SerialPortManager::~SerialPortManager()
{
    clearRealData();
}

void SerialPortManager::scanPorts()
{
    m_availablePorts.clear();
    auto ports = QSerialPortInfo::availablePorts();
    for(auto &port : ports) {
        // qDebug() << "port.portName is " << port.portName();
        m_availablePorts.append(port.portName());
    }
}

void SerialPortManager::toggleConnection(QString port, int baudRate)
{
    baudRate_ = baudRate;
    qDebug() << "port: " << port << "   baudRate: " << baudRate;
    if(serialPort_->isOpen()) {
        serialPort_->close();
        serialPort_->disconnect();
        clearRealData();
        emit connectChanged(false);
    }
    else {
        serialPort_->setPortName(port);
        serialPort_->setBaudRate(baudRate);
        if(serialPort_->open(QIODevice::ReadWrite)) {
            emit connectChanged(true);
            batchChannelId_ = ChannelId(QUuid::createUuid(), 0);
        }
        else {
            emit dataReceived(tr("Failed to open port: %1").arg(serialPort_->errorString()));
        }
    }
}


void SerialPortManager::handleReadyRead()
{
    if(!serialPort_->isOpen()) {
        return;
    }

    QByteArray data = serialPort_->readAll();
    readAllBuffer_.append(data);

    parseTModemFrame(readAllBuffer_);
}

QStringList SerialPortManager::availablePorts()
{
    return m_availablePorts;
}

bool SerialPortManager::isConnected()
{
    // qDebug() << "serialPort_->isOpen........" << serialPort_->isOpen();
    return serialPort_->isOpen();
}

void SerialPortManager::clearRealData()
{
    tmodemSn_  = 0;
    m_tsl3Buffer.clear();
    nowIndex_  = 0;
    tslIndex_  = 0;
    depthHistory_.clear();
    minDepth_  = 0.0;
    maxDepth_  = 0.0;
    latitude_  = 000.000;
    longitude_ = 000.000;
    angle_     = 000.000;
    speed_     = 0.0;
    depth_     = 0.0;
}

// --------------------------------------- CRC Helpers -------------------------------------------
// CRC8 (poly 0x07, init 0x00) - bitwise implementation
uint8_t SerialPortManager::crc8_poly7(const uint8_t *data, int len)
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
uint16_t SerialPortManager::crc16_modbus(const uint8_t *data, int len)
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


QByteArray SerialPortManager::buildXrmapActivePayload(uint16_t map_ver,  const QString &map_name,
                                uint32_t map_size,  uint16_t all_map_CRC16, uint32_t all_map_CRC32,
                                uint16_t pkt_bytes,  uint16_t MAP_PKT_NUM,  uint32_t unix_sec)
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
QByteArray SerialPortManager::buildTModemFrame_xrmap(uint8_t dev_addr, uint8_t sn, bool needAck, uint8_t commandByte, const QByteArray &payload)
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

void SerialPortManager::parseTModemFrame(QByteArray& rawData)
{
    // qDebug() << "rawData.size().... " << rawData.size();
    QList<StructFrameTM> frames;

    const quint8 HEAD1       = 0xAA;
    const quint8 HEAD2       = 0xBB;
    const int    HEADER_LEN  = sizeof(TModemHeader); // 8字节
    const int    MAX_PAYLOAD = 4096;

    int pos = 0;
    int dataLen = rawData.size();
    bool frameLengNot = false;
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
            frameLengNot = true;
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

        frames.append(tbbpFrame);  // 9. 添加到帧列表

        pos += frameLen;  // 10. 移动到下一帧
    }

    if(!frameLengNot) {
        rawData.remove(0, pos);
        parseTsl3FromTModem();
    }

}

void SerialPortManager::parseTsl3FromTModem()
{
    QList<QByteArray> tslByteList;
    int byteCount = 0;
    int maxCount = m_tsl3Buffer.count();
    int tslIdx = sizeof(pack_head_t3)+sizeof(ping_info_t3)+sizeof(navi_info_t3)+sizeof(aux_info_t3);
    while(nowIndex_ < (maxCount-100))
    {
        if('#' == m_tsl3Buffer.at(nowIndex_)) {
            byteCount = tslIdx + U8_TO_16(m_tsl3Buffer.at(nowIndex_+22),m_tsl3Buffer.at(nowIndex_+23))+1;
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

    // qDebug() << "tslByteList.size()........" << tslByteList.size();
    for(auto tslDataTemp : tslByteList) {
        tsl_3 tslSingleStru;
        memcpy(&tslSingleStru, tslDataTemp, tslIdx);
        LLA lla;
        lla.latitude  = dm_to_dd(tslSingleStru.boat.latitude);
        lla.longitude = dm_to_dd(tslSingleStru.boat.longitude);
        lla.altitude  = tslSingleStru.auxInfo.depth * 0.01f;
        // qDebug() << "lla.latitude " << lla.latitude << "  " << lla.longitude << "  " << lla.altitude;

        QByteArray rawDat;
        for(int i = 0; i < tslSingleStru.ping.size; i++) {
            rawDat.append(tslDataTemp[tslIdx + i]);
        }
        for(int i = tslSingleStru.ping.size; i < PING_SIZE_MAX; i++) {
            rawDat.append('\0');
        }

        // ----------- 将声呐数据发送到 Dataset ------------
        int pingSize = tslSingleStru.ping.size;
        QVector<QVector<uint8_t>> dataVec;
        QVector<uint8_t> channelData;
        for(int i = 0; i < pingSize; i++) {
            channelData.append((uint8_t)tslDataTemp[tslIdx + i]);
        }
        for(int i = pingSize; i < PING_SIZE_MAX; i++) {
            channelData.append((uint8_t)'\0');
        }
        dataVec.append(channelData);

        float upRng = tslSingleStru.ping.upRng;
        float loRng = tslSingleStru.ping.loRng;
        float depth = tslSingleStru.auxInfo.depth;
        if(tslSingleStru.ping.frequency == snrFrq455) {
            if(depth > 10000) {
                depth = 0;
            }
            else if(depth > 30000) {
                depth = 0;
            }
        }

        ChartParameters chartParams;
        chartParams.depth       = depth;
        chartParams.pingSize    = pingSize;
        chartParams.upRng       = upRng;
        chartParams.loRng       = loRng;
        chartParams.temperature = tslSingleStru.auxInfo.temperature;
        chartParams.heading     = tslSingleStru.boat.heading;
        chartParams.speed       = tslSingleStru.boat.speed;
        chartParams.time        = tslSingleStru.boat.time;
        chartParams.longitude   = lla.longitude;
        chartParams.latitude    = lla.latitude;
        latitude_  = chartParams.latitude;;
        longitude_ = chartParams.longitude;
        angle_     = chartParams.heading ;
        speed_     = chartParams.speed ;
        depth_     = depth;
        emit dataPanelUpdate();

        emit chartComplete(batchChannelId_, chartParams, dataVec, readingDrawTrack_);

        emit positionComplete(lla.latitude, lla.longitude, lla.altitude, readingDrawTrack_);

        depthHistory_.append(static_cast<float>(lla.altitude));
        minDepth_ = std::min(minDepth_, lla.altitude);
        maxDepth_ = std::max(maxDepth_, lla.altitude);

        emit signal_drawRealtimeContour(depthHistory_, minDepth_, maxDepth_, readingDrawTrack_);
        break;
    }

}

double SerialPortManager::dm_to_dd(double ddmmmmmmm)
{
    double dm = (double)ddmmmmmmm / 100.0f;
    int dd = (int)dm;
    double mm = (dm-dd) / 0.6f;

    return ((double)dd+mm);
}


QByteArray SerialPortManager::decompressTsl3(const QByteArray &compressed)
{
    if (compressed.isEmpty()) {
        qWarning() << "[TslCompressor] Compressed data is empty.";
        return QByteArray();
    }

    int maxDecompressedSize = 4096;
    QByteArray result;
    result.resize(maxDecompressedSize);
    lzo_uint decompressedLen = result.size();
    int res = lzo1x_decompress_safe((const lzo_bytep)compressed.constData(), compressed.size(),
                                    (lzo_bytep)result.data(),&decompressedLen, nullptr);
    if (res != LZO_E_OK) {
        qDebug() << "Decompression failed with error code:" << res;
        return QByteArray();
    }

    result.resize(decompressedLen);
    return result;
}
