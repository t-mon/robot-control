#include "uartinterface.h"

Q_LOGGING_CATEGORY(dcUartInterface, "UartInterface")


UartInterface::UartInterface(QObject *parent)
    : QObject{parent}
{

    quint16 foo = 0x1122;
    QByteArray data = "AABBCCDDEEFF";

    qCInfo(dcUartInterface()) << uint16ToHexString(foo);
    qCInfo(dcUartInterface()) << "crc of" << data << calculateCrc(data);


    m_serialPort = new QSerialPort(this);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error){
        qCWarning(dcUartInterface()) << "Error occurred" << error << m_serialPort->errorString();
    });

    connect(m_serialPort, &QSerialPort::readyRead, this, [this](){
        QByteArray data = m_serialPort->readAll();
        for (int i = 0; i < data.length(); i++) {
            quint8 byte = static_cast<quint8>(data.at(i));
            qCDebug(dcUartInterface()) << "<--" << UartInterface::byteToHexString(byte);
            switch (byte) {
            case ProtocolByteEnd: {

                // This is the end of the message. Process whatever is in the buffer and reset afterwards.

                // The minimum data size to interprete is 3 bytes: 1 Command, 1 Packet ID and one CRC byte.
                if (m_dataBuffer.length() < 3) {
                    resetBuffer();
                    break;
                }

                // Data in the buffer has already been unescaped.

                // Last byte is the crc
                QByteArray packetData = m_dataBuffer.left(m_dataBuffer.length() - 2);
                /* Verify crc */
                quint16 crcReceived = m_dataBuffer.at(m_dataBuffer.length() - 1);
                crcReceived |= m_dataBuffer.at(m_dataBuffer.length() - 2) << 8;

                quint16 crcCalculated = calculateCrc(packetData);
                if (crcCalculated != crcReceived) {
                    qCWarning(dcUartInterface()) << "Received packet with invalid CRC value: Buffer:" << m_dataBuffer.toHex() << "calculated CRC:" << UartInterface::uint16ToHexString(crcCalculated) << "!=" << UartInterface::uint16ToHexString(crcReceived) << ". Discard data...";
                    resetBuffer();
                    break;
                }

                qCDebug(dcUartInterface()) << "Buffer:" << m_dataBuffer.toHex() << "Packet:" << packetData.toHex() << "CRC OK:" << UartInterface::byteToHexString(crcReceived);
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
                        qCWarning(dcUartInterface()) << "SLIP protocol violation. Received unexpected stuffed byte. Discard data...";
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

    });
}

QSerialPortInfo UartInterface::serialPortInfo() const
{
    return m_serialPortInfo;
}

void UartInterface::setSerialPortInfo(const QSerialPortInfo &serialPortInfo)
{
    if (m_serialPortInfo.systemLocation() == serialPortInfo.systemLocation())
        return;

    m_serialPortInfo = serialPortInfo;
    m_serialPort->close();

    m_available = false;
    emit availableChanged(m_available);

    m_serialPort->setPort(m_serialPortInfo);

    if (m_enabled) {
        openSerialPort();
    }
}

bool UartInterface::enabled() const
{
    return m_enabled;
}

bool UartInterface::available() const
{
    return m_available;
}

void UartInterface::sendPacket(const RobotControllerPacket &packet)
{
    qCDebug(dcUartInterface()) << "Sending packet" << packet.packetData().toHex();
    streamByte(ProtocolByteEnd, true);
    for (int i = 0; i < packet.packetData().size(); i++) {
        streamByte(static_cast<quint8>(packet.packetData().at(i)));
    }
    quint16 crc = calculateCrc(packet.packetData());
    streamByte(static_cast<quint8>(crc) & 0xff);
    streamByte(static_cast<quint8>(crc >> 8) & 0xff);
    streamByte(ProtocolByteEnd, true);
}

void UartInterface::enable()
{
    if (m_enabled)
        return;

    m_enabled = true;
    emit enabledChanged(m_enabled);

    if (!m_serialPortInfo.isNull())
        openSerialPort();

}

void UartInterface::disable()
{
    if (!m_enabled)
        return;

    m_enabled = false;
    emit enabledChanged(m_enabled);
}

void UartInterface::setEnabled(bool enabled)
{
    if (enabled) {
        enable();
    } else {
        disable();
    }
}

quint16 UartInterface::calculateCrc(const QByteArray &data)
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

void UartInterface::streamByte(quint8 byte, bool specialCharacter)
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

void UartInterface::writeByte(quint8 byte)
{
    qCDebug(dcUartInterface()) << "-->" << UartInterface::byteToHexString(byte);
    m_serialPort->write(QByteArray(1, byte));
}

void UartInterface::processData(const QByteArray &data)
{
    qCDebug(dcUartInterface()) << "Received packet data" << data.toHex();
    emit packetReceived(data);
}

void UartInterface::resetBuffer()
{
    m_dataBuffer.clear();
    m_escape = false;
}

void UartInterface::openSerialPort()
{
    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort->setParity(QSerialPort::NoParity);

    if (!m_serialPort->open(QSerialPort::ReadWrite)) {
        qCWarning(dcUartInterface()) << "Failed to open serial port" << m_serialPort->portName() << m_serialPort->errorString();
        m_available = false;
        emit availableChanged(m_available);
    } else {
        qCDebug(dcUartInterface()) << "Serial port opened successfully" << m_serialPort->portName();
        m_available = true;
        emit availableChanged(m_available);
    }
}
