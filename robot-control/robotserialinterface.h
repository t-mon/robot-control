#ifndef ROBOTSERIALINTERFACE_H
#define ROBOTSERIALINTERFACE_H

#include <QObject>
#include <QQmlEngine>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QLoggingCategory>

#include "robotcontrollerpacket.h"

Q_DECLARE_LOGGING_CATEGORY(dcRobotSerialInterface)
Q_DECLARE_LOGGING_CATEGORY(dcRobotSerialInterfaceTraffic)

class RobotSerialInterface : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged FINAL)

public:
    enum ProtocolByte {
        ProtocolByteEnd = 0xC0,
        ProtocolByteEsc = 0xDB,
        ProtocolByteTransposedEnd = 0xDC,
        ProtocolByteTransposedEsc = 0xDD
    };
    Q_ENUM(ProtocolByte)

    explicit RobotSerialInterface(QObject *parent = nullptr);

    QSerialPortInfo serialPortInfo() const;
    void setSerialPortInfo(const QSerialPortInfo &serialPortInfo);

    bool enabled() const;
    bool available() const;

    void sendPacket(const RobotControllerPacket &packet);

    static inline QString byteToHexString(quint8 byte) {
        return QString("0x%1").arg(byte, 2, 16, QLatin1Char('0'));
    }

    static inline QString uint16ToHexString(quint16 value) {
        return QString("0x%1").arg(value, 4, 16, QLatin1Char('0'));
    }

    static quint16 calculateCrc(const QByteArray &data);

public slots:
    void enable();
    void disable();
    void setEnabled(bool enabled);

signals:
    void availableChanged(bool available);
    void enabledChanged(bool enabled);

    void dataReceived(const QByteArray &data);
    void packetReceived(const RobotControllerPacket &packet);

private slots:
    void onReadyRead();

private:
    QSerialPort *m_serialPort = nullptr;
    QSerialPortInfo m_serialPortInfo;

    bool m_available = false;
    bool m_enabled = false;

    QByteArray m_dataBuffer;
    bool m_escape = false;

    void streamByte(quint8 byte, bool specialCharacter = false);
    void writeByte(quint8 byte);

    void processData(const QByteArray &data);
    void resetBuffer();

    void openSerialPort();
    void closeSerialPort();

};

#endif // ROBOTSERIALINTERFACE_H
