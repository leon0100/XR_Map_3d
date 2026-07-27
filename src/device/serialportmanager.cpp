#include "serialportmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>













SerialPortMger::SerialPortMger(QObject *parent) : QObject(parent)
{

}


void SerialPortMger::toggleConnection(QString port, int baudRate)
{
    // baudRate_ = baudRate;
    qDebug() << "port: " << port << "   baudRate: " << baudRate;
    // if(serialPort_->isOpen()) {
    //     serialPort_->close();
    //     m_speedTimer.stop();
    //     emit connectionChanged(false);
    // }
    // else {
    //     serialPort_->setPortName(port);
    //     serialPort_->setBaudRate(baudRate);

    //     if(serialPort_->open(QIODevice::ReadWrite)) {
    //         emit connectionChanged(true);
    //     } else {
    //         emit dataReceived(tr("Failed to open port: %1").arg(serialPort_->errorString()));
    //     }
    // }
}


SerialPortMger::~SerialPortMger()
{

}


/*-----------------------------------------------------------------------------*/
SerialPortManager::SerialPortManager(QObject *parent) : QObject(parent)
{
    serialPort_ = new QSerialPort(this);
    serialPort_->setDataBits(QSerialPort::Data8);
    serialPort_->setParity(QSerialPort::NoParity);
    serialPort_->setStopBits(QSerialPort::OneStop);
    serialPort_->setFlowControl(QSerialPort::NoFlowControl);

    connect(serialPort_, &QSerialPort::readyRead, this, &SerialPortManager::handleReadyRead);

    m_speedTimer.setInterval(1000);
    connect(&m_speedTimer, &QTimer::timeout, this, &SerialPortManager::updateSpeedStats);

    scanPorts();

    m_speedTimer.start();
}


double SerialPortManager::parseNMEACoordinate(const QString &coord, const QString &direction)
{
    if (coord.isEmpty()) return 0.0;

    int dotIndex = coord.indexOf('.');

    if (dotIndex < 2) return 0.0;

    QString degreesStr = coord.left(dotIndex - 2);

    QString minutesStr = coord.mid(dotIndex - 2);

    double degrees = degreesStr.toDouble();
    double minutes = minutesStr.toDouble();

    double result = degrees + minutes / 60.0;

    // 根据方向调整正负
    if (direction == "S" || direction == "W")  {
        result = -result;
    }

    return result;
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

    QByteArray data = nmeaSentence.mid(1, checksumIndex - 1);

    // 提取校验和
    QByteArray checksumStr = nmeaSentence.mid(checksumIndex + 1, 2);
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
    if(serialPort_->isOpen()) {
        serialPort_->close();
    }
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
        m_speedTimer.stop();
        emit connectionChanged(false);
    }
    else {
        serialPort_->setPortName(port);
        serialPort_->setBaudRate(baudRate);

        if(serialPort_->open(QIODevice::ReadWrite)) {
            emit connectionChanged(true);
        } else {
            emit dataReceived(tr("Failed to open port: %1").arg(serialPort_->errorString()));
        }
    }
}

void SerialPortManager::toggleConnection2(int baudRate)
{
     qDebug() << "toggleConnection2............." << baudRate;
}

void SerialPortManager::saveSerialPortData()
{
    if(serialPort_->isOpen()) {
        return;
    }

    if(saveFilePath_.isEmpty()){
        return;
    }
    if(!saveFilePath_.endsWith(".txt")) {
        return;
    }
    if(readAllBuffer_.isEmpty()) {
        return;
    }

    QFile file(saveFilePath_);
    if (file.exists()) {
        // 先清空原有内容
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return;
        }
        file.close();
    }
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        return;
    }


    QTextStream out(&file);
    out.setCodec("UTF-8");

    out << QString::fromUtf8(readAllBuffer_);
    file.close();
}


