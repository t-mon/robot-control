cmake_minimum_required(VERSION 3.16)

include(RobotControlInclude.cmake)

project(robot-control
    VERSION 0.1.0
    DESCRIPTION "The control center for the robot arm"
    LANGUAGES CXX)

set(PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR})

message("Building robot-arm version = ${CMAKE_PROJECT_VERSION}")

add_compile_definitions(VERSION_STRING=\"${CMAKE_PROJECT_VERSION}\")
add_compile_definitions(PROJECT_NAME=\"${CMAKE_PROJECT_NAME}\")

add_subdirectory(robot-control)
