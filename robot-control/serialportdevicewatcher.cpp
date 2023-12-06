#include "serialportdevicewatcher.h"
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcRobotSerialInterface)

SerialPortDeviceWatcher::SerialPortDeviceWatcher(QObject *parent)
    : QObject{parent}
{
    m_udev = udev_new();
    if (!m_udev) {
        qCWarning(dcRobotSerialInterface()) << "Could not initialize udev";
        return;
    }

    // Read initially all tty devices
    struct udev_enumerate *enumerate = udev_enumerate_new(m_udev);
    if (!enumerate) {
        qCWarning(dcRobotSerialInterface()) << "Could not create udev enumerate for initial device reading.";
        udev_unref(m_udev);
        m_udev = nullptr;
        return;
    }

    // We are only interested in FTDI devices
    udev_enumerate_add_match_subsystem(enumerate, "tty");
    udev_enumerate_add_match_property(enumerate, "ID_VENDOR_ID", "0043"); // Arduino Uno
    udev_enumerate_add_match_property(enumerate, "ID_MODEL_ID", "2341");

    if (udev_enumerate_scan_devices(enumerate) < 0) {
        qCWarning(dcRobotSerialInterface()) << "Failed to scan devices from udev enumerate.";
        udev_enumerate_unref(enumerate);
        enumerate = nullptr;
        udev_unref(m_udev);
        m_udev = nullptr;
        return;
    }

    qCDebug(dcRobotSerialInterface()) << "Load initial list of available serial ports...";
    struct udev_list_entry *devices = nullptr;
    devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *dev_list_entry = nullptr;
    udev_list_entry_foreach(dev_list_entry, devices) {
        struct udev_device *device = nullptr;
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        device = udev_device_new_from_syspath(m_udev, path);

        // Print properties
        struct udev_list_entry *properties = udev_device_get_properties_list_entry(device);
        struct udev_list_entry *property_list_entry = nullptr;
        udev_list_entry_foreach(property_list_entry, properties) {
            qCDebug(dcRobotSerialInterface()) << " - Property" << udev_list_entry_get_name(property_list_entry) << udev_list_entry_get_value(property_list_entry);
        }

        QString devicePath = QString::fromLatin1(udev_device_get_property_value(device,"DEVNAME"));
        QString manufacturerString = QString::fromLatin1(udev_device_get_property_value(device,"ID_VENDOR_ENC"));
        QString descriptionString = QString::fromLatin1(udev_device_get_property_value(device,"ID_MODEL_ENC"));
        QString serialNumberString = QString::fromLatin1(udev_device_get_property_value(device, "ID_SERIAL_SHORT"));

        // Clean up this device since we have all information
        udev_device_unref(device);

        qCDebug(dcRobotSerialInterface()) << "[+]" << devicePath  << manufacturerString << descriptionString << serialNumberString;
        DeviceInfo info;
        info.systemLocation = devicePath;
        info.serialNumber = serialNumberString;
        m_deviceInfos.insert(info.systemLocation, info);
        emit deviceInfoAdded(info);
    }

    udev_enumerate_unref(enumerate);
    enumerate = nullptr;

    // Create udev monitor
    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (!m_monitor) {
        qCWarning(dcRobotSerialInterface()) << "Could not initialize udev monitor.";
        udev_unref(m_udev);
        m_udev = nullptr;
        return;
    }

    // Set monitor filter to tty subsystem
    if (udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "tty", nullptr) < 0) {
        qCWarning(dcRobotSerialInterface()) << "Could not set subsystem device type filter to tty.";
        udev_monitor_unref(m_monitor);
        m_monitor = nullptr;
        udev_unref(m_udev);
        m_udev = nullptr;
        return;
    }

    // Enable the monitor
    if (udev_monitor_enable_receiving(m_monitor) < 0) {
        qCWarning(dcRobotSerialInterface()) << "Could not enable udev monitor.";
        udev_monitor_unref(m_monitor);
        m_monitor = nullptr;
        udev_unref(m_udev);
        m_udev = nullptr;
        return;
    }

    // Create socket notifier for read
    int socketDescriptor = udev_monitor_get_fd(m_monitor);
    m_notifier = new QSocketNotifier(socketDescriptor, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, [this, socketDescriptor](int socket){

        if (socketDescriptor != socket) {
            qCWarning(dcRobotSerialInterface()) << "socket != socketdescriptor";
            return;
        }

        Q_UNUSED(socket)

        // Create udev device
        udev_device *device = udev_monitor_receive_device(m_monitor);
        if (!device) {
            qCWarning(dcRobotSerialInterface()) << "Got socket sotification but could not read device information.";
            return;
        }

        QString actionString = QString::fromLatin1(udev_device_get_action(device));
        QString devicePath = QString::fromLatin1(udev_device_get_property_value(device,"DEVNAME"));
        QString manufacturerString = QString::fromLatin1(udev_device_get_property_value(device,"ID_VENDOR_ENC"));
        QString descriptionString = QString::fromLatin1(udev_device_get_property_value(device,"ID_MODEL_ENC"));
        QString serialNumberString = QString::fromLatin1(udev_device_get_property_value(device, "ID_SERIAL_SHORT"));

        QString vendorIdString = QString::fromLatin1(udev_device_get_property_value(device, "ID_VENDOR_ID"));
        QString productIdString = QString::fromLatin1(udev_device_get_property_value(device, "ID_MODEL_ID"));

        // Clean udev device
        udev_device_unref(device);

        // Make sure we know the action
        if (actionString.isEmpty())
            return;

        //        // Make sure this is the right FTDI device
        //        if ((vendorIdString != "0403" && productIdString != "6001")  ) {
        //            qCDebug(dcRobotSerialInterface()) << actionString << "device. This is not the right device. Ignoring" << vendorIdString << productIdString;
        //            return;
        //        }

        DeviceInfo info;
        info.systemLocation = devicePath;
        info.serialNumber = serialNumberString;
        info.manufacturer = manufacturerString;
        info.description = descriptionString;

        if (actionString == "add") {
            qCDebug(dcRobotSerialInterface()) << "[+]" << devicePath << serialNumberString;
            if (!m_deviceInfos.contains(info.systemLocation)) {
                m_deviceInfos.insert(info.systemLocation, info);
                emit deviceInfoAdded(info);
            }
        }

        if (actionString == "remove") {
            qCDebug(dcRobotSerialInterface()) << "[-]" << devicePath << serialNumberString;

            if (m_deviceInfos.contains(info.systemLocation)) {
                m_deviceInfos.remove(info.systemLocation);
                emit deviceInfoRemoved(info);
            }
        }
    });

    m_notifier->setEnabled(true);

    qCDebug(dcRobotSerialInterface()) << "Arduino device watcher initialized successfully.";
    if (m_deviceInfos.isEmpty()) {
        qCDebug(dcRobotSerialInterface()) << "There are currently Arduino connected.";
    }
}

SerialPortDeviceWatcher::~SerialPortDeviceWatcher()
{

}

QList<SerialPortDeviceWatcher::DeviceInfo> SerialPortDeviceWatcher::deviceInfos() const
{
    return m_deviceInfos.values();
}

QDebug operator<<(QDebug debug, const SerialPortDeviceWatcher::DeviceInfo &deviceInfo)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "SerialPortDevice(";
    debug.nospace() << deviceInfo.systemLocation << ", ";
    debug.nospace() << deviceInfo.manufacturer << ", ";
    debug.nospace() << deviceInfo.serialNumber << ", ";
    if (!deviceInfo.description.isEmpty()) {
        debug.nospace() << deviceInfo.description << ")";
    } else {
        debug.nospace() << ")";
    }

    return debug;
}
