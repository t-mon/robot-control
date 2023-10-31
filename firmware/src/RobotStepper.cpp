#include "RobotStepper.h"

RobotStepper::RobotStepper(uint8_t interface, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, bool enable):
    AccelStepper(interface, pin1, pin2, pin3, pin4, enable)
{
    
}

RobotStepper::~RobotStepper()
{

}

uint8_t RobotStepper::process()
{
    // Perform steps, read value, return value
    run();
    return m_currentValue;
}

void RobotStepper::disableOutputs()
{
    m_lastValue = m_currentValue;
    m_currentValue = 0x00;
}

void RobotStepper::enableOutputs()
{
    m_currentValue = m_lastValue;
}

void RobotStepper::setOutputPins(uint8_t mask)
{
    m_currentValue = mask;
}

void RobotStepper::step4(long step)
{
    switch (step & 0x3) {
	case 0:
	    setOutputPins(0b1100);
	    break;
	case 1:
	    setOutputPins(0b0110);
	    break;
	case 2:
	    setOutputPins(0b0011);
	    break;
	case 3:
	    setOutputPins(0b1001);
	    break;
    }
}

void RobotStepper::step8(long step)
{
    switch (step & 0x7) {
	case 0:
	    setOutputPins(0b1000);
        break;
    case 1:
	    setOutputPins(0b1100);
        break;
	case 2:
	    setOutputPins(0b0100);
        break;
    case 3:
        setOutputPins(0b0110);
        break;
	case 4:
	    setOutputPins(0b0010);
        break;
    case 5:
	    setOutputPins(0b0011);
        break;
	case 6:
	    setOutputPins(0b0001);
        break;
    case 7:
	    setOutputPins(0b1001);
        break;
    }
}