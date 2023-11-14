#include "robotcontroller.h"

Q_LOGGING_CATEGORY(dcRobotController, "RobotController")


RobotController::RobotController(QObject *parent)
    : QObject{parent}
{
    m_uartInterface = new UartInterface(this);
    connect(m_uartInterface, &UartInterface::availableChanged, this, &RobotController::onInterfaceAvailableChanged);
    connect(m_uartInterface, &UartInterface::packetReceived, this, &RobotController::onInterfacePacketReceived);

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        qCInfo(dcRobotController()) << "[+] Checking serial port" << serialPortInfo.systemLocation();
        if (serialPortInfo.systemLocation() == "/dev/ttyUSB0") {
            m_uartInterface->setSerialPortInfo(serialPortInfo);
        }
    }

    m_uartInterface->enable();

    qCDebug(dcRobotController()) << "Created successfully";
}

UartInterface *RobotController::uartInterface() const
{
    return m_uartInterface;
}

RobotController::State RobotController::state() const
{
    return m_state;
}

QString RobotController::firmwareVersion() const
{
    return m_firmwareVersion;
}

RobotControllerReply *RobotController::getFirmwareVersion()
{
    qCDebug(dcRobotController()) << "Reading firmware version from robot controller";
    RobotControllerReply *reply = createReply(RobotControllerPacket(RobotControllerPacket::CommandGetFirmwareVersion, m_packetId++));
    m_uartInterface->sendPacket(reply->requestPacket());
    m_pendingReplies.insert(reply->packetId(), reply);
    reply->startWait();
    return reply;
}

void RobotController::onInterfaceAvailableChanged(bool available)
{
    if (available) {
        // Start initializing the robot
        setState(StateInitializing);

        RobotControllerReply *firmwareReply = getFirmwareVersion();
        connect(firmwareReply, &RobotControllerReply::finished, this, [this, firmwareReply](){
            if (firmwareReply->error() != RobotControllerReply::ErrorNoError) {
                qCWarning(dcRobotController()) << "Could not read firmware version. The reply finished with error" << firmwareReply->error();
                setState(StateError);
                return;
            }

            QByteArray firmwareData = firmwareReply->responsePacket().payload();
            if (firmwareData.length() != 3) {
                qCWarning(dcRobotController()) << "Could not read firmware version. The response has an unexpected payload length" << firmwareData.length() << firmwareData.toHex();
                setState(StateError);
                return;
            }

            QString firmwareVersion = QString("%1.%2.%3").arg(firmwareData.at(0)).arg(firmwareData.at(1)).arg(firmwareData.at(2));
            if (m_firmwareVersion != firmwareVersion) {
                m_firmwareVersion = firmwareVersion;
                emit firmwareVersionChaged(m_firmwareVersion);
            }

            // Get status

            setState(StateReady);
        });
    } else {
        // Cleanup
        foreach (RobotControllerReply *pendingReply, m_pendingReplies) {
            qCDebug(dcRobotController()) << "Abort request" << pendingReply->requestPacket() << "because the interface is not available any more.";
            pendingReply->abort();
        }

        m_packetId = 0;
        m_pendingReplies.clear();
        m_firmwareVersion.clear();
        setState(StateDisconnected);
    }
}

void RobotController::onInterfacePacketReceived(const QByteArray &packetData)
{

}

void RobotController::setState(State state)
{
    if (m_state == state)
        return;

    qCInfo(dcRobotController()) << "State changed:" << state;
    m_state = state;
    emit stateChanged(m_state);
}

RobotControllerReply *RobotController::createReply(const RobotControllerPacket &requestPacket)
{
    RobotControllerReply *reply = new RobotControllerReply(requestPacket, this);
    connect(reply, &RobotControllerReply::finished, reply, &RobotControllerReply::deleteLater);
    connect(reply, &RobotControllerReply::finished, reply, [this, reply](){
        if (m_pendingReplies.contains(reply->packetId())) {
            m_pendingReplies.remove(reply->packetId());
        }
    });

    return reply;

}
