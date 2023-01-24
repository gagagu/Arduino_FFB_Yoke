
#define languageEN  // Special thank to Lollo for the inspiration and code

#include "src/Joystick.h"
#include "config.h"
#include <LiquidCrystal.h>
#include <Encoder.h> //https://github.com/PaulStoffregen/Encoder


/**** BEGIN DEBUG INFO **********************
  Info: Please use only one debub info simultaneously
  Enable: remove the "NO" letters
  Disable: add "NO" letter before
*************************/


#define NODEBUG       // gerneral debug info for Joystick, Please use the Arduino_FFB_Yoke_App for read out
// change to DEBUG to enable debugging
// change to NODEBUG to disable debugging

/**** END DEBUG INFO *********************/

#ifdef _VARIANT_ARDUINO_DUE_X_
#define Serial SerialUSB
#endif

/*****************************
  Joystick min max values (not Potentiometers)
****************************/
int32_t  JOYSTICK_minX= -256;
int32_t  JOYSTICK_maxX= 256;
int32_t  JOYSTICK_minY= -256;
int32_t  JOYSTICK_maxY= 256;

/*****************************
  Memory array positions for Effects
****************************/
#define MEM_ROLL  0
#define MEM_PITCH  1
#define MEM_AXES  2


/*****************************
  variables
****************************/
unsigned long nextJoystickMillis;   // count millis for next joystick update
unsigned long nextEffectsMillis;    // count millis for next Effect update
unsigned long currentMillis;        // millis for the current loop

int32_t forces[MEM_AXES] = {0, 0};  // stored forces


Joystick_ Joystick(                 // define Joystick parameters
  JOYSTICK_DEFAULT_REPORT_ID,       // ID defined in Joystick.h
  JOYSTICK_TYPE_JOYSTICK,           // type Joystick
  12, 1,                            // Button Count, Hat Switch Count
  true, true, false,                // X, Y, Z
  false, false, false,              // Rx, Ry, Rz
  false, false);                    // rudder, throttle

LiquidCrystal lcd(12, 7,A0,A1,A2,A3); // init library for lcd display
Encoder counterRoll(0, 1);            // init encoder library for roll ir sensor
Encoder counterPitch(3,2);            // init encoder library for pitch ir sensor

/********************************
     initial setup
*******************************/
void setup() {

  ArduinoSetup();                   // setup for Arduino itself (pins)
  setupJoystick();                  // Joystick setup

  // setup communication if needed
#if defined(DEBUG)
  Serial.begin(SERIAL_BAUD);
#endif

  nextJoystickMillis = 0;           // reset counters
  nextEffectsMillis = 0;            // reset counters

  lcd.begin(20 , 4);                // start lcd display
  LcdPrintIntro();                  // show intro to lcd display
  delay(3000);                      // wait
  EnableMotors();                   // enable motors
}

/******************************
      main loop
****************************/
void loop() {
  currentMillis = millis();                                           // number of milliseconds passed since the Arduino board began running the current program

  if (currentMillis >= nextJoystickMillis) {                          // do not run more frequently than these many milliseconds, the window system needs time to process
    ReadMux();                                                        // Read values from buttons and potis (except yoke axes)
    CheckCalibrationMode();                                           // check if calibration an do it
    UpdateJoystickButtons();                                          // evaluate joystick buttons and send it to system                                

    if (currentMillis >= nextEffectsMillis) {                         // we calculate condition forces every 100ms or more frequently if we get position updates
      updateEffects(true);                                            // update/calculate new effect paraeters
      nextEffectsMillis = currentMillis + 100;                        // set time for new effect loop
    } else {
      updateEffects(false);                                           // calculate forces without recalculating condition forces
      // this helps having smoother spring/damper/friction
      // if our update rate matches our input device
    } //nextEffectsMillis  || pos_updated

    DriveMotors();                                                    // move motors
    nextJoystickMillis = currentMillis + 2;                           // set time for new joystick loop

#ifdef DEBUG
    Serial.println("");
#endif
  } //nextJoystickMillis
} // loop
