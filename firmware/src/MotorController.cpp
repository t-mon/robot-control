#include "MotorController.h"

MotorController::MotorController() 
{

}

MotorController::~MotorController() 
{

}

void MotorController::setMCP(MCP23017 &mcp)
{
    m_mcp = &mcp;
}

void MotorController::begin()
{
    // We can control 4 motors with one mcp.
    Serial.println("Init MCP32017");
    m_mcp->init();
    m_mcp->portMode(MCP23017Port::A, 0); // Output
    m_mcp->portMode(MCP23017Port::B, 0); // Output
    m_mcp->writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A 
    m_mcp->writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B

    Serial.println("Creating stepper 1");
    m_stepper1 = new RobotStepper(RobotStepper::HALF4WIRE, 0, 0, 0, 0, true);

    Serial.println("Creating stepper 2");
    m_stepper2 = new RobotStepper(RobotStepper::HALF4WIRE, 0, 0, 0, 0, true);

    Serial.println("Controller initialized");
}

RobotStepper *MotorController::stepper1() const
{
    return m_stepper1;
}

RobotStepper *MotorController::stepper2() const
{
    return m_stepper2;
}

void MotorController::process()
{

    uint8_t value1 = m_stepper1->process();
    uint8_t value2 = m_stepper2->process();

    uint8_t valueA = value1 | (value2 << 4);
    if (valueA != m_valueA) {
        m_valueA = valueA;
        m_mcp->writePort(MCP23017Port::A, m_valueA);

    //     Serial.print("Steppers: 0x");
    //     Serial.print(value1, HEX);
    //     Serial.print(" to go: ");
    //     Serial.print(m_stepper1->distanceToGo());
    //     Serial.print(" | 0x");
    //     Serial.print(value1, HEX);
    //     Serial.print(m_stepper2->distanceToGo());        
    //     Serial.print(value2, HEX);
    //     Serial.print(" to go: ");
    //     Serial.print(m_stepper2->distanceToGo());
    //     Serial.print("\n");

    //     if (m_stepper1->distanceToGo() == 0) {
    //         Serial.print("Stepper 1: Movement done.");
    //         m_stepper1->disableOutputs();
    //     }

    //     if (m_stepper2->distanceToGo() == 0) {
    //         Serial.print("Stepper 2: Movement done.");
    //         m_stepper2->disableOutputs();
    //     }
    }
}