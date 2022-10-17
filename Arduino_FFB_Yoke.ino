
#include "src/Joystick.h"
#include "config.h"

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
#define JOYSTICK_minX -32768
#define JOYSTICK_maxX 32767
#define JOYSTICK_minY -32768
#define JOYSTICK_maxY 32767

/*****************************
  Memory array positions for Effects
****************************/
#define MEM_ROLL  0
#define MEM_PITCH  1
#define MEM_AXES  2

/*****************************
  variables
****************************/
unsigned long lastEffectsUpdate;    // count millis for next effect calculation
unsigned long nextJoystickMillis;   // count millis for next joystick update
unsigned long nextEffectsMillis;    // count millis sfor next Effect update
unsigned long currentMillis;        // millis for the current loop

bool pos_updated = false;           // indicate position updates
int16_t pos[MEM_AXES] = {0, 0};     // sored joystick positions (not Potentiometers values, mapped values)
int lastX;                          // X value from last loop
int lastY;                          // Y value from last loop
int lastVelX;                       // Velocity X value from last loop
int lastVelY;                       // Velocity y value from last loop
int lastAccelX;                     // Acceleration X value from last loop
int lastAccelY;                     // Acceleration X value from last loop

EffectParams effects[2];            // stored effect parameters
int32_t forces[MEM_AXES] = {0, 0};  // stored forces

Joystick_ Joystick(                 // define Joystick parameters
  JOYSTICK_DEFAULT_REPORT_ID,       // ID defined in Joystick.h
  JOYSTICK_TYPE_JOYSTICK,           // type Joystick
  12, 1,                            // Button Count, Hat Switch Count
  true, true, false,                // X, Y, Z
  false, false, false,              // Rx, Ry, Rz
  false, false);                    // rudder, throttle

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

  lastEffectsUpdate = 0;            // reset counters
  nextJoystickMillis = 0;           // reset counters
  nextEffectsMillis = 0;            // reset counters
}

/******************************
      main loop
****************************/
void loop() {
  currentMillis = millis();                                             // number of milliseconds passed since the Arduino board began running the current program
  SetShiftRegister();                                                 // update LEDs through shift register
  UpdateAdjustmentsValues();                                          // read adjustments like force,pwm potentiometers and calibration button
  UpdateJoystickButtons();                                            // read Joystick Buttons and sent it to system
    
  if (currentMillis >= nextJoystickMillis) {                            // do not run more frequently than these many milliseconds, the window system needs time to process
    if (CheckCalibrationMode()) {                                       // ist calibration mode started?
      DisableMotors();                                                  // disable motors if enable
      UpdateCalibration();                                              // calibrate
    } else {
        UpdateJoystickPos();                                              // read Joystick Potentiometers and send it to system

        if (currentMillis >= nextEffectsMillis || pos_updated) {          // we calculate condition forces every 100ms or more frequently if we get position updates
          updateEffects(true);                                            // update/calculate new effect paraeters
          nextEffectsMillis = currentMillis + 100;                        // set time for new effect loop
          pos_updated = false;
        } else {
          updateEffects(false);                                           // calculate forces without recalculating condition forces
          // this helps having smoother spring/damper/friction
          // if our update rate matches our input device
        } //nextEffectsMillis  || pos_updated

        EnableMotors();                                                   // Enable Motor if disabled
        DriveMotors();                                                    // move motors
      nextJoystickMillis = currentMillis + 2;                           // set time for new joystick loop
    }
    #ifdef DEBUG
      Serial.println("");
    #endif    
  } //nextJoystickMillis
} // loop
