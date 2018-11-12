#include "elevatorControl.h"

NCD4Relay relayController;

void Elevator::init(){
    fromThird = false, fromFirst = false, light = false;
    pinMode(Left, INPUT);
    pinMode(Bottom, INPUT);
    pinMode(Right, INPUT);
    pinMode(Top, INPUT);
    pinMode(Center, INPUT);
    pinMode(gateOnePin, INPUT_PULLDOWN);
    pinMode(gateTwoPin, INPUT_PULLDOWN);
    pinMode(gateThreePin, INPUT_PULLDOWN);
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
    
    // Added cont: loop so that it would not always return false and have a 
    // chance to output what it recieved from the VFD;
    
    switch(position){
        case 1:{
                
            //Go To First Floor
            if(moving || floorPosition == 1 || !gates || magFail){
                Serial.println("not moving");
                return false;
            }
            
            Serial1.print(forwardStr);
            retry:
            movingTo = 1;
            Serial.println("Going to first");
            startTime = millis();
            while(Serial1.available() < 1 && millis() < runConfirmTimeout + startTime);
            if(Serial1.available() < 1){
                if(retries < runRetries){
                    retries++;
                    goto retry;
                }else{
                    goto cont;
                }
            }
            cont:
            delay(20);   // Had to add this for it to receive the whole message, used to only receive "(". 
            // I thought this was strange becuase the other switch cases 2 and 3 dont need the delay.
            // Maybe something to do with the runConfirmTimeout or startTime?
            // Minimum delay amount is around 10.
            moving = true;
            dataLen = Serial1.available();
            char responseData[dataLen+1];
            for(int i = 0; i < dataLen; i++){
                responseData[i] = Serial1.read();
            }
            responseData[dataLen] = '\0';  // I had to add this in order for it to not print 8 more uneccesary characters
            // it seems the String() function wasnt finding the end, i think this did the trick.
            // after adding this i got the below statement to finally output "is true".
            responseString = String(responseData);
            Serial.print("Response str: ");
            Serial.println(responseString);
            if(!responseString.equalsIgnoreCase(forwardConfirm)){
                Serial.print("is false");
                return false;
            }else{
                Serial.print("is true");
                return true;
            }
        }
        break;
   
        case 2:{
            //Go To Second Floor
            if(moving || floorPosition == 2 || !gates || magFail){
                return false;
            }
            retry1:
            // Using fromthird and from first to determine direction if elevator
            // is stopped in between floors
            int direction = 0;
            if(previousFloor == 1 || fromFirst == true){
                Serial1.print(reverseStr);
                movingTo = 2;
                moving = true;
                fromFirst = false;
                direction = 1;
            }else if(previousFloor == 3 || fromThird == true){
                Serial1.print(forwardStr);
                movingTo = 2;
                moving = true;
                fromThird = false;
            }
            startTime = millis();
            while(Serial1.available() < 5 && millis() < runConfirmTimeout + startTime);
            if(Serial1.available() < 5){
                if(retries < runRetries){
                    retries++;
                    goto retry1;
                }else{
                    goto cont2;
                }
            }
            cont2:
            dataLen = Serial1.available();
            char responseData1[dataLen+1];
            for(int i = 0; i < dataLen; i++){
                responseData1[i] = Serial1.read();
            }
            responseData1[dataLen] = '\0';
            responseString = String(responseData1);
            Serial.print("Response str: ");
            Serial.println(responseString);
            if(direction == 1){
                if(!responseString.equalsIgnoreCase(reverseConfirm)){
                    return false;
                }
            }else{
                if(!responseString.equalsIgnoreCase(forwardConfirm)){
                    return false;
                }
                else{
                    return true;
                }
            }
        }
        break;   
        case 3:{
            //Go To First Floor
            if(moving || floorPosition == 3 || !gates || magFail){
                return false;
            }
        
            retry2:
            Serial1.print(reverseStr);
            movingTo = 3;
            moving = true;
            startTime = millis();
            while(Serial1.available() < 5 && millis() < runConfirmTimeout + startTime);
            if(Serial1.available() < 5){
                if(retries < runRetries){
                    retries++;
                    goto retry2;
                }else{
                    goto cont3;
                }
            }
            cont3:
            dataLen = Serial1.available();
            char responseData2[dataLen+1];
            for(int i = 0; i < dataLen; i++){
                responseData2[i] = Serial1.read();
            }
            responseData2[dataLen] = '\0';
            responseString = String(responseData2);
            Serial.print("Response str: ");
            Serial.println(responseString);
            if(!responseString.equalsIgnoreCase(reverseConfirm)){
                return false;
            }else{
                return true;
            }
        }
        break;
    }
    Serial.println("GoTo function failed");
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
    thirdMag = digitalRead(thirdmagPin);
    
    if(interLockEnabled){
        gates = (gateOne && gateTwo && gateThree);
    }else{
        gates = true;
    }
    // Status messages for testing
  /*  
  if (millis() - previousMillis1 > 3000) {
    previousMillis1 = millis();
    Serial.print(gateOne);
    Serial.print(gateTwo);
    Serial.print(gateThree);
    Serial.print(firstMag);
    Serial.print(secondMag);
    Serial.print(thirdMag);
    Serial.print("  ");
    Serial.print(gates);
    Serial.print("   Moving to: ");
    Serial.print(movingTo);
    Serial.print("   Flr Pos: ");
    Serial.print(floorPosition);
    Serial.print("   Prev Flr: ");
    Serial.print(previousFloor);
    Serial.print("   Moving Status: ");
    Serial.println(moving);
    }
    */
    
    
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
        if(gates == 0){
            if(moving){
                stop();
                Serial.print("STOPPED INTERLOCKED ENABLED:  ");
                Serial.print(gates);
                Serial.print(gateOne);
                Serial.print(gateTwo);
                Serial.println(gateThree);
                
            }
        }
    }
    //Check first floor sensor
    if(firstMag != prevFirstMag){
        if(firstMag == LOW){
            floorPosition = 1;
            previousFloor = 1;
            if(movingTo == 1 || (movingTo == 2 && previousFloor == 3)){
                Serial.println("STOPPED ARRIVED FIRST FLOOR");
                stop();
                
            }
            Serial.println("First Floor ON");
        }
    }
    prevFirstMag = firstMag;
    
    //Check second floor sensor
    if(secondMag != prevSecondMag){
        if(secondMag == LOW){
            floorPosition = 2;
            previousFloor = 2;
            if(movingTo == 2){
                Serial.println("STOPPED ARRIVED SECOND FLOOR");
                stop();
                
            }
            Serial.println("Second Floor ON");
        }
    }
    prevSecondMag = secondMag;
    
    //Check thrid floor sensor
    if(thirdMag != prevThirdMag){
        if(thirdMag == LOW){
            floorPosition = 3;
            previousFloor = 3;
            if(movingTo == 3 || (movingTo == 2 && previousFloor == 1)){
                Serial.println("STOPPED ARRIVED THIRD FLOOR");
                stop();
                
            }
            Serial.println("Third Floor ON");
        }
    }
    prevThirdMag = thirdMag;
    
}

