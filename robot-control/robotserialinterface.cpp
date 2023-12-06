#include "robotserialinterface.h"

Q_LOGGING_CATEGORY(dcRobotSerialInterface, "RobotSerialInterface")
Q_LOGGING_CATEGORY(dcRobotSerialInterfaceTraffic, "RobotSerialInterfaceTraffic")

RobotSerialInterface::RobotSerialInterface(QObject *parent)
    : QObject{parent}
{
    m_serialPort = new QSerialPort(this);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error){
        if (error != QSerialPort::NoError) {
            qCWarning(dcRobotSerialInterface()) << "Error occurred" << error << m_serialPort->errorString();
            if (m_serialPort->isOpen()) {
                qCWarning(dcRobotSerialInterface()) << "Cloing serial port due to error" << m_serialPort->errorString();
                closeSerialPort();
            }
        }
    });

    connect(m_serialPort, &QSerialPort::readyRead, this, &RobotSerialInterface::onReadyRead);
}

QSerialPortInfo RobotSerialInterface::serialPortInfo() const
{
    return m_serialPortInfo;
}

void RobotSerialInterface::setSerialPortInfo(const QSerialPortInfo &serialPortInfo)
{
    m_serialPortInfo = serialPortInfo;

    if (m_serialPort->isOpen()) {
        qCDebug(dcRobotSerialInterface()) << "Using serial port" << serialPortInfo.systemLocation() << "now. Reset interface...";
        closeSerialPort();
    }

    m_serialPort->setPort(m_serialPortInfo);

    if (m_enabled)
        openSerialPort();
}

bool RobotSerialInterface::enabled() const
{
    return m_enabled;
}

bool RobotSerialInterface::available() const
{
    return m_available;
}

void RobotSerialInterface::sendPacket(const RobotControllerPacket &packet)
{
    qCDebug(dcRobotSerialInterfaceTraffic()) << "Sending packet" << packet.packetData().toHex();
    streamByte(ProtocolByteEnd, true);
    for (int i = 0; i < packet.packetData().size(); i++) {
        streamByte(static_cast<quint8>(packet.packetData().at(i)));
    }
    quint16 crc = calculateCrc(packet.packetData());
    streamByte(static_cast<quint8>(crc >> 8) & 0xff);
    streamByte(static_cast<quint8>(crc) & 0xff);
    streamByte(ProtocolByteEnd, true);
}

void RobotSerialInterface::enable()
{
    if (m_enabled)
        return;

    m_enabled = true;
    emit enabledChanged(m_enabled);

    if (!m_serialPortInfo.isNull())
        openSerialPort();

}

void RobotSerialInterface::disable()
{
    if (!m_enabled)
        return;

    m_enabled = false;
    emit enabledChanged(m_enabled);
}

void RobotSerialInterface::setEnabled(bool enabled)
{
    if (enabled) {
        enable();
    } else {
        disable();
    }
}

