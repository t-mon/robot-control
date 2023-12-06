#include <Arduino.h>

#include "SerialInterface.h"
#include "MotorController.h"

SerialInterface *serialInterface = nullptr;
MotorController *motorController = nullptr;

void setup()
{
    motorController = new MotorController();
    motorController->init();

    serialInterface = new SerialInterface(Serial, motorController);
    serialInterface->init();
    
    serialInterface->debug("Setup done");
    serialInterface->sendNotification(SerialInterface::NotificationReady);
}

void loop() 
{
    // Process API
    serialInterface->process();
    motorController->process();
}