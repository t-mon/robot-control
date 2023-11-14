#ifndef ROBOTCONTROLLER_H
#define ROBOTCONTROLLER_H

#include <QObject>
#include <QQmlEngine>

#include "uartinterface.h"
#include "robotcontrollerreply.h"

Q_DECLARE_LOGGING_CATEGORY(dcRobotController)

class RobotController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(UartInterface *uartInterface READ uartInterface CONSTANT FINAL)

public:
    enum State {
        StateDisconnected,
        StateInitializing,
        StateReady,
        StateError,
        StateUnknown
    };
    Q_ENUM(State)

    explicit RobotController(QObject *parent = nullptr);

    UartInterface *uartInterface() const;

    State state() const;
    QString firmwareVersion() const;

    RobotControllerReply *getFirmwareVersion();

signals:
    void stateChanged(State state);
    void firmwareVersionChaged(const QString &firmwareVersion);

private slots:
    void onInterfaceAvailableChanged(bool available);
    void onInterfacePacketReceived(const QByteArray &packetData);

private:
    UartInterface *m_uartInterface = nullptr;

    State m_state = StateUnknown;
    QString m_firmwareVersion;

    void setState(State state);

    // Protocol
    quint8 m_packetId = 0;
    QHash<quint8, RobotControllerReply *> m_pendingReplies;

    RobotControllerReply *createReply(const RobotControllerPacket &requestPacket);

};

#endif // ROBOTCONTROLLER_H