void RobotSerialInterface::onReadyRead()
{
    QByteArray data = m_serialPort->readAll();
    for (int i = 0; i < data.length(); i++) {
        quint8 byte = static_cast<quint8>(data.at(i));
        qCDebug(dcRobotSerialInterfaceTraffic()) << "<--" << RobotSerialInterface::byteToHexString(byte);
        switch (byte) {
        case ProtocolByteEnd: {

            // This is the end of the message. Process whatever is in the buffer and reset afterwards.

            // The minimum data size to interprete is 3 bytes: 1 Command, 1 Packet ID and one CRC byte.
            if (m_dataBuffer.length() < 3) {
                resetBuffer();
                break;
            }

            // Note: data in the buffer has already been unescaped.

            QByteArray packetData = m_dataBuffer.left(m_dataBuffer.length() - 2);
            QByteArray crcData = m_dataBuffer.right(2);
            qCDebug(dcRobotSerialInterfaceTraffic()) << "Packet received:" << packetData.toHex() << "CRC data:" << crcData.toHex();

            /* Verify crc */
            quint16 crcReceived;
            QDataStream crcStream(crcData);
            crcStream >> crcReceived;

            quint16 crcCalculated = calculateCrc(packetData);
            if (crcCalculated != crcReceived) {
                qCWarning(dcRobotSerialInterface()) << "Received packet with invalid CRC value: Buffer:" << m_dataBuffer.toHex() << "calculated CRC:" << RobotSerialInterface::uint16ToHexString(crcCalculated) << "!=" << RobotSerialInterface::uint16ToHexString(crcReceived) << ". Discard data...";
                resetBuffer();
                break;
            }

            qCDebug(dcRobotSerialInterface()) << "Buffer:" << m_dataBuffer.toHex() << "Packet:" << packetData.toHex() << "CRC OK:" << RobotSerialInterface::uint16ToHexString(crcReceived);
            processData(packetData);
            resetBuffer();
            break;
        }
        case ProtocolByteEsc:
            /* The next character is stuffed */
            m_escape = true;
            break;
        default:
            if (m_escape) {
                /* This is a stuffed byte */
                m_escape = false;
                switch(byte) {
                case ProtocolByteTransposedEnd:
                    m_dataBuffer.append(static_cast<quint8>(ProtocolByteEnd));
                    break;
                case ProtocolByteTransposedEsc:
                    m_dataBuffer.append(static_cast<quint8>(ProtocolByteEsc));
                    break;
                default:
                    /* SLIP protocol violation...unexpected byte after escape byte */
                    qCWarning(dcRobotSerialInterface()) << "SLIP protocol violation. Received unexpected stuffed byte. Discard data...";
                    resetBuffer();
                    break;
                }
            } else {
                m_escape = false;
                m_dataBuffer.append(byte);
            }
            break;
        }
    }
}

quint16 RobotSerialInterface::calculateCrc(const QByteArray &data)
{
    // CRC-16/CCITT-FALSE

    quint16 polynom = 0x1021;
    quint16 crc = 0xffff;

    for (int i = 0; i < data.size(); i++) {
        crc ^= static_cast<quint16>(data.at(i)) << 8;
        for (quint8 j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ polynom;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

void RobotSerialInterface::streamByte(quint8 byte, bool specialCharacter)
{
    // Special byte: write the byte as given
    if (specialCharacter) {
        writeByte(byte);
        return;
    }

    // Not a special character, we need to stuff the bytes we are gping to sent
    switch(byte) {
    case ProtocolByteEnd:
        writeByte(ProtocolByteEsc);
        writeByte(ProtocolByteTransposedEnd);
        break;
    case ProtocolByteEsc:
        writeByte(ProtocolByteEsc);
        writeByte(ProtocolByteTransposedEsc);
        break;
    default:
        writeByte(byte);
    }
}

void RobotSerialInterface::writeByte(quint8 byte)
{
    qCDebug(dcRobotSerialInterfaceTraffic()) << "-->" << RobotSerialInterface::byteToHexString(byte);
    m_serialPort->write(QByteArray(1, byte));
}

void RobotSerialInterface::processData(const QByteArray &data)
{
    emit dataReceived(data);

    RobotControllerPacket packet(data);
    if (!packet.isValid()) {
        qCWarning(dcRobotSerialInterface()) << "Received invalid packet data" << data.toHex();
        return;
    }

    qCDebug(dcRobotSerialInterface()) << "Received packet" << packet;
    emit packetReceived(packet);
}

void RobotSerialInterface::resetBuffer()
{
    m_dataBuffer.clear();
    m_escape = false;
}

void RobotSerialInterface::openSerialPort()
{
    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort->setParity(QSerialPort::NoParity);

    if (!m_serialPort->open(QSerialPort::ReadWrite)) {
        qCWarning(dcRobotSerialInterface()) << "Failed to open serial port" << m_serialPort->portName() << m_serialPort->errorString();
        m_available = false;
        emit availableChanged(m_available);
    } else {
        qCDebug(dcRobotSerialInterface()) << "Serial port opened successfully" << m_serialPort->portName();
        m_available = true;
        emit availableChanged(m_available);
    }
}

void RobotSerialInterface::closeSerialPort()
{
    m_serialPort->close();
    if (m_available) {
        m_available = false;
        emit availableChanged(m_available);
    }

    resetBuffer();
}
