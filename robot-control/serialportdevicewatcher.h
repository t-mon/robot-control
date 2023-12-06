#ifndef SERIALPORTDEVICEWATCHER_H
#define SERIALPORTDEVICEWATCHER_H

#include <QHash>
#include <QObject>
#include <QSocketNotifier>

#include <libudev.h>

class SerialPortDeviceWatcher : public QObject
{
    Q_OBJECT
public:
    typedef struct DeviceInfo {
        QString systemLocation;
        QString serialNumber;
        QString manufacturer;
        QString description;
    } DeviceInfo;

    explicit SerialPortDeviceWatcher(QObject *parent = nullptr);
    ~SerialPortDeviceWatcher();

    QList<DeviceInfo> deviceInfos() const;

signals:
    void deviceInfoAdded(const SerialPortDeviceWatcher::DeviceInfo &deviceInfo);
    void deviceInfoRemoved(const SerialPortDeviceWatcher::DeviceInfo &deviceInfo);

private:
    QSocketNotifier *m_notifier = nullptr;
    struct udev *m_udev = nullptr;
    struct udev_monitor *m_monitor = nullptr;

    QHash<QString, DeviceInfo> m_deviceInfos;

};

QDebug operator<<(QDebug debug, const SerialPortDeviceWatcher::DeviceInfo &deviceInfo);

#endif // SERIALPORTDEVICEWATCHER_H
