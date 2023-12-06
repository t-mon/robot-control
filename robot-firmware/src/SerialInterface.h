#ifndef SERIALINTERFACE_H
#define SERIALINTERFACE_H

#include <Arduino.h>

class MotorController;

class SerialInterface
{
public:
    enum Command {
        CommandGetFirmwareVersion = 0x00,
        CommandGetStatus = 0x01,
        CommandResetController = 0x02,
        CommandEnableSteppers = 0x10,
        CommandGetMotorInformation = 0x11
    };

    enum Notification {
        NotificationReady = 0xf0,
        NotificationDebugMessage = 0xfe,
        NotificationUnknown = 0xff
    };

    enum ControllerStatus {
        ControllerStatusOk = 0x00,
        ControllerStatusError = 0x01,
        ControllerStatusUnknown = 0xff
    };
    
    enum Status {
        StatusSuccess = 0x00,
        StatusInvalidProtocol = 0x01,
        StatusInvalidCommand = 0x02,
        StatusInvalidPlayload = 0x03,
        StatusChecksumError = 0x04,
        StatusUnknownError = 0xff
    };

    SerialInterface(HardwareSerial &serial, MotorController *motorController);
    ~SerialInterface();

    void init();

    void process();

    void sendData(const char *data, size_t len);
    void debug(const char *message);

    void sendNotification(SerialInterface::Notification notification, uint8_t payload[] = nullptr, size_t payloadLenght = 0);

    static size_t writeFloatToByteArray(uint8_t byteArray[], size_t offset, float value);
    static size_t writeUint32ToByteArray(uint8_t byteArray[], size_t offset, uint32_t value);

private:
    enum SlipProtocol {
        SlipProtocolEnd = 0xC0,
        SlipProtocolEsc = 0xDB,
        SlipProtocolTransposedEnd = 0xDC,
        SlipProtocolTransposedEsc = 0xDD
    };

    MotorController *m_motorController = nullptr;

    // UART read
    HardwareSerial *m_hardwareSerial = nullptr;
    
    uint8_t m_buffer[255];
    uint8_t m_bufferIndex = 0;
    boolean m_protocolEscaping = false;
    uint8_t m_notificationId = 0;

    uint16_t calculateCrc(uint8_t data[], size_t length);

protected:
    virtual void processReceivedByte(uint8_t receivedByte);
    virtual void processData(uint8_t buffer[], uint8_t length);
    
    virtual void streamByte(uint8_t dataByte, boolean specialCharacter = false);
    virtual void writeByte(uint8_t dataByte);
    
    virtual void sendPacket(uint8_t packet[], uint8_t length);
    virtual void sendResponse(uint8_t command, uint8_t requestId, Status status, uint8_t payload[] = nullptr, size_t payloadLenght = 0);

};

#endif // SERIALINTERFACE_H