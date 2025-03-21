/*
  Version 2.0.0
*/

/* 
 Created by A.Eckers aka Gagagu
 http://www.gagagu.de
 https://github.com/gagagu/Arduino_FFB_Yoke
 https://www.youtube.com/@gagagu01
*/

/*
  This repository contains code for Arduino projects. 
  The code is provided "as is," without warranty of any kind, either express or implied, 
  including but not limited to the warranties of merchantability, 
  fitness for a particular purpose, or non-infringement. 
  The author(s) make no representations or warranties about the accuracy or completeness of 
  the code or its suitability for your specific use case.

  By using this code, you acknowledge and agree that you are solely responsible for any 
  consequences that may arise from its use. 

  For DIY projects involving electronic and electromechanical moving parts, caution is essential. 
  Ensure that you take the appropriate safety precautions, particularly when working with electricity. 
  Only work with devices if you understand their functionality and potential risks, and always wear 
  appropriate protective equipment. 
  Make sure you are working in a safe, well-lit environment, and that all components are properly installed and secured to avoid injury or damage.

  Special caution is required when building a force feedback device. Unexpected or sudden movements may occur, 
  which could lead to damage to people or other objects. 
  Ensure that all mechanical parts are securely mounted and that the work area is free of obstacles.
  
  By using this project, you acknowledge and agree that you are solely responsible for any consequences that may arise from its use. 
  The author(s) will not be held liable for any damages, injuries, or issues arising from the use of the project, 
  including but not limited to malfunctioning hardware, electrical damage, personal injury, or damage caused by 
  unintended movements of the force feedback device. The responsibility for proper handling, installation, 
  and use of the devices and components lies with the user.
  
  Use at your own risk.
*/

/*
  This is the MAIN FILE and has to be open in an Directory called "Arduino_FFB_Yoke". Otherwise the building of code will not work.
  This version is usable for Hardware v1.3 with Arduino Micro (original) and Hardware v2.0 with Arduino pro Micro
  See the defines.h for pin and cpu definitions definitions
*/
/*************************
  Includes
**************************/
#include "src/Joystick.h"  // Joystick and FFB Library (many changes made) by https://github.com/jmriego/Fino
#include <Encoder.h>       // Encoder Library https://github.com/PaulStoffregen/Encoder
#include <EEPROM.h>        // https://docs.arduino.cc/learn/built-in-libraries/eeprom

#include "defines.h"
#include "Multiplexer.h"
#include "AxisCalibration.h"  // Include the header file
#include "BeepManager.h"

/*************************
  Variables
**************************/
int16_t JOYSTICK_minX = -256;         // initial Joystick axis resolution
int16_t JOYSTICK_maxX = 256;
int16_t JOYSTICK_minY = -256;
int16_t JOYSTICK_maxY = 256;

unsigned long nextJoystickMillis = 0;   // count millis for next joystick update
unsigned long nextEffectsMillis = 0;    // count millis for next Effect update
unsigned long currentMillis;            // millis for the current loop

bool blSerialDebug = false;             // serial debug mode
bool blCalibration = true;              // start flag
byte bSerialIndex = 0;                  // global index for serial communication

int16_t forces[MEM_AXES] = { 0, 0 };        // stored forces
Gains gains[FFB_AXIS_COUNT];                // stored gain parameters
EffectParams effects[MEM_AXES];             // stored effect parameters

int16_t adjForceMax[MEM_AXES] = { 0, 0 };  // stored max adjusted force
byte adjPwmMin[MEM_AXES] = { 0, 0 };       // stored start pwm (motor power) on force != 0
byte adjPwmMax[MEM_AXES] = { 0, 0 };       // stored max pwm (motor power) on max force

Encoder counterRoll(ROLL_ENC_A,ROLL_ENC_B );   // init encoder library for roll ir sensor
Encoder counterPitch(PITCH_ENC_A,PITCH_ENC_B);  // init encoder library for pitch ir sensor

BeepManager beepManager(BUZZER_PIN);  // Instanz der BeepManager Klasse

// variables for Speed calculation
byte roll_speed = 0;
byte pitch_speed = 0;

// variables for calculation
unsigned long lastEffectsUpdate = 0;  // count millis for next effect calculation
int16_t lastX;                        // X value from last loop
int16_t lastY;                        // Y value from last loop
int16_t lastVelX;                     // Velocity X value from last loop
int16_t lastVelY;                     // Velocity y value from last loop
int16_t lastAccelX;                   // Acceleration X value from last loop
int16_t lastAccelY;                   // Acceleration X value from last loop
int32_t counterRollValue = 0;
int32_t counterPitchValue = 0;
bool isCalibrationPressed = false;   // flag for calibration button

