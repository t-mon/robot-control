#include "robotcontroller.h"

Q_LOGGING_CATEGORY(dcRobotController, "RobotController")

RobotController::RobotController(QObject *parent)
    : QObject{parent}
{
    m_readyTimer.setSingleShot(true);
    m_readyTimer.setInterval(2000);
    connect(&m_readyTimer, &QTimer::timeout, this, [this](){
        // No ready signal received, try to get the status from the firmware
        qCDebug(dcRobotController()) << "Did not receive the ready notification within 2 seconds. Try to request information.";
        initControllerInformation();
    });

    m_interface = new RobotSerialInterface(this);
    connect(m_interface, &RobotSerialInterface::availableChanged, this, &RobotController::onInterfaceAvailableChanged);
    connect(m_interface, &RobotSerialInterface::packetReceived, this, &RobotController::onInterfacePacketReceived);

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        qCInfo(dcRobotController()) << "[+] Checking serial port" << serialPortInfo.systemLocation();
        if (serialPortInfo.systemLocation() == "/dev/ttyACM0") {
            m_interface->setSerialPortInfo(serialPortInfo);
        }
    }

    m_interface->enable();

    qCDebug(dcRobotController()) << "Created successfully";
}

RobotSerialInterface *RobotController::interface() const
{
    return m_interface;
}

RobotController::State RobotController::state() const
{
    return m_state;
}

QString RobotController::firmwareVersion() const
{
    return m_firmwareVersion;
}

RobotController::ControllerStatus RobotController::controllerStatus() const
{
    return m_controllerStatus;
}

bool RobotController::steppersEnabled() const
{
    return m_steppersEnabled;
}

void RobotController::setSteppersEnabled(bool enabled)
{
    if (m_steppersEnabled == enabled)
        return;

    qCDebug(dcRobotController()) << (enabled ? "Enabling" : "Disabling") << "steppers";

    RobotControllerReply *repyl = enableSteppers(enabled);
    connect(repyl, &RobotControllerReply::finished, this, [this, repyl, enabled](){

        if (repyl->error() != RobotControllerReply::ErrorNoError) {
            qCWarning(dcRobotController()) << "Could not enable steppers. The reply finished with error" << repyl->error();
            setState(StateError);
            emit enableSteppersFailed();
            return;
        }

        qCInfo(dcRobotController()) << "Steppers" << (enabled ? "enabled" : "disabled");

        m_steppersEnabled = enabled;
        emit steppersEnabledChanged(m_steppersEnabled);
    });

}

RobotControllerReply *RobotController::getFirmwareVersion()
{
    qCDebug(dcRobotController()) << "Reading firmware version";
    RobotControllerReply *reply = createReply(RobotControllerPacket(RobotControllerPacket::CommandGetFirmwareVersion, m_packetId++));
    m_interface->sendPacket(reply->requestPacket());
    m_pendingReplies.insert(reply->packetId(), reply);
    reply->startWait();
    return reply;
}

RobotControllerReply *RobotController::getStatus()
{
    qCDebug(dcRobotController()) << "Getting status";
    RobotControllerReply *reply = createReply(RobotControllerPacket(RobotControllerPacket::CommandGetStatus, m_packetId++));
    m_interface->sendPacket(reply->requestPacket());
    m_pendingReplies.insert(reply->packetId(), reply);
    reply->startWait();
    return reply;
}

RobotControllerReply *RobotController::enableSteppers(bool enabled)
{
    RobotControllerReply *reply= createReply(RobotControllerPacket(RobotControllerPacket::CommandEnableSteppers, m_packetId++, QByteArray::fromHex(enabled ? "01" : "00")));
    m_interface->sendPacket(reply->requestPacket());
    m_pendingReplies.insert(reply->packetId(), reply);
    reply->startWait();
    return reply;
}

RobotControllerReply *RobotController::getMotorsInformation()
{
    RobotControllerReply *reply= createReply(RobotControllerPacket(RobotControllerPacket::CommandGetMotorInformation, m_packetId++));
    m_interface->sendPacket(reply->requestPacket());
    m_pendingReplies.insert(reply->packetId(), reply);
    reply->startWait();
    return reply;
}

