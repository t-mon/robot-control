cmake_minimum_required(VERSION 3.16)

include(../RobotControlInclude.cmake)

find_package(Qt6 6.6 COMPONENTS Core Gui Quick QuickControls2 SerialPort REQUIRED)

pkg_check_modules(udev REQUIRED)

qt_standard_project_setup(REQUIRES 6.6)

qt_add_executable(robot-control main.cpp)

qt_add_qml_module(robot-control
    URI RobotControl
    VERSION 1.0
    QML_FILES ui/Main.qml
    QML_FILES ui/HomeView.qml
    QML_FILES ui/MainView.qml
    QML_FILES ui/SettingsView.qml
    QML_FILES ui/components/RobotStatusBar.qml
    SOURCES application.h application.cpp
    SOURCES engine.h engine.cpp
    SOURCES robotcontrollerreply.h robotcontrollerreply.cpp
    SOURCES robotcontrollerpacket.h robotcontrollerpacket.cpp
    SOURCES robotcontroller.h robotcontroller.cpp
    SOURCES robotserialinterface.h robotserialinterface.cpp
    SOURCES serialportdevicewatcher.h serialportdevicewatcher.cpp
)

set_target_properties(robot-control PROPERTIES
    # MACOSX_BUNDLE_GUI_IDENTIFIER com.example.approbot-arm
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(robot-control PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::SerialPort
    udev
)

include(GNUInstallDirs)
install(TARGETS robot-control
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