Joystick_ Joystick(            // define Joystick parameters
  JOYSTICK_DEFAULT_REPORT_ID,  // ID defined in Joystick.h
  JOYSTICK_TYPE_JOYSTICK,      // type Joystick
  12, 1,                       // Button Count, Hat Switch Count
  true, true, false,           // X, Y, Z
  false, false, false,         // Rx, Ry, Rz
  false, false);               // rudder, throttle

Multiplexer mux(&Joystick);   // class for mutiplexers

Axis rollAxis(ROLL_L_PWM, ROLL_R_PWM, true, &counterRoll, &mux, &beepManager);        // Roll Axis class
Axis pitchAxis(PITCH_U_PWM, PITCH_D_PWM, false, &counterPitch, &mux, &beepManager);   // pitch axis class

/********************************
     initial setup
*******************************/
void setup() {
  ArduinoSetup();   // setup for Arduino itself (pins)
  SetupJoystick();  // Joystick setup

  Serial.begin(SERIAL_BAUD);  // init serial

  // if serial debig, no motors enabled
#ifndef SERIAL_DEBUG
   EnableMotors();             // enable motors
#endif

   delay(1000);
   beepManager.SystemStart();  // Systemstart-Sound
}  //setup


/***************************
      main loop
****************************/
void loop() {

// if serial debug mode than only display pins, mux and counters
#ifdef SERIAL_DEBUG
  mux.ReadMux();                // read ir sensors
  mux.UpdateJoystickButtons();  // read yoke buttons
  Serial.print(", Roll:");      
  Serial.print(counterRollValue);
  Serial.print(", Pitch:");
  Serial.print(counterPitchValue);  
  if calibration button is pressed than reset counters
  if (mux.CalibrationButtonPushed()) {  
    counterRoll.readAndReset(); 
    counterPitch.readAndReset();
  }
  Serial.println("");
  delay(30);
#else
// else normal yoke mode

  currentMillis = millis();   // number of milliseconds passed since the Arduino board began running the current program

  counterRollValue=counterRoll.read();
  counterPitchValue=counterPitch.read();
  mux.ReadMux();              // Read values of buttons and end switch sensors (except yoke axes)

  if (blCalibration) {        // calibration should start?
    Calibrate();
    blCalibration = false;
  } else {
    // prevent double start calibration and missing the button press
    if (mux.CalibrationButtonPushed()) {  
      isCalibrationPressed = true;
    } else {
      if (isCalibrationPressed == true) {
        isCalibrationPressed = false;
        blCalibration = true;
      }
    }

    // normal yoke mode
    if (currentMillis >= nextJoystickMillis) {  // do not run more frequently than these many milliseconds, the window system needs time to process
      mux.UpdateJoystickButtons();              // set Joystick buttons
      Joystick.sendState();                     // send joystick values to system

      if (currentMillis >= nextEffectsMillis) {   // we calculate condition forces every 100ms or more frequently if we get position updates
        UpdateEffects(true);                      // update/calculate new effect paraeters
        nextEffectsMillis = currentMillis + 100;  // set time for new effect loop
      } else {
        UpdateEffects(false);  // calculate forces without recalculating condition forces
                               //this helps having smoother spring/damper/friction
                               //if our update rate matches our input device
      }                        //nextEffectsMillis  || pos_updated

      if (mux.MotorPower()) {
        PrepareMotors();
      }                                         // move motors
      nextJoystickMillis = currentMillis + 20;  // set time for new joystick loop

      SerialEvent();                                                  // check if serial event received
    }  //nextJoystickMillis
  }    // else
#endif
}  // loop


/***************************
  Calibration
****************************/
void Calibrate() {
  delay(1000);

  // check motor power available
  if (!mux.MotorPower()) {               // is motor power available?
    beepManager.CalibrationError();  //  Calibration error beep
    delay(BEEP_CODE_DELAY);
    beepManager.NoMotorPower();
    return;
  }

  beepManager.CalibrationStart();  // Calibration start beep
  delay(BEEP_CODE_DELAY);

  // Start ROLL axis calibration
  rollAxis.Calibrate();     
  // check error
  if(rollAxis.CheckError(true))
  {
    return;
  }

  delay(500);

  // Start PITCH axis calibration
  pitchAxis.Calibrate();
  // check error
  if(pitchAxis.CheckError(false))
  {
    return;
  }

  SetRangeJoystick();  // Set Joystick range

  
}


