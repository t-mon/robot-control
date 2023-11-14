#include "robotcontrollerpacket.h"
#include "uartinterface.h"

RobotControllerPacket::RobotControllerPacket(const QByteArray &packetData) :
    m_packetData{packetData}
{
    m_command = static_cast<Command>(m_packetData.at(0));
    m_packetId = static_cast<quint8>(m_packetData.at(1));
    if (m_command <= 0xf0 ) {
        m_type = TypeResponse;
        m_status = static_cast<Status>(m_packetData.at(2));
        m_payload = m_packetData.right(m_packetData.length() - 3);
    } else {
        m_type = TypeNotification;
        m_payload = m_packetData.right(m_packetData.length() - 2);
    }

}

RobotControllerPacket::RobotControllerPacket(Command command, quint8 packetId, const QByteArray &payload, Type type) :
    m_command{command},
    m_packetId{packetId},
    m_payload{payload},
    m_type{type}
{
    // Check if we have data
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::ReadWrite);
    stream << static_cast<quint8>(m_command);
    stream << m_packetId;
    for (int i = 0; i < m_payload.length(); i++) {
        stream << static_cast<quint8>(m_payload.at(i));
    }
    m_packetData = packet;

    Q_ASSERT_X(isValid(), "RobotControllerPacket", "try to build a packet which is not valid.");
}

RobotControllerPacket::Type RobotControllerPacket::type() const
{
    return m_type;
}

RobotControllerPacket::Command RobotControllerPacket::command() const
{
    Q_ASSERT_X(m_type != TypeNotification, "RobotControllerPacket", "reading command() from packet type notification.");
    return m_command;
}

RobotControllerPacket::Notification RobotControllerPacket::notification() const
{
    Q_ASSERT_X(m_type == TypeNotification, "RobotControllerPacket", "reading notification() from packet which is a request or response.");
    return notification();
}

quint8 RobotControllerPacket::packetId() const
{
    return m_packetId;
}

RobotControllerPacket::Status RobotControllerPacket::status() const
{
    Q_ASSERT_X(m_type == TypeResponse, "RobotControllerPacket", "reading status() from packet type other than response. This is not valid.");
    return m_status;
}

QByteArray RobotControllerPacket::payload() const
{
    return m_payload;
}

QByteArray RobotControllerPacket::packetData() const
{
    return m_packetData;
}

bool RobotControllerPacket::isValid() const
{
    return m_command != RobotControllerPacket::CommandUnknown && m_packetData.length() >= 2;
}

QDebug operator<<(QDebug debug, const RobotControllerPacket &packet)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "Packet(";
    switch (packet.type()) {
    case RobotControllerPacket::TypeRequest:
        debug.nospace() << "Request, ";
        debug.nospace() << static_cast<RobotControllerPacket::Command>(packet.command()) << ", ";
        debug.nospace() << "id: " << UartInterface::byteToHexString(packet.packetId())  << " (" << packet.packetId() << ")";
        if (!packet.payload().isEmpty())
            debug.nospace() << ", " << packet.payload().toHex();

        break;
    case RobotControllerPacket::TypeNotification:
        debug.nospace() << "Notification, ";
        debug.nospace() << static_cast<RobotControllerPacket::Notification>(packet.command()) << ", ";
        debug.nospace() << "id: " << UartInterface::byteToHexString(packet.packetId())  << " (" << packet.packetId() << ")";
        if (!packet.payload().isEmpty())
            debug.nospace() << ", " << packet.payload().toHex();

        break;
    case RobotControllerPacket::TypeResponse:
        debug.nospace() << "Response, ";
        debug.nospace() << static_cast<RobotControllerPacket::Command>(packet.command()) << ", ";
        debug.nospace()  << "id: " << UartInterface::byteToHexString(packet.packetId())  << " (" << packet.packetId() << ")";
        debug.nospace() << packet.status() << ", ";
        if (!packet.payload().isEmpty())
            debug.nospace() << ", " << packet.payload().toHex();

        break;

    case RobotControllerPacket::TypeUnknown:
        debug.nospace() << "Unknown packet type";
        break;
    }

    debug.nospace() << ")";
    return debug;
}
