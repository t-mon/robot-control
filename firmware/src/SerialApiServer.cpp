#include "SerialApiServer.h"
#include "MotorController.h"

SerialApiServer::SerialApiServer(HardwareSerial &serial, MotorController *motorController) :
    m_motorController(motorController),
    m_hardwareSerial(&serial)
{

}

SerialApiServer::~SerialApiServer()
{
    m_hardwareSerial->end();
}

void SerialApiServer::init()
{
    m_hardwareSerial->begin(115200);
    delay(250);
    sendNotification(NotificationReady);
}

void SerialApiServer::process()
{
    while (m_hardwareSerial->available()) {
        uint8_t receivedByte = m_hardwareSerial->read();
        processReceivedByte(receivedByte);
    }
}

void SerialApiServer::sendData(const char *data, size_t len)
{
    // Stream the data slip encoded
    streamByte(SlipProtocolEnd, true);
    for (size_t i = 0; i < len; i++) {
        streamByte(data[i]);
    }
    streamByte(SlipProtocolEnd, true);
    m_hardwareSerial->flush();
}

void SerialApiServer::debug(const char *message)
{
    size_t payloadLength = strlen(message);
    uint8_t payload[payloadLength];
    for (uint8_t i = 0; i < payloadLength; i++) {
        payload[i] = static_cast<uint8_t>(message[i]);
    }
    sendNotification(NotificationDebugMessage, payload, payloadLength);
}

uint16_t SerialApiServer::calculateCrc(uint8_t data[], size_t length)
{
    // CRC-16/CCITT-FALSE
    uint16_t polynom = 0x1021;
    uint16_t crc = 0xffff;

    for (size_t i = 0; i < length; i++) {
        crc ^= static_cast<uint16_t>(data[i] << 8);
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ polynom;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

void SerialApiServer::processReceivedByte(uint8_t receivedByte)
{
    if (m_protocolEscaping) {
        switch (receivedByte) {
            case SlipProtocolTransposedEnd:
                m_buffer[m_bufferIndex++] = SlipProtocolEnd;
                m_protocolEscaping = false;
                break;
            case SlipProtocolTransposedEsc:
                m_buffer[m_bufferIndex++] = SlipProtocolEsc;
                m_protocolEscaping = false;
                break;
            default:
                // SLIP protocol violation...received escape, but it is not an escaped byte
                break;
        }
    }

    switch (receivedByte) {
        case SlipProtocolEnd:
            // We are done with this package, process it and reset the buffer
            if (m_bufferIndex > 0) {
                processData(m_buffer, m_bufferIndex);
            }
            m_bufferIndex = 0;
            m_protocolEscaping = false;
            break;
        case SlipProtocolEsc:
            // The next byte will be escaped, lets wait for it
            m_protocolEscaping = true;
            break;
        default:
            // Nothing special, just add to buffer
            m_buffer[m_bufferIndex++] = receivedByte;
            break;
    }
}

void SerialApiServer::streamByte(uint8_t dataByte, boolean specialCharacter)
{
    // If this is a special character, write it without escaping
    if (specialCharacter) {
        writeByte(dataByte);
    } else {
        switch (dataByte) {
        case SlipProtocolEnd:
            writeByte(SlipProtocolEsc);
            writeByte(SlipProtocolTransposedEnd);
            break;
        case SlipProtocolEsc:
            writeByte(SlipProtocolEsc);
            writeByte(SlipProtocolTransposedEsc);
            break;
        default:
            writeByte(dataByte);
            break;
        }
    }
}

void SerialApiServer::writeByte(uint8_t dataByte)
{
    m_hardwareSerial->write(dataByte);
}

void SerialApiServer::processData(uint8_t buffer[], uint8_t length)
{
    uint8_t command = buffer[0];
    uint8_t requestId = buffer[1];

    switch (command) {
        case CommandGetFirmwareVersion: {
            if (length != 2) {
                sendResponse(command, requestId, StatusInvalidPlayload);
                return;
            }
            
            uint8_t payloadSize = 3;
            uint8_t payload[payloadSize];
            payload[0] = FIRMWARE_MAJOR;
            payload[1] = FIRMWARE_MINOR;
            payload[2] = FIRMWARE_PATCH;
            sendResponse(command, requestId, StatusSuccess, payload, payloadSize);
            break;
        }
        case CommandEnableSteppers: {
            if (length != 3) {
                sendResponse(command, requestId, StatusInvalidPlayload);
                return;
            }
            boolean enabled = buffer[2] != 0;
            m_motorController->setStepperEnabled(enabled);
            sendResponse(command, requestId, StatusSuccess);
            break;
        }
        default:
            sendResponse(command, requestId, StatusInvalidCommand);
            break;
    }
}

void SerialApiServer::sendPacket(uint8_t packet[], uint8_t length)
{
    streamByte(SlipProtocolEnd, true);

    // Stream data
    for (size_t i = 0; i < length; i++) {
        streamByte(packet[i]);
    }

    // Send crc
    uint16_t crc = calculateCrc(packet, length);
    streamByte(static_cast<uint8_t>(crc & 0xff));
    streamByte(static_cast<uint8_t>((crc >> 8) & 0xff));

    streamByte(SlipProtocolEnd, true);

    m_hardwareSerial->flush();
}

void SerialApiServer::sendResponse(uint8_t command, uint8_t requestId, Status status, uint8_t payload[], size_t payloadLenght)
{
    size_t packetSize = 3 + payloadLenght;
    uint8_t packet[packetSize];
    packet[0] = command;
    packet[1] = requestId;
    packet[2] = status;
    for (size_t i = 0; i < payloadLenght; i++) {
        packet[i + 3] = payload[i];
    }

    sendPacket(packet, packetSize);
}

void SerialApiServer::sendNotification(SerialApiServer::Notification notification, uint8_t payload[], size_t payloadLenght)
{
    size_t packetSize = 2 + payloadLenght;
    uint8_t packet[packetSize];
    packet[0] = notification;
    packet[1] = m_notificationId++;
    for (size_t i = 0; i < payloadLenght; i++) {
        packet[2 + i] = payload[i];
    }
    sendPacket(packet, packetSize);
}