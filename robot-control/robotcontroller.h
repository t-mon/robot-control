#ifndef ROBOTCONTROLLER_H
#define ROBOTCONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include <QTimer>

#include "robotserialinterface.h"
#include "robotcontrollerreply.h"

Q_DECLARE_LOGGING_CATEGORY(dcRobotController)

class RobotController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(RobotSerialInterface *interface READ interface CONSTANT FINAL)
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)

    Q_PROPERTY(QString firmwareVersion READ firmwareVersion NOTIFY firmwareVersionChanged FINAL)
    Q_PROPERTY(ControllerStatus controllerStatus READ controllerStatus NOTIFY controllerStatusChanged FINAL)
    Q_PROPERTY(bool steppersEnabled READ steppersEnabled WRITE setSteppersEnabled NOTIFY steppersEnabledChanged FINAL)

public:
    enum State {
        StateDisconnected,
        StateInitializing,
        StateReady,
        StateError,
        StateUnknown
    };
    Q_ENUM(State)

    enum ControllerStatus {
        ControllerStatusOk = 0x00,
        ControllerStatusError = 0x01,
        ControllerStatusUnknown = 0xff
    };
    Q_ENUM(ControllerStatus)

    explicit RobotController(QObject *parent = nullptr);

    RobotSerialInterface *interface() const;

    State state() const;

    // Controller properties
    QString firmwareVersion() const;
    ControllerStatus controllerStatus() const;

    bool steppersEnabled() const;
    void setSteppersEnabled(bool enabled);

    // Firmware commands
    RobotControllerReply *getFirmwareVersion();
    RobotControllerReply *getStatus();
    RobotControllerReply *enableSteppers(bool enabled);
    RobotControllerReply *getMotorsInformation();


signals:
    void stateChanged(State state);
    void firmwareVersionChanged(const QString &firmwareVersion);
    void controllerStatusChanged(ControllerStatus controllerStatus);
    void steppersEnabledChanged(bool steppersEnabled);
    void enableSteppersFailed();

private slots:
    void onInterfaceAvailableChanged(bool available);
    void onInterfacePacketReceived(const RobotControllerPacket &packet);

private:
    RobotSerialInterface *m_interface = nullptr;

    QTimer m_readyTimer;

    State m_state = StateUnknown;
    QString m_firmwareVersion;
    ControllerStatus m_controllerStatus = ControllerStatusUnknown;
    bool m_steppersEnabled = false;

    void setState(State state);

    // Protocol
    quint8 m_packetId = 0;
    QHash<quint8, RobotControllerReply *> m_pendingReplies;

    RobotControllerReply *createReply(const RobotControllerPacket &requestPacket);

    void initControllerInformation();
    void processResponse(const RobotControllerPacket &responsePacket);
    void processNotification(const RobotControllerPacket &notificationPacket);

};

#endif // ROBOTCONTROLLER_H
