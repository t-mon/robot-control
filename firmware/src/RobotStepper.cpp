#include "RobotStepper.h"

RobotStepper::RobotStepper(int dirPin, int stepPin) :
    AccelStepper(AccelStepper::DRIVER, dirPin, stepPin)
{

}

RobotStepper::~RobotStepper()
{

}
