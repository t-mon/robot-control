#ifndef ROBOTCONTROLLERPACKET_H
#define ROBOTCONTROLLERPACKET_H

#include <QObject>
#include <QDebug>

class RobotControllerPacket
{
    Q_GADGET
public:
    enum Type {
        TypeRequest,
        TypeResponse,
        TypeNotification,
        TypeUnknown
    };
    Q_ENUM(Type)

    enum Status {
        StatusSuccess = 0x00,
        StatusInvalidProtocol = 0x01,
        StatusInvalidCommand = 0x02,
        StatusInvalidPlayload = 0x03,
        StatusChecksumError = 0x04,
        StatusUnknown = 0xff
    };
    Q_ENUM(Status)

    enum Command {
        CommandGetFirmwareVersion = 0x00,
        CommandGetStatus = 0x01,
        CommandResetController = 0x02,
        CommandEnableSteppers = 0x10,
        CommandGetMotorInformation = 0x11,
        CommandUnknown = 0xff
    };
    Q_ENUM(Command)

    enum Notification {
        NotificationReady = 0xf0,
        NotificationDebugMessage = 0xfe,
        NotificationUnknown = 0xff
    };
    Q_ENUM(Notification)

    explicit RobotControllerPacket() = default;
    RobotControllerPacket(const QByteArray &packetData);
    RobotControllerPacket(Command command, quint8 packetId, const QByteArray &payload = QByteArray(), Type type = TypeRequest);

    Type type() const;
    Command command() const;
    Notification notification() const;

    quint8 packetId() const;
    Status status() const;
    QByteArray payload() const;

    QByteArray packetData() const;

    bool isValid() const;

private:
    Type m_type = TypeUnknown;
    Command m_command = CommandUnknown;
    Notification m_notification = NotificationUnknown;

    quint8 m_packetId = 0;
    Status m_status = StatusUnknown;
    QByteArray m_payload;

    QByteArray m_packetData;

    void parsePacketData();
    void buildPacket();
};

QDebug operator<<(QDebug debug, const RobotControllerPacket &packet);

#endif // ROBOTCONTROLLERPACKET_H
