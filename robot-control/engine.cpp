#include "engine.h"
#include "serialportdevicewatcher.h"

Q_LOGGING_CATEGORY(dcEngine, "Engine")

Engine::Engine(QObject *parent)
    : QObject{parent}
{
    m_robotController = new RobotController(this);
    connect(m_robotController, &RobotController::stateChanged, this, [this](RobotController::State state){
        qCDebug(dcEngine()) << "Robot controller state changed" << state;
        evaluateReadyState();
    });

    m_serialPortDeviceWatcher = new SerialPortDeviceWatcher(this);
    connect(m_serialPortDeviceWatcher, &SerialPortDeviceWatcher::deviceInfoAdded, this, [this](const SerialPortDeviceWatcher::DeviceInfo &deviceInfo){
        qCInfo(dcRobotSerialInterface()) << "Device plugged in" << deviceInfo;
        foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
            if (serialPortInfo.systemLocation() == deviceInfo.systemLocation) {
                m_robotController->interface()->setSerialPortInfo(serialPortInfo);
            }
        }
    });

    m_robotController->interface()->enable();
}

bool Engine::ready() const
{
    return m_ready;
}

RobotController *Engine::robotController() const
{
    return m_robotController;
}

void Engine::evaluateReadyState()
{
    bool ready = m_robotController->state() == RobotController::StateReady;
    if (m_ready == ready)
        return;

    qCInfo(dcEngine()) << "Engine is" << (ready ? "now ready" : "not ready any more");
    m_ready = ready;
    emit readyChanged(m_ready);
}
