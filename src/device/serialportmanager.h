// d:/MyProject/XR_Map_3d/src/device/serialportmanager.h
#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QByteArray>

class SerialPortManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availablePorts READ availablePorts NOTIFY availablePortsChanged)
    Q_PROPERTY(bool        connected      READ connected      NOTIFY connectedChanged)
    Q_PROPERTY(QString     saveFilePath   READ saveFilePath   WRITE setSaveFilePath NOTIFY saveFilePathChanged)

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    QStringList availablePorts() const;
    bool        connected() const;
    QString     saveFilePath() const;
    void        setSaveFilePath(const QString &path);

    Q_INVOKABLE void scanPorts();
    Q_INVOKABLE void toggleConnection(const QString &portName, const QString &baudRate);
    Q_INVOKABLE void saveSerialPortData();
    Q_INVOKABLE void clearBuffer();

signals:
    void availablePortsChanged();
    void connectedChanged();
    void saveFilePathChanged();
    void dataReceived(const QString &data);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    void setConnected(bool connected);

    QSerialPort *m_serial = nullptr;
    QByteArray   m_rxBuffer;
    QString      m_saveFilePath;
    bool         m_connected = false;
};

#endif // SERIALPORTMANAGER_H
