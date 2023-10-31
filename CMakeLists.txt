cmake_minimum_required(VERSION 3.16)

project(robot-arm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Quick QuickControls2)


qt_add_executable(approbot-arm
    main.cpp
)

qt_add_qml_module(approbot-arm
    URI robot-arm
    VERSION 1.0
    QML_FILES Main.qml
    QML_FILES ui/MainView.qml
    QML_FILES ui/HomeView.qml
    QML_FILES ui/SettingsView.qml
    SOURCES engine.h engine.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(approbot-arm PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.approbot-arm
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(approbot-arm
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
)

include(GNUInstallDirs)
install(TARGETS approbot-arm
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
