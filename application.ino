// This #include statement was automatically added by the Particle IDE.
#include "elevatorControl.h"

SYSTEM_THREAD(ENABLED)

Elevator elevator;

void setup() {

    //Initialize Serial ports
    Serial.begin(9600);
    Serial1.begin(9600);
    
    //Initialize GPIOs
    elevator.init();
}

void loop() {
    elevator.scan();
    elevator.evalFob();
}

