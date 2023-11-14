#include "MotorController.h"

MotorController::MotorController()
{

}

MotorController::~MotorController() 
{

}

boolean MotorController::stepperEnabled() const
{
    return m_stepperEnabled;
}

void MotorController::setStepperEnabled(boolean enabled)
{
    digitalWrite(stepperEnablePin, enabled ? LOW : HIGH);
}

RobotStepper *MotorController::stepper1() const
{
    return m_stepper1;
}

RobotStepper *MotorController::stepper2() const
{
    return m_stepper2;
}

RobotStepper *MotorController::stepper3() const
{
    return m_stepper3;
}

void MotorController::init()
{        
    // Enable stepper
    pinMode(stepperEnablePin, OUTPUT);
    setStepperEnabled(m_stepperEnabled);

    m_stepper1 = new RobotStepper(stepPinX, dirPinX);
    // m_stepper1->setMaxSpeed(2000);
    // m_stepper1->setAcceleration(200);
    // m_stepper1->setSpeed(500);
	// m_stepper1->moveTo(200);

    m_stepper2 = new RobotStepper(stepPinY, dirPinY);
    // m_stepper2->setMaxSpeed(2000);
    // m_stepper2->setAcceleration(300);
    // m_stepper2->setSpeed(500);
	// m_stepper2->moveTo(400);

    m_stepper3 = new RobotStepper(stepPinZ, dirPinZ);
}

void MotorController::process()
{
    // Change direction once the motor reaches target position
	if (m_stepper1->distanceToGo() == 0) 
		m_stepper1->moveTo(-m_stepper1->currentPosition());
    
    // Change direction once the motor reaches target position
	if (m_stepper2->distanceToGo() == 0) 
		m_stepper2->moveTo(-m_stepper2->currentPosition());
    
    // Change direction once the motor reaches target position
	if (m_stepper3->distanceToGo() == 0) 
		m_stepper3->moveTo(-m_stepper3->currentPosition());
    

    m_stepper1->run();
    m_stepper2->run();
    m_stepper3->run();
}