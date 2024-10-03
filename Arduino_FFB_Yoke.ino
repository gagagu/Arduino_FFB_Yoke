// Version 2.0.0 Beta
#include "src/Joystick.h"   // Joystick and FFB Library (many changes made) by https://github.com/jmriego/Fino
#include <Encoder.h>        // Encoder Library https://github.com/PaulStoffregen/Encoder
#include <EEPROM.h>         // https://docs.arduino.cc/learn/built-in-libraries/eeprom

#include "defines.h"

unsigned long nextJoystickMillis=0;   // count millis for next joystick update
unsigned long nextEffectsMillis=0;    // count millis for next Effect update
unsigned long currentMillis;          // millis for the current loop

bool blSerialDebug = false;           // serial debug mode
bool blCalibration =  true;                  // start flag
byte bSerialIndex=0;

int16_t forces[MEM_AXES] = {0, 0};      // stored forces
Gains gains[FFB_AXIS_COUNT];            // stored gain parameters
EffectParams effects[MEM_AXES];         // stored effect parameters

int16_t adjForceMax[MEM_AXES] = {0,0};  // stored max adjusted force
byte adjPwmMin[MEM_AXES] = {0,0};       // stored start pwm (motor power) on force != 0
byte adjPwmMax[MEM_AXES] = {0,0};       // stored max pwm (motor power) on max force

Joystick_ Joystick(                     // define Joystick parameters
  JOYSTICK_DEFAULT_REPORT_ID,           // ID defined in Joystick.h
  JOYSTICK_TYPE_JOYSTICK,               // type Joystick
  12, 1,                                // Button Count, Hat Switch Count
  true, true, false,                    // X, Y, Z
  false, false, false,                  // Rx, Ry, Rz
  false, false);                        // rudder, throttle

Encoder counterRoll(0, 1);                // init encoder library for roll ir sensor
Encoder counterPitch(3, 2);               // init encoder library for pitch ir sensor

typedef struct
{
  bool blError;
  bool blMotorInverted;
  bool blAxisInverted;
  int16_t iMin;
  int16_t iMax;
} AxisConfiguration;

AxisConfiguration ROLL_AxisConfiguration;
AxisConfiguration PITCH_AxisConfiguration;


// variables for Speed calculation
byte roll_speed = 0;
byte pitch_speed = 0;

// variables for button pin states
uint16_t iYokeButtonPinStates = 0;
uint16_t iSensorPinStates = 0;


// variables for calculation
unsigned long lastEffectsUpdate = 0;    // count millis for next effect calculation
int16_t lastX;                          // X value from last loop
int16_t lastY;                          // Y value from last loop
int16_t lastVelX;                       // Velocity X value from last loop
int16_t lastVelY;                       // Velocity y value from last loop
int16_t lastAccelX;                     // Acceleration X value from last loop
int16_t lastAccelY;                     // Acceleration X value from last loop

 bool isCalibrationPressed=false;

/********************************
     initial setup
*******************************/
void setup() {
  PITCH_AxisConfiguration.blMotorInverted=false;
  PITCH_AxisConfiguration.blAxisInverted=false;
  PITCH_AxisConfiguration.blError=false;
  PITCH_AxisConfiguration.iMin=0;
  PITCH_AxisConfiguration.iMax=0;
  
  ROLL_AxisConfiguration.iMin=0;
  ROLL_AxisConfiguration.iMax=0;
  ROLL_AxisConfiguration.blError=false;
  ROLL_AxisConfiguration.blMotorInverted=false;
  ROLL_AxisConfiguration.blAxisInverted=false;

  ArduinoSetup();                   // setup for Arduino itself (pins)
  SetupJoystick();                  // Joystick setup

  Serial.begin(SERIAL_BAUD);        // init serial
delay(3000);
  EnableMotors();                   // enable motors

} //setup

/******************************
      main loop
****************************/
void loop() {
 
  currentMillis = millis();                                           // number of milliseconds passed since the Arduino board began running the current program
 
  ReadMux();                                                          // Read values of buttons and end switch sensors (except yoke axes)
  
  if(blCalibration)
  {
    
    DoCalibration();                                               // Start with Calibration
    blCalibration=false;
  }
  else
  {
    if((iSensorPinStates & (1 << ADJ_CALIBRATION_BUTTON)) !=0){
      isCalibrationPressed=true;
    }else{
      if(isCalibrationPressed==true){
        isCalibrationPressed=false;
        blCalibration=true;
      }
    }

    if (currentMillis >= nextJoystickMillis) {                        // do not run more frequently than these many milliseconds, the window system needs time to process
     UpdateJoystickButtons();                                        // set Joystick buttons
     Joystick.sendState();                                           // send joystick values to system

     if (currentMillis >= nextEffectsMillis) {                       // we calculate condition forces every 100ms or more frequently if we get position updates
       UpdateEffects(true);                                          // update/calculate new effect paraeters
       nextEffectsMillis = currentMillis + 100;                      // set time for new effect loop
     } else {
       UpdateEffects(false);                                         // calculate forces without recalculating condition forces
       // this helps having smoother spring/damper/friction
       // if our update rate matches our input device
     } //nextEffectsMillis  || pos_updated

     PrepareMotors();                                                // move motors
     nextJoystickMillis = currentMillis + 20;                        // set time for new joystick loop

     SerialEvent();                                                  // check if serial event received
   } //nextJoystickMillis
  } // else
} // loop
