#include "spark_wiring_string.h"
#include "spark_wiring.h"
#include "spark_wiring_usartserial.h"
#include <NCD4Relay.h>

#define Left A0
#define Bottom A1
#define Right D2
#define Top D3
#define Center D4
#define gateOnePin A5     
#define gateTwoPin D5     
#define gateThreePin D6
#define firstmagPin A2      
#define secondmagPin A4
#define thirdmagPin D7

class Elevator{
    private:
        bool prevFirstMag, prevSecondMag, prevThirdMag;
        
        bool prevFirstGate, prevSecondGate, prevThirdGate;
        
        bool magFail;
        
        bool gates, gateOne, gateTwo, gateThree;

        bool firstMag, secondMag, thirdMag;
        
        bool threeFloorSystem;
        bool interLockEnabled;
        
        // New variables added
        bool fromThird, fromFirst;
        
        int previousPosition;
        int movingTo;       //0-Stop, 1-Moving to Floor 1, 2-Moving to Floor 2, 3-Moving to Floor 3 
        int previousFloor;   //1-Floor 1, 2-Floor 2, 3-Floor 3
        //void evalJumpers();
        
        //Fob variables
        bool left, right, center, top, bottom;
        bool prevLeft, prevRight, prevCenter, prevTop, prevBottom;
        
        String forwardConfirm = "(1)82";
        String reverseConfirm = "(2)83";
        String stopConfirm = "(3)84";
        String paramChangeConfirm = "(4)85";
        unsigned long runConfirmTimeout = 10;
        int runRetries = 10;
        
        char forwardStr[10] = "(10600)48";
        char reverseStr[10] = "(20600)49";
        char stopStr[6] = "(3)84";
        char deaccelStr[12] = "(4040010)::";     //(4040015):?        (4040020):;         (4040010)::     (4040025);0
        char accelStr[12] = "(4030030):;";
        
    public:
        int floorPosition;
        int previousMillis1 = 0;
        int light = 0;
        bool moving;
        bool goToFloor(int position);
        bool stop();
        void scan();
        void evalJumpers();
        void evalFob();
        void init();
        void serialCtrl();
};