void Elevator::evalJumpers(){
    // Floor Mode
  threeFloorSystem = digitalRead(A6);
  if(threeFloorSystem == HIGH) {
        Serial.println("Three Floor System Enabled");
  }else{
        Serial.println("Two Floor System Enabled");
  }
  
  // Interlocks mode
  interLockEnabled = digitalRead(A7);
  if(interLockEnabled == HIGH) {
        Serial.println("Interlocks Enabled");
  }else{
        Serial.println("Interlocks Disabled");
  }
}

void Elevator::evalFob(){
    left = digitalRead(Left);
    right = digitalRead(Right);
    center = digitalRead(Center);
    bottom = digitalRead(Bottom);
    top = digitalRead(Top);
    
    if(left != prevLeft && left == HIGH){
        //Serial.println("First Floor Button HIGH");
        goToFloor(1);
    }
    prevLeft = left;
    
    if(right != prevRight && right == HIGH && threeFloorSystem){
        Serial.println("Third Floor Button HIGH");
        goToFloor(3);
    }
    prevRight = right;
    
    if(center != prevCenter && center == HIGH){
        Serial.println("Stop Button HIGH");
        stop();
        
    }
    prevCenter = center;
    
    if(bottom != prevBottom || blelight == true){
        if(bottom == HIGH || blelight == true){
        if(light % 2 == 0){
            relayController.turnOnRelay(4);
            Serial.println("Light ON");
        }else{
            relayController.turnOffRelay(4);
            Serial.println("Light OFF");
        }
        light++;
        blelight = false;
        }
    }
    prevBottom = bottom;
    
    if(top != prevTop && top == HIGH){
        Serial.println("Second Floor Button HIGH");
        goToFloor(2);
    }
    prevTop = top;
}

