#ifndef ROBOTCONTROLLERREPLY_H
#define ROBOTCONTROLLERREPLY_H

#include <QObject>
#include <QTimer>

#include "robotcontrollerpacket.h"

class RobotControllerReply : public QObject
{
    Q_OBJECT

    friend class RobotController;

public:
    enum Error {
        ErrorNoError,
        ErrorInterfaceError,
        ErrorTimeout,
        ErrorAborted,
    };
    Q_ENUM(Error)

    RobotControllerPacket requestPacket() const;
    RobotControllerPacket responsePacket() const;

    quint8 packetId() const;

    RobotControllerReply::Error error() const;

private:
    RobotControllerReply(const RobotControllerPacket &requestPacket, QObject *parent = nullptr);
    RobotControllerPacket m_requestPacket;
    RobotControllerPacket m_responsePacket;

    Error m_error = ErrorNoError;

    QTimer m_timer;
    int m_timeoutInterval = 2000;

    void abort();
    void startWait();
    void setFinished();

signals:
    void finished();

private slots:
    void onTimeout();

};

#endif // ROBOTCONTROLLERREPLY_H
