#include "SerialInterface.h"
#include "MotorController.h"

SerialInterface::SerialInterface(HardwareSerial &serial, MotorController *motorController) :
    m_motorController(motorController),
    m_hardwareSerial(&serial)
{

}

SerialInterface::~SerialInterface()
{
    m_hardwareSerial->end();
}

void SerialInterface::init()
{
    m_hardwareSerial->begin(115200);
    delay(250);
}

void SerialInterface::process()
{
    while (m_hardwareSerial->available()) {
        uint8_t receivedByte = m_hardwareSerial->read();
        processReceivedByte(receivedByte);
    }
}

void SerialInterface::sendData(const char *data, size_t len)
{
    // Stream the data slip encoded
    streamByte(SlipProtocolEnd, true);
    for (size_t i = 0; i < len; i++) {
        streamByte(data[i]);
    }
    streamByte(SlipProtocolEnd, true);
    m_hardwareSerial->flush();
}

void SerialInterface::debug(const char *message)
{
    size_t payloadLength = strlen(message);
    uint8_t payload[payloadLength];
    for (uint8_t i = 0; i < payloadLength; i++) {
        payload[i] = static_cast<uint8_t>(message[i]);
    }
    sendNotification(NotificationDebugMessage, payload, payloadLength);
}

uint16_t SerialInterface::calculateCrc(uint8_t data[], size_t length)
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

void SerialInterface::processReceivedByte(uint8_t receivedByte)
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
            // 1 command, 1 packetId, 2 crc16 bytes
            if (m_bufferIndex >= 4) {
                /* Check CRC */
                uint16_t receivedCrc = (uint16_t)m_buffer[m_bufferIndex - 1];
                receivedCrc |= (uint16_t)(m_buffer[m_bufferIndex - 2]) << 8;

                uint16_t calculatedCrc = calculateCrc(m_buffer, m_bufferIndex - 2);

                if (receivedCrc != calculatedCrc) {
                    sendResponse(m_buffer[0], m_buffer[1], StatusChecksumError);
                } else {
                    processData(m_buffer, m_bufferIndex - 2);
                }
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


void SerialInterface::processData(uint8_t buffer[], uint8_t length)
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
        case CommandGetStatus: {
            if (length != 2) {
                sendResponse(command, requestId, StatusInvalidPlayload);
                return;
            }
            
            uint8_t payloadSize = 2;
            uint8_t payload[payloadSize];
            payload[0] = ControllerStatusOk;
            payload[1] = m_motorController->stepperEnabled() ? 1 : 0;
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
        case CommandGetMotorInformation: {
            if (length < 2 || length > 3) {
                sendResponse(command, requestId, StatusInvalidPlayload);
                return;
            }

            /*  3 motors, each has following bytes:
                    1 byte running
                    4 byte current position
             */

            size_t motorCount = 3;
            size_t payloadSize = motorCount * 13;
            uint8_t payload[payloadSize];
            for (size_t i = 0; i < motorCount; i++) {
                RobotStepper *robotStepper = nullptr;
                switch (i) {
                case 0:
                    robotStepper = m_motorController->stepper1();
                    break;
                case 1:
                    robotStepper = m_motorController->stepper2();
                    break;
                case 2:
                    robotStepper = m_motorController->stepper3();
                    break;
                default:
                    break;
                }

                size_t index = 0;
                payload[index] = robotStepper->isRunning() ? 1 : 0;

                index += writeUint32ToByteArray(payload, index, robotStepper->currentPosition());
                index += writeUint32ToByteArray(payload, index, robotStepper->targetPosition());
                index += writeFloatToByteArray(payload, index, robotStepper->speed());
            }

            sendResponse(command, requestId, StatusSuccess, payload, payloadSize);
            break;
        }
        default:
            sendResponse(command, requestId, StatusInvalidCommand);
            break;
    }
}

void SerialInterface::streamByte(uint8_t dataByte, boolean specialCharacter)
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

void SerialInterface::writeByte(uint8_t dataByte)
{
    m_hardwareSerial->write(dataByte);
}

void SerialInterface::sendPacket(uint8_t packet[], uint8_t length)
{
    streamByte(SlipProtocolEnd, true);

    // Stream data
    for (size_t i = 0; i < length; i++) {
        streamByte(packet[i]);
    }

    // Send CRC16
    uint16_t crc = calculateCrc(packet, length);
    streamByte(static_cast<uint8_t>((crc >> 8) & 0xff));
    streamByte(static_cast<uint8_t>(crc & 0xff));
    
    streamByte(SlipProtocolEnd, true);

    m_hardwareSerial->flush();
}

void SerialInterface::sendResponse(uint8_t command, uint8_t requestId, Status status, uint8_t payload[], size_t payloadLenght)
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

void SerialInterface::sendNotification(SerialInterface::Notification notification, uint8_t payload[], size_t payloadLenght)
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

size_t SerialInterface::writeFloatToByteArray(uint8_t byteArray[], size_t offset, float value)
{
    size_t typeSize = sizeof(float);
    uint8_t typeData[typeSize];
    memcpy(typeData, &value, typeSize);
    // Arduino is little endian, the protocol is big endian, 
    // we need to change the endianess here
    for (size_t i = 0; i < typeSize; i++) {
        byteArray[offset + typeSize - i] = typeData[i];
    }
    return typeSize;
}

size_t SerialInterface::writeUint32ToByteArray(uint8_t byteArray[], size_t offset, uint32_t value)
{
    size_t typeSize = sizeof(uint32_t);
    uint8_t typeData[typeSize];
    memcpy(typeData, &value, typeSize);
    // Arduino is little endian, the protocol is big endian, 
    // we need to change the endianess here
    for (size_t i = 0; i < typeSize; i++) {
        byteArray[offset + typeSize - i] = typeData[i];
    }
    return typeSize;
}