void RobotController::onInterfaceAvailableChanged(bool available)
{
    if (available) {
        // Start initializing the robot
        setState(StateInitializing);

        // Wait 2 seconds for the ready signal, otherwise get the status

        qCDebug(dcRobotController()) << "Waiting for the firmware to become ready...";
        m_readyTimer.start();

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

void RobotController::onInterfacePacketReceived(const RobotControllerPacket &packet)
{
    switch (packet.type()) {
    case RobotControllerPacket::TypeNotification:
        processNotification(packet);
        break;
    case RobotControllerPacket::TypeResponse:
        processResponse(packet);
        break;
    default:
        qCWarning(dcRobotController()) << "Received unhandeld packet type:" << packet;
        break;
    }
}

void RobotController::setState(State state)
{
    if (m_state == state)
        return;

    qCInfo(dcRobotController()) << "State changed:" << state;
    m_state = state;

    if (m_state == StateReady)
        m_readyTimer.stop();

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

void RobotController::initControllerInformation()
{
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

        QString firmwareVersion = QString("%1.%2.%3")
                                      .arg(static_cast<quint8>(firmwareData.at(0)))
                                      .arg(static_cast<quint8>(firmwareData.at(1)))
                                      .arg(static_cast<quint8>(firmwareData.at(2)));

        if (m_firmwareVersion != firmwareVersion) {
            m_firmwareVersion = firmwareVersion;
            emit firmwareVersionChanged(m_firmwareVersion);
        }

        // Get status
        RobotControllerReply *statusReply = getStatus();
        connect(statusReply, &RobotControllerReply::finished, this, [this, statusReply](){
            if (statusReply->error() != RobotControllerReply::ErrorNoError) {
                qCWarning(dcRobotController()) << "Could not read firmware version. The reply finished with error" << statusReply->error();
                setState(StateError);
                return;
            }

            QByteArray statusData = statusReply->responsePacket().payload();
            if (statusData.length() != 2) {
                qCWarning(dcRobotController()) << "Could not read status. The response has an unexpected payload length" << statusData.length() << statusData.toHex();
                setState(StateError);
                return;
            }


            RobotControllerReply *getMotorInformationReply = getMotorsInformation();
            connect(getMotorInformationReply, &RobotControllerReply::finished, this, [this, getMotorInformationReply](){

                if (getMotorInformationReply->error() != RobotControllerReply::ErrorNoError) {
                    qCWarning(dcRobotController()) << "Could not get motor information. The reply finished with error" << getMotorInformationReply->error();
                    setState(StateError);
                    return;
                }

                QByteArray motorInformationData = getMotorInformationReply->responsePacket().payload();

                qCDebug(dcRobotController()) << "Motor information:" << motorInformationData.toHex();
                //00 00000000 00000000 000029ac 00 023c08bc07c2010100000108ca08cb000202ff04040105081e
                //00 00000000 00000000 0000c908 00 023d08ba07c2010100000108c808c9000202ff04040105081e
                //00 00000000 00000000 0000 29ac2644 3d08b807c2010100000108c608c7000202ff01040305081f
                //00 00003039 0000ddd5 4426ac29
                QDataStream stream(motorInformationData);
                stream.setByteOrder(QDataStream::BigEndian);
                stream.setFloatingPointPrecision(QDataStream::SinglePrecision); // 23 Bit

                quint8 runningValue;
                quint32 currentPosition;
                quint32 targetPosition;
                float speed;

                for (int i = 0; i < 3; i++) {
                    stream >> runningValue >> currentPosition >> targetPosition >> speed;
                    qCDebug(dcRobotController()) << "Motor" << i << "running:" << (runningValue == 0 ? false : true);
                    qCDebug(dcRobotController()) << "Motor" << i << "position:" << currentPosition;
                    qCDebug(dcRobotController()) << "Motor" << i << "target position:" << targetPosition;
                    qCDebug(dcRobotController()) << "Motor" << i << "speed:" << speed;
                }

                qCInfo(dcRobotController()) << "Initialized successfully. Firmware version" << m_firmwareVersion;
                setState(StateReady);
            });
        });
    });
}


void RobotController::processResponse(const RobotControllerPacket &responsePacket)
{
    if (m_pendingReplies.contains(responsePacket.packetId())) {
        RobotControllerReply *reply = m_pendingReplies.value(responsePacket.packetId());
        if (reply->requestPacket().command() != responsePacket.command()) {
            qCWarning(dcRobotController()) << "Received response packet with matching packet id, but from a different request command. Dischard response packet.";
            qCWarning(dcRobotController()) << "Request:" << reply->requestPacket();
            qCWarning(dcRobotController()) << "Response:" << responsePacket;
            return;
        }

        m_pendingReplies.remove(responsePacket.packetId());
        reply->m_responsePacket = responsePacket;
        reply->setFinished();
    }
}

void RobotController::processNotification(const RobotControllerPacket &notificationPacket)
{
    switch (notificationPacket.notification()) {
    case RobotControllerPacket::NotificationReady:
        qCDebug(dcRobotController()) << "Controller is ready. Start initializing information...";
        initControllerInformation();
        break;
    case RobotControllerPacket::NotificationDebugMessage:
        qCDebug(dcRobotController()) << "Debug message:" << QString::fromUtf8(notificationPacket.payload());
        break;
    default:
        break;
    }
}
