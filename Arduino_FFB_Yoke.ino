
// Version 1.3.0

#define LANGUAGE_EN         // Special thank to Lollo for the inspiration and code
#define USE_CENTER_SENSOR   // comment out when not using center sensor

#define SERIAL_BAUD 115200  // Communication Speed


#include "src/Joystick.h"   // Joystick and FFB Library (many changes made) by https://github.com/jmriego/Fino
#include <LiquidCrystal.h>  // https://www.arduino.cc/reference/en/libraries/liquidcrystal/
#include <Encoder.h>        // Encoder Library https://github.com/PaulStoffregen/Encoder
#include <EEPROM.h>         // https://docs.arduino.cc/learn/built-in-libraries/eeprom

#ifdef _VARIANT_ARDUINO_DUE_X_  // Due uses SerialUSB for Communication
  #define Serial SerialUSB
#endif

/*****************************
  Memory array positions for Effects
****************************/
#define MEM_ROLL  0
#define MEM_PITCH  1
#define MEM_AXES  2

/*****************************
  Joystick min max default values (uncalibrated)
****************************/
int16_t  JOYSTICK_minX = -256;
int16_t  JOYSTICK_maxX = 256;
int16_t  JOYSTICK_minY = -256;
int16_t  JOYSTICK_maxY = 256;

/*****************************
  variables
****************************/
unsigned long nextJoystickMillis=0;   // count millis for next joystick update
unsigned long nextEffectsMillis=0;    // count millis for next Effect update
unsigned long currentMillis;          // millis for the current loop

bool blSerialDebug = false;           // serial debug mode
bool blStart = true;                  // start flag
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

LiquidCrystal lcd(12, 7, A0, A1, A2, A3); // init library for lcd display
Encoder counterRoll(0, 1);                // init encoder library for roll ir sensor
Encoder counterPitch(3, 2);               // init encoder library for pitch ir sensor

/********************************
     initial setup
*******************************/
void setup() {

  ArduinoSetup();                   // setup for Arduino itself (pins)
  setupJoystick();                  // Joystick setup

  Serial.begin(SERIAL_BAUD);        // init serial

  lcd.begin(20 , 4);                // start lcd display
  LcdPrintIntro();                  // show intro to lcd display
  delay(3000);                      // wait
  
  LcdPrintAdjustmendValues();       // show adjusted parameters
 
  EnableMotors();                   // enable motors
} //setup

/******************************
      main loop
****************************/
void loop() {
  currentMillis = millis();                                           // number of milliseconds passed since the Arduino board began running the current program
    
  ReadMux();                                                          // Read values of buttons and end switch sensors (except yoke axes)
  if (currentMillis >= nextJoystickMillis) {                          // do not run more frequently than these many milliseconds, the window system needs time to process
    CheckCalibrationMode();                                           // check if calibration button was pressed an do it
    UpdateJoystickButtons();                                          // set Joystick buttons
    Joystick.sendState();                                             // send joystick values to system

    if (currentMillis >= nextEffectsMillis) {                         // we calculate condition forces every 100ms or more frequently if we get position updates
      updateEffects(true);                                            // update/calculate new effect paraeters
      nextEffectsMillis = currentMillis + 100;                        // set time for new effect loop
    } else {
      updateEffects(false);                                           // calculate forces without recalculating condition forces
      // this helps having smoother spring/damper/friction
      // if our update rate matches our input device
    } //nextEffectsMillis  || pos_updated

    DriveMotors();                                                     // move motors
    nextJoystickMillis = currentMillis + 20;                           // set time for new joystick loop

    SerialEvent();                                                     // check if serial event received
  } //nextJoystickMillis
} // loop