bool Elevator::stop(){
    bool secondFloor = digitalRead(secondmagPin);
    if(secondFloor == 1 && movingTo == 1 && previousFloor == 2){
        fromFirst = true;
    }
    if(secondFloor == 1 && movingTo == 3 && previousFloor == 2){
        fromThird = true;
    }
    int retries = 0;
    unsigned long startTime;
    int dataLen;
    String responseString;
    retry:
    
    Serial1.print(stopStr);
    Serial.println("STOPPED");
    moving = false;
    movingTo = 0;
    startTime = millis();
    while(Serial1.available() < 5 && millis() < runConfirmTimeout + startTime);
    if(Serial1.available() < 5){
        if(retries < runRetries){
            retries++;
            goto retry;
        }else{
            goto cont4;
        }
    }
    cont4:
    dataLen = Serial1.available();
    char responseData4[dataLen+1];
    for(int i = 0; i < dataLen; i++){
        responseData4[i] = Serial1.read();
    }
    responseData4[dataLen] = '\0';
    responseString = String(responseData4);
    Serial.print("Response str: ");
    Serial.println(responseString);
    if(!responseString.equalsIgnoreCase(stopConfirm)){
        return false;
    }
    
    return true;
}

void Elevator::serialFlush(){
    while(Serial1.available() > 0) {
        char t = Serial1.read();
    }
}   

void Elevator::serialCtrl(){
    int input = 0;
    if (Serial.available() > 0) {
        input = Serial.read();
        if(input == 'z'){
            if(threeFloorSystem == HIGH) {
                Serial.println("Three Floor System Enabled");
            }else{
                Serial.println("Two Floor System Enabled");
            }
  
            if(interLockEnabled == HIGH) {
                Serial.println("Interlocks Enabled");
            }else{
                Serial.println("Interlocks Disabled");
            }
        }
        if(input == 's'){
            serialFlush();
        }
        if(input == 'b'){
            Serial.println(bleinput);
        }
    }
}

void Elevator::bleCtrl(){
    char bletest[20] = { 0 };
    if(bleinput[0] == '1'){
        if(bleinput[1] == '1'){
            goToFloor(1);
        }
        if(bleinput[1] == '2'){
            goToFloor(2);
        }
        if(bleinput[1] == '3'){
            goToFloor(3);
        }
        if(bleinput[1] == '4'){
            stop();
        }
        if(bleinput[1] == '5'){
            blelight = true;
        }
        for(int i = 0; i < 15; i++){
            Serial.print(bleinput[i]);
            bleinput[i] = 0;
        }
        
    }else if(bleinput[0] == '0' && bleinput[1] == '0'){
            bleoutput[0] = '0';
            bleoutput[1] = '0';
            bleoutput[2] = firstMag;
            bleoutput[3] = secondMag;
            bleoutput[4] = thirdMag;
            sendBLE = true;
            Serial.println();
    }
    else if(bleinput[0] == '0' && bleinput[1] == '1'){
            bleoutput[0] = '0';
            bleoutput[1] = '1';
            bleoutput[2] = !firstMag + 48;
            bleoutput[3] = gateOne + 48;
            bleoutput[4] = !secondMag + 48;
            bleoutput[5] = gateTwo + 48;
            bleoutput[6] = !thirdMag + 48;
            bleoutput[7] = gateThree + 48;
            sendBLE = true;
            Serial.println();
            for(int i = 0; i < 15; i++){
                Serial.print(bleinput[i]);
                bleinput[i] = 0;
            }
    }else if(bleinput[0] == '0' && bleinput[1] == '2'){
        Serial1.print("(0)81");
        delay(50);
        if(Serial1.available()){
            for(int i = 0; i < 20; i++){
                bletest[i] = Serial1.read();
            }
        }
        memset(bleoutput, 0x00, 15);
        for(int i = 0; i < 14; i++){
            bleoutput[i] = bletest[i + 2];
        }
        Serial.println();
        Serial.println(bleoutput);
        Serial.println(bletest);
        sendBLE = true;
    }
    
}
