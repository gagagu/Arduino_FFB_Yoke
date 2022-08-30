// gerneral debug info (forces)
// change NODEBUG to DEBUG to activae debug output on serial
#define DEBUG

// Button Debug Info
#define NOBUTTONDEBUG

#ifdef _VARIANT_ARDUINO_DUE_X_
  #define Serial SerialUSB
#endif

// the digits mean Mmmmrrr (M=Major,m=minor,r=revision)
#define SKETCH_VERSION 3000001

#include "src/Joystick.h"
#include "config.h"


// -------------------------
// Various global variables
// -------------------------
unsigned long lastEffectsUpdate;
unsigned long nextJoystickMillis;
unsigned long nextEffectsMillis;

// --------------------------
// Joystick related variables
// --------------------------
#define JOYSTICK_minX -32768
#define JOYSTICK_maxX 32767
#define JOYSTICK_minY -32768
#define JOYSTICK_maxY 32767

#define MEM_ROLL  0
#define MEM_PITCH  1
#define MEM_AXES  2

bool is_connected = false;
bool forces_requested = false;
bool pos_updated = false;
//bool calibration_init = false;
///bool calibration_init_start = false;

int16_t pos[MEM_AXES] = {0, 0};
int lastX;
int lastY;
int lastVelX;
int lastVelY;
int lastAccelX;
int lastAccelY;

EffectParams effects[2];
int32_t forces[MEM_AXES] = {0, 0};

Joystick_ Joystick(
    JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
    12, 1, // Button Count, Hat Switch Count
    true, true, false, // X, Y, Z
    false, false, false, // Rx, Ry, Rz
    false, false); // rudder, throttle

void setup() {

    ArduinoSetup();
    setupJoystick();

    // setup communication
    #if defined(DEBUG) || defined(BUTTONDEBUG)
      Serial.begin(SERIAL_BAUD);
    #endif

    // setup timing and run them as soon as possible
    lastEffectsUpdate = 0;
    nextJoystickMillis = 0;
    nextEffectsMillis = 0;

    // start calibration
    //calibration_init_start =false;
    //calibration_init=false;
}

/******************************
 * // main loop
 * ***************************/
void loop(){
    unsigned long currentMillis;
    currentMillis = millis();
    
    // calibration mode button pressed?
    //if(!calibration_init){
      //calibration_init = CheckCalibrationActivation();
    //}

    // init mode activated?
    //if(calibration_init)
   // {
      // init calibration mode
     // if(!calibration_init_start)
      //{
       // calibration_init_start=true;
       // DisableMotors();    // disable motor movement
       // ResetPotiValues();  // delet old poti max min values
        
       // #ifdef DEBUG
       //   Serial.println("Calibration mode activated, waiting for Axes movement:");
       // #endif
     // }
      
      // LED Blinking to indicate calibration mode
      //if (currentMillis >= nextJoystickMillis) {
      //  LedSwitch();
      //  nextJoystickMillis = currentMillis + 10000;
     // }

      // read poti values and save it
    //  if(CalibrationRollPoti() && CalibrationPitchPoti())
    //  {
        // if finished than disable mode
     //   LedOn();
     //   calibration_init=false;
    //    calibration_init_start=false;
    //    #ifdef DEBUG
     //     Serial.println("Init mode ended.");
     //   #endif
     // }
    //} else { // normal mode
      // do not run more frequently than these many milliseconds
      if (currentMillis >= nextJoystickMillis) {
          UpdateJoystickPos();
          UpdateJoystickButtons();
          nextJoystickMillis = currentMillis + 2;
  
          // we calculate condition forces every 100ms or more frequently if we get position updates
          if (currentMillis >= nextEffectsMillis || pos_updated) {
              updateEffects(true);
              nextEffectsMillis = currentMillis + 100;
              pos_updated = false;
              
          } else {
              // calculate forces without recalculating condition forces
              // this helps having smoother spring/damper/friction
              // if our update rate matches our input device
              updateEffects(false);
          }
  
      }
      EnableMotors(); // call every time, find a better way.
      DriveMotors();
      
      #if defined(DEBUG) || defined(BUTTONDEBUG)     
        Serial.println("");
      #endif
    //}
}
