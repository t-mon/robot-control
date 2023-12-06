#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include "RobotStepper.h"

// CNC shield
const uint8_t stepperEnablePin = 8;

const int stepPinX = 2;
const int dirPinX = 5;

const int stepPinY = 3;
const int dirPinY = 6;

const int stepPinZ = 4;
const int dirPinZ = 7;


class MotorController 
{
public:
    MotorController();
    ~MotorController();

    boolean stepperEnabled() const;
    void setStepperEnabled(boolean enabled);

    RobotStepper *stepper1() const; // x
    RobotStepper *stepper2() const; // y
    RobotStepper *stepper3() const; // z

    void init();
    void process();

private:
    boolean m_stepperEnabled = true;
    RobotStepper *m_stepper1 = nullptr;
    RobotStepper *m_stepper2 = nullptr;
    RobotStepper *m_stepper3 = nullptr;


};

#endif // MOTORCONTROLLER_H