#include <Arduino.h>

#include "SerialApiServer.h"
#include "MotorController.h"

SerialApiServer *apiServer = nullptr;
MotorController *motorController = nullptr;

void setup()
{
    motorController = new MotorController();
    motorController->init();

    apiServer = new SerialApiServer(Serial, motorController);
    apiServer->init();
    
    apiServer->debug("Setup done");
}

void loop() 
{
    // Process API
    apiServer->process();
    motorController->process();
}