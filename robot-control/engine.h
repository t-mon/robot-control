#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QQmlEngine>

#include "robotcontroller.h"

Q_DECLARE_LOGGING_CATEGORY(dcEngine)

class SerialPortDeviceWatcher;

class Engine : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(RobotController *robotController READ robotController CONSTANT FINAL)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged FINAL)

public:
    explicit Engine(QObject *parent = nullptr);

    bool ready() const;

    RobotController *robotController() const;

signals:
    void readyChanged(bool ready);

private:
    RobotController *m_robotController = nullptr;
    SerialPortDeviceWatcher *m_serialPortDeviceWatcher = nullptr;

    bool m_ready = false;

    void evaluateReadyState();
};

#endif // ENGINE_H
