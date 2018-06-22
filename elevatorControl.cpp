#include "elevatorControl.h"

NCD4Relay relayController;

void Elevator::init(){
    pinMode(Left, INPUT);
    pinMode(Bottom, INPUT);
    pinMode(Right, INPUT);
    pinMode(Top, INPUT);
    pinMode(Center, INPUT);
    pinMode(firstswitchPin, INPUT_PULLDOWN);
    pinMode(secondswitchPin, INPUT_PULLDOWN);
    pinMode(thirdswitchPin, INPUT_PULLDOWN);
    pinMode(firstmagPin, INPUT_PULLDOWN);
    pinMode(secondmagPin, INPUT_PULLDOWN);
    pinMode(thirdmagPin, INPUT_PULLDOWN);
    pinMode(A6, INPUT_PULLDOWN);
    pinMode(A7, INPUT_PULLDOWN);
    relayController.setAddress(0,0,0);
    evalJumpers();
    scan();
}

bool Elevator::goToFloor(int position){
    scan();
    int retries = 0;
    unsigned long startTime;
    int dataLen;
    String responseString;
    
    switch(position){
        case 1:{
            //Go To First Floor
            if(moving || floorPosition == 1 || !gates || magFail){
                return false;
            }
            retry:
            Serial1.print(forwardStr);
            startTime = millis();
            while(Serial1.available() < 5 && millis() < runConfirmTimeout + startTime);
            if(Serial1.available() < 5){
                if(retries < runRetries){
                    retries++;
                    goto retry;
                }else{
                    return false;
                }
            }
            dataLen = Serial1.available();
            char responseData[dataLen+1];
            for(int i = 0; i < dataLen; i++){
                responseData[i] = Serial1.read();
            }
            responseString = String(responseData);
            if(!responseString.equalsIgnoreCase(forwardConfirm)){
                return false;
            }
            movingTo = 1;
            moving = true;
            return true;
        }
        break;
            
        case 2:{
            //Go To Second Floor
            if(moving || floorPosition == 2 || !gates || magFail){
                return false;
            }
            retry1:
            //Determine direction
            int direction = 0;
            if(previousFloor == 1){
                Serial1.print(reverseStr);
                direction = 1;
            }else{
                Serial1.print(forwardStr);
            }
            startTime = millis();
            while(Serial1.available() < 5 && millis() < runConfirmTimeout + startTime);
            if(Serial1.available() < 5){
                if(retries < runRetries){
                    retries++;
                    goto retry1;
                }else{
                    return false;
                }
            }
            dataLen = Serial1.available();
            char responseData1[dataLen+1];
            for(int i = 0; i < dataLen; i++){
                responseData1[i] = Serial1.read();
            }
            responseString = String(responseData1);
            if(direction == 1){
                if(!responseString.equalsIgnoreCase(reverseConfirm)){
                    return false;
                }
            }else{
                if(!responseString.equalsIgnoreCase(forwardConfirm)){
                    return false;
                }
            }
            
            movingTo = 2;
            moving = true;
            return true;
        }
        break;   
        case 3:{
            //Go To First Floor
            if(moving || floorPosition == 3 || !gates || magFail){
                return false;
            }
            retry2:
            Serial1.print(reverseStr);
            startTime = millis();
            while(Serial1.available() < 5 && millis() < runConfirmTimeout + startTime);
            if(Serial1.available() < 5){
                if(retries < runRetries){
                    retries++;
                    goto retry2;
                }else{
                    return false;
                }
            }
            dataLen = Serial1.available();
            char responseData2[dataLen+1];
            for(int i = 0; i < dataLen; i++){
                responseData2[i] = Serial1.read();
            }
            responseString = String(responseData2);
            if(!responseString.equalsIgnoreCase(reverseConfirm)){
                return false;
            }
            movingTo = 3;
            moving = true;
            return true;
        }
        break;
    }
    return false;
}

void Elevator::scan(){
    //Read Current Status of Gates at all floors
    gateOne = digitalRead(gateOnePin);
    gateTwo = digitalRead(gateTwoPin);
    gateThree = digitalRead(gateThreePin);
    
    //Read floor position sensors.
    firstMag = digitalRead(firstmagPin);
    secondMag = digitalRead(secondmagPin);
    thridMag = digitalRead(thirdmagPin);
    
    //Verify floor position sensors are working properly
    int combinedMags = firstMag + secondMag + thirdMag;
    if(threeFloorSystem && combinedMags < 2){
        magFail = true;
    }
    if(!threeFloorSystem && combinedMags < 1){
        magFail = true;
    }
    
    //Stop the lift if interlocks are enabled.
    if(interLockEnabled){
        if(!gateOne || !gateTwo || !gateThree){
            if(moving){
                stop();
            }
        }
    }
    //Check first floor sensor
    if(firstMag != prevFirstMag){
        if(firstMag == LOW){
            floorPosition = 1;
            previousPosition = 1;
            if(movingTo == 1 || (movingTo == 2 && previousFloor = 3)){
                stop();
                Serial.println("First Floor ON");
            }
        }
    }
    prevFirstMag = firstMag;
    
    //Check second floor sensor
    if(secondMag != prevsecondMag){
        if(secondMag == LOW){
            floorPosition = 2;
            previousPosition = 2;
            if(movingTo == 2){
                stop();
                Serial.println("Second Floor ON");
            }
        }
    }
    previousSecondMag = secondMag;
    
    //Check thrid floor sensor
    if(thirdMag != prevThirdMag){
        if(thirdMag == LOW){
            floorPosition = 3;
            previousPosition = 3;
            if(movingTo == 3 || (movingTo == 2 && previousFloor = 1)){
                stop();
                Serial.println("Third Floor ON");
            }
        }
    }
    prevThirdMag = thirdMag;
}

//Check these
void Elevator::evalJumpers(){
    // Floor Mode
  threeFloorSystem = digitalRead(A6);
  
  // Interlocks mode
  interLockEnabled = digitalRead(A7);
}

void Elevator::evalFob(){
    left = digitalRead(Left);
    right = digitalRead(Right);
    center = digitalRead(Center);
    bottom = digitalRead(Bottom);
    top = digitalRead(Top);
    
    if(left != prevLeft && left == HIGH){
        goToFloor(1);
    }
    prevLeft = left;
    
    if(right != prevRight && right == HIGH && threeFloorSystem){
        goToFloor(3);
    }
    prevRight = right;
    
    if(center != prevCenter && center == HIGH){
        stop();
    }
    prevCenter = center;
    
    if(bottom != prevBottom && bottom == HIGH){
        relayController.turnOnRelay(4);
    }
    prevBottom = bottom;
    
    if(top != prevTop && top == HIGH){
        goToFloor(2);
    }
    prevTop = top;
}

bool Elevator::stop(){
    int retries = 0;
    unsigned long startTime;
    int dataLen;
    String responseString;
    retry:
    
    Serial1.print(stopStr);
    startTime = millis();
    while(Serial1.available() < 5 && millis() < runConfirmTimeout + startTime);
    if(Serial1.available() < 5){
        if(retries < runRetries){
            retries++;
            goto retry;
        }else{
            return false;
        }
    }
    dataLen = Serial1.available();
    char responseData[dataLen+1];
    for(int i = 0; i < dataLen; i++){
        responseData[i] = Serial1.read();
    }
    responseString = String(responseData);
    if(!responseString.equalsIgnoreCase(stopConfirm)){
        return false;
    }
    return true;
}