void SerialPortManager::openSerialPortFile()
{
    if (saveFilePath_.isEmpty()) {
        qDebug() << "File path is empty!";
        return;
    }

    QFile file(saveFilePath_);
    if (!file.exists()) {
        qDebug() << "File does not exist:" << saveFilePath_;
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for reading:" << file.errorString();
        return;
    }

    // 清空临时缓冲区
    QByteArray readAllBuffer;
    readAllBuffer.clear();

    // 读取文件内容到临时缓冲区
    QTextStream in(&file);
    in.setCodec("UTF-8");

    while (!in.atEnd()) {
        QString line = in.readLine();
        readAllBuffer.append(line.toUtf8());
        readAllBuffer.append("\r\n");
    }

    file.close();

    qDebug() << "Data loaded from file to temporary buffer. Size:" << readAllBuffer.size() << "bytes";


    //解析0183协议和测深仪协议
    int endIndex;
    BoatPoint typBoatPt;
    while ((endIndex = readAllBuffer.indexOf("\r\n")) != -1)
    {
        QByteArray sentenceBytes = readAllBuffer.left(endIndex);
        readAllBuffer.remove(0, endIndex + 2);

        QString sentence = QString::fromUtf8(sentenceBytes);
        qDebug() << "============ New Sentence ==============";
        QStringList lines = sentence.split('\n',Qt::SkipEmptyParts);
        for(QString lin : lines) {
            QString line = lin.trimmed();
            if(line.startsWith("$GPRMC")) {
                QStringList parts = line.split(',');

                double latitude = parseNMEACoordinate(parts[3], parts[4]);  // 解析纬度
                double longitude = parseNMEACoordinate(parts[5], parts[6]); // 解析经度

                typBoatPt.latitude = latitude;
                typBoatPt.longitude = longitude;

                // currValidGPS.latitude = latitude;
                // currValidGPS.longitude = longitude;
                hasGPSData_ = true;

                qDebug() << "GPRMC latitude  " << latitude << "  " << longitude;
            }
            else if(line.startsWith("$GPGGA")) {
                QStringList parts = line.split(',');

                double latitude = parseNMEACoordinate(parts[2], parts[3]);  // 解析纬度
                double longitude = parseNMEACoordinate(parts[4], parts[5]); // 解析经度

                typBoatPt.latitude = latitude;
                typBoatPt.longitude = longitude;
                // currValidGPS.latitude = latitude;
                // currValidGPS.longitude = longitude;
                hasGPSData_ = true;

                qDebug() << "GPGGA latitude  " << latitude << "  " << longitude;
            }
            else if(line.startsWith("$SDDBT")) {
                QStringList parts = line.split(',');

                bool ok;
                double depth = parts[3].toDouble(&ok);
                qDebug() << "depth is    " << depth;
                if(ok) {
                    if(hasGPSData_) {
                        // currValidGPS.depth = depth;
                        // gpsDepthDataVec_.append(currValidGPS);
                        typBoatPt.depth = depth;
                        // BoatWayPointList.append(typBoatPt);
                        // BoatWayPointList_0.append(typBoatPt);
                        hasGPSData_ = false;
                        // currValidGPS.latitude = 0.0;
                        // currValidGPS.longitude = 0.0;
                        // currValidGPS.depth = 0.0;
                    }
                }
            }

        }
    }

}


void SerialPortManager::writeData(const QString &data)
{
    if(serialPort_->isOpen()) {
        const QByteArray bytes = data.toUtf8();
        serialPort_->write(bytes);
    }
}

void SerialPortManager::handleReadyRead()
{
    if(!serialPort_->isOpen()) {
        return;
    }

    QByteArray data = serialPort_->readAll();
    int bytesRead = data.size();
    readAllBuffer_.append(data);

    m_receivedBytes += bytesRead;
    m_bytesSinceLastUpdate += bytesRead;
    m_receivedFrames++;

    // 转换为可显示的字符串
    QString receivedText = QString::fromUtf8(data);
    emit signal_realTimeTrackData(receivedText);

    qDebug() << "bytesRead:" << bytesRead << "  |  receivedText:"<<receivedText;

    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz] ");
    emit dataReceived(timestamp + receivedText);

    emit statsUpdated();
}

void SerialPortManager::updateSpeedStats()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 elapsed = now - m_lastUpdateTime;

    if(elapsed > 0) {
        m_receiveSpeed = (m_bytesSinceLastUpdate / 1024.0) / (elapsed / 1000.0);
        m_bytesSinceLastUpdate = 0;
        m_lastUpdateTime = now;
        emit statsUpdated();
    }
}

// 属性访问器实现
QStringList SerialPortManager::availablePorts() {
    return m_availablePorts;
}

int SerialPortManager::receivedBytes() {
    emit statsUpdated();
    return m_receivedBytes;
}

double SerialPortManager::receiveSpeed() {
    return m_receiveSpeed;
}

int SerialPortManager::receivedFrames() {
    return m_receivedFrames;
}

bool SerialPortManager::isConnected() {
    return serialPort_->isOpen();
}

QString SerialPortManager::saveFilePath()
{
    return saveFilePath_;
}

void SerialPortManager::setSaveFilePath(const QString &path)
{
    qDebug() << "path is " << path;
    saveFilePath_ = path;
}


