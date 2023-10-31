#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "MCP23017.h"
#include "RobotStepper.h"

// Each motor controller can drive 4 steppers
// Stepper 1 = A 0-3,
// Stepper 2 = A 4-7
// Stepper 3 = B 0-3,
// Stepper 4 = B 4-7



class MotorController 
{
public:
    MotorController();
    ~MotorController();

    void setMCP(MCP23017 &mcp);
    void begin();

    RobotStepper *stepper1() const;
    RobotStepper *stepper2() const;

    void process();

private:
    MCP23017 *m_mcp = nullptr;

    RobotStepper *m_stepper1 = nullptr;
    RobotStepper *m_stepper2 = nullptr;
    uint8_t m_valueA = 0x00;

    RobotStepper *m_stepper3 = nullptr;
    RobotStepper *m_stepper4 = nullptr; 
    uint8_t m_valueB = 0x00;

};
