cmake_minimum_required(VERSION 3.16)

include(robot.cmake)

qt_add_qml_module(RobotModule
    URI RobotModule
    VERSION 1.0
    SOURCES uartinterface.h uartinterface.cpp
    SOURCES robotcontrollerreply.h robotcontrollerreply.cpp
    SOURCES robotcontrollerpacket.h robotcontrollerpacket.cpp
    SOURCES robotcontroller.h robotcontroller.cpp
)
