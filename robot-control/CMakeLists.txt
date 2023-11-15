cmake_minimum_required(VERSION 3.16)

include(../RobotControlInclude.cmake)

find_package(Qt6 6.2 COMPONENTS Core Gui Quick QuickControls2 SerialPort REQUIRED)
qt_standard_project_setup(REQUIRES 6.6)

qt_add_executable(robot-control main.cpp)

qt_add_qml_module(RobotControl
    URI RobotControl
    QML_FILES Main.qml
    QML_FILES HomeView.qml
    QML_FILES MainView.qml
    QML_FILES SettingsView.qml
    SOURCES application.h application.cpp
)

target_link_libraries(robot-control PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::SerialPort
)

set_target_properties(robot-control PROPERTIES
    # MACOSX_BUNDLE_GUI_IDENTIFIER com.example.approbot-arm
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
