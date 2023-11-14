#include "robotcontrollerreply.h"

RobotControllerPacket RobotControllerReply::requestPacket() const
{
    return m_requestPacket;
}

RobotControllerPacket RobotControllerReply::responsePacket() const
{
    return m_responsePacket;
}

quint8 RobotControllerReply::packetId() const
{
    return m_requestPacket.packetId();
}

RobotControllerReply::Error RobotControllerReply::error() const
{
    return m_error;
}

RobotControllerReply::RobotControllerReply(const RobotControllerPacket &requestPacket, QObject *parent) :
    QObject{parent},
    m_requestPacket{requestPacket}
{
    connect(&m_timer, &QTimer::timeout, this, &RobotControllerReply::onTimeout);
}

void RobotControllerReply::abort()
{
    m_timer.stop();
    m_error = ErrorAborted;
    emit finished();
}

void RobotControllerReply::startWait()
{
    m_timer.start(m_timeoutInterval);
}

void RobotControllerReply::setFinished()
{
    m_timer.stop();
    if (m_responsePacket.isValid()) {
        if (m_responsePacket.status() != RobotControllerPacket::StatusSuccess) {
            m_error = ErrorInterfaceError;
        } else {
            m_error = ErrorNoError;
        }
    }

    emit finished();
}

void RobotControllerReply::onTimeout()
{
    m_error = ErrorTimeout;
    emit finished();

}
