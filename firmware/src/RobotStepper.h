#ifndef ROBOTSTEPPER_H
#define ROBOTSTEPPER_H

#include <Arduino.h>
#include "AccelStepper.h"

class RobotStepper : public AccelStepper 
{
public:
    RobotStepper(int dirPin, int stepPin);
    ~RobotStepper();

private:

};

#endif // ROBOTSTEPPER