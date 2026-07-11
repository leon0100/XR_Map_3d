#include "serialportmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>


SerialPortManager::SerialPortManager(QObject *parent) : QObject(parent)
{
    m_serial = new QSerialPort(this);
    connect(m_serial, &QSerialPort::readyRead,    this, &SerialPortManager::onReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred,this, &SerialPortManager::onErrorOccurred);
    scanPorts();
}

SerialPortManager::~SerialPortManager()
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }
}

QStringList SerialPortManager::availablePorts() const
{
    QStringList names;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        names << info.portName();
    }
    if (names.isEmpty()) {
        names << tr("No Devices Found");
    }
    return names;
}

bool SerialPortManager::connected() const { return m_connected; }

QString SerialPortManager::saveFilePath() const { return m_saveFilePath; }

void SerialPortManager::setSaveFilePath(const QString &path)
{
    if (m_saveFilePath != path) {
        m_saveFilePath = path;
        emit saveFilePathChanged();
    }
}

void SerialPortManager::scanPorts()
{
    emit availablePortsChanged();
}

void SerialPortManager::toggleConnection(const QString &portName, const QString &baudRate)
{
    if (m_connected) {
        if (m_serial->isOpen()) {
            m_serial->close();
        }
        setConnected(false);
        return;
    }

    if (portName.isEmpty() || portName == tr("No Devices Found")) {
        return;
    }

    m_serial->setPortName(portName);
    bool ok = false;
    const qint32 baud = baudRate.toInt(&ok);
    if (!ok) {
        return;
    }
    m_serial->setBaudRate(baud);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial->open(QIODevice::ReadWrite)) {
        m_rxBuffer.clear();
        setConnected(true);
    } else {
        qWarning() << "SerialPort open failed:" << m_serial->errorString();
        setConnected(false);
    }
}

void SerialPortManager::onReadyRead()
{
    const QByteArray data = m_serial->readAll();
    m_rxBuffer.append(data);
    emit dataReceived(QString::fromUtf8(data));
}

void SerialPortManager::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }
    if (error == QSerialPort::ResourceError) {
        if (m_serial->isOpen()) {
            m_serial->close();
        }
        setConnected(false);
    }
    qWarning() << "SerialPort error:" << error << m_serial->errorString();
}

void SerialPortManager::saveSerialPortData()
{
    if (m_saveFilePath.isEmpty() || m_rxBuffer.isEmpty()) {
        return;
    }
    QFile file(m_saveFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(m_rxBuffer);
        file.close();
        qDebug() << "SerialPort data saved to:" << m_saveFilePath;
    }
}

void SerialPortManager::clearBuffer()
{
    m_rxBuffer.clear();
}

void SerialPortManager::setConnected(bool connected)
{
    if (m_connected != connected) {
        m_connected = connected;
        emit connectedChanged();
    }
}
