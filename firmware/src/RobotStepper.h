#include <Arduino.h>
#include "AccelStepper.h"

class RobotStepper : public AccelStepper 
{
public:
    RobotStepper(uint8_t interface=AccelStepper::FUNCTION, uint8_t pin1=0, uint8_t pin2=1, uint8_t pin3=2, uint8_t pin4=3, bool enable=false);
    ~RobotStepper();

    uint8_t process();

    void disableOutputs() override;
    void enableOutputs() override;

protected:
    void setOutputPins(uint8_t mask) override;
    void step4 (long step) override;
    void step8 (long step) override;

private:
    uint8_t m_lastValue = 0x00;
    uint8_t m_currentValue = 0x00;


};