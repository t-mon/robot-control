#include <Arduino.h>
#include <Wire.h> 

#include "MotorController.h"

#include "MCP23017.h"

#define CONTROLLER_1_MCP23017_ADDR 0x20

#ifdef __ARDUINO__
#define STATUS_LED LED_BUILTIN
#define I2C_SDA SDA
#define I2C_SCL SCL
#define INIT_SEQUENCE_ENABLED
#endif

#ifdef __ESP32DEV__
#define STATUS_LED 2
#define I2C_SDA 21
#define I2C_SCL 22
#define INIT_SEQUENCE_ENABLED
#endif

MotorController controller = MotorController();

void setup() {

#ifdef INIT_SEQUENCE_ENABLED
    // Init sequence
    pinMode(STATUS_LED, OUTPUT);

    digitalWrite(STATUS_LED, HIGH);
    delay(500);
    digitalWrite(STATUS_LED, LOW);
    delay(500);
    digitalWrite(STATUS_LED, HIGH);
    delay(500);
    digitalWrite(STATUS_LED, LOW);
    delay(500);
    digitalWrite(STATUS_LED, HIGH);
#endif

    // Init debug serial
    Serial.begin(115200);

    // Init I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    bool detectedController = false;

    Serial.print("I2C scanner. Scanning ...\n");
    for (byte i = 1; i < 120; i++) {
        Wire.beginTransmission (i);
        if (Wire.endTransmission () == 0) {
            Serial.print("Detected address: ");
            Serial.print(i, DEC);
            Serial.print(" (0x");
            Serial.print(i, HEX);
            Serial.println(")");
            if (i == CONTROLLER_1_MCP23017_ADDR) {
                Serial.print("Found controller on I2C port 0x");
                Serial.print(i, HEX);
                Serial.println("");
                detectedController = true;
            }
        }
    }

    // Setup controller 1 (motors 1-4) on MCP 0x20
    if (detectedController) {
        Serial.println("Setting up controller");
        MCP23017 mcp = MCP23017(CONTROLLER_1_MCP23017_ADDR, Wire);
        controller.setMCP(mcp);
        controller.begin();

        controller.stepper1()->setMaxSpeed(150);
        controller.stepper1()->setAcceleration(200);
        controller.stepper1()->moveTo(600);

        controller.stepper2()->setMaxSpeed(50);
        controller.stepper2()->setAcceleration(100);
        // controller.stepper2()->moveTo(300);

    }

    Serial.println("Setup done");
}

void loop() {
    // Read serial port data


    // Process steppers 1-4
    controller.process();

}