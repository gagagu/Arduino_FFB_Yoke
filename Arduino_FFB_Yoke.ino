/*
  Version 2.0.0 – improved
  Created by A.Eckers aka Gagagu
  http://www.gagagu.de / https://github.com/gagagu/Arduino_FFB_Yoke
*/

/*************************
  Includes
**************************/
#include "src/Joystick.h"
#include <Encoder.h>
#include <EEPROM.h>
#include "defines.h"
#include "Multiplexer.h"
#include "AxisCalibration.h"
#include "BeepManager.h"

/*************************
  Variables
**************************/
int16_t JOYSTICK_minX = -256;
int16_t JOYSTICK_maxX =  256;
int16_t JOYSTICK_minY = -256;
int16_t JOYSTICK_maxY =  256;

unsigned long nextJoystickMillis = 0;
unsigned long nextEffectsMillis  = 0;
unsigned long currentMillis;

bool blSerialDebug = false;
bool blCalibration = true;
byte bSerialIndex  = 0;

int16_t forces[MEM_AXES]      = { 0, 0 };
Gains        gains[FFB_AXIS_COUNT];
EffectParams effects[MEM_AXES];

int16_t adjForceMax[MEM_AXES] = { 0, 0 };
byte    adjPwmMin[MEM_AXES]   = { 0, 0 };
byte    adjPwmMax[MEM_AXES]   = { 0, 0 };

Encoder counterRoll (ROLL_ENC_A,  ROLL_ENC_B);
Encoder counterPitch(PITCH_ENC_A, PITCH_ENC_B);

BeepManager beepManager(BUZZER_PIN);

byte roll_speed  = 0;
byte pitch_speed = 0;

unsigned long lastEffectsUpdate = 0;
int16_t lastX      = 0;
int16_t lastY      = 0;
int16_t lastVelX   = 0;
int16_t lastVelY   = 0;
int16_t lastAccelX = 0;
int16_t lastAccelY = 0;

int32_t counterRollValue  = 0;
int32_t counterPitchValue = 0;

bool isCalibrationPressed = false;

Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  12, 1,
  true,  true,  false,
  false, false, false,
  false, false);

Multiplexer mux(&Joystick);
Axis rollAxis (ROLL_L_PWM,  ROLL_R_PWM,  true,  &counterRoll,  &mux, &beepManager);
Axis pitchAxis(PITCH_U_PWM, PITCH_D_PWM, false, &counterPitch, &mux, &beepManager);

/********************************
     Setup
*******************************/
void setup() {
  ArduinoSetup();
  SetupJoystick();
  Serial.begin(SERIAL_BAUD);

#if defined(DBG_LEVEL)
  Serial.print(F("[init] Debug level: "));
  Serial.println(DBG_LEVEL);
#endif

  EnableMotors();
  delay(1000);
  beepManager.SystemStart();
}

/***************************
      Main loop
****************************/
void loop() {

  // Normal yoke operation – always runs regardless of DBG_LEVEL.
  // DBGx macros compile to nothing at DBG_LEVEL 0, so there is no
  // overhead in release builds.
  currentMillis     = millis();
  counterRollValue  = counterRoll.read();
  counterPitchValue = counterPitch.read();
  mux.ReadMux();

  if (blCalibration) {
    Calibrate();
    blCalibration = false;
  } else {
    // Edge-triggered calibration button
    if (mux.CalibrationButtonPushed()) {
      isCalibrationPressed = true;
    } else if (isCalibrationPressed) {
      isCalibrationPressed = false;
      blCalibration        = true;
    }

    if (currentMillis >= nextJoystickMillis) {
      mux.UpdateJoystickButtons();
      Joystick.sendState();

      if (currentMillis >= nextEffectsMillis) {
        UpdateEffects(true);
        nextEffectsMillis = currentMillis + 100;
      } else {
        UpdateEffects(false);
      }

      if (mux.MotorPower()) PrepareMotors();

      nextJoystickMillis = currentMillis + 20;
      SerialEvent();
    }
  }

  // DBG_LEVEL 1: log encoder positions, motor speeds and forces each loop tick.
  // Compiles away completely at DBG_LEVEL 0.
  DBG_TS();
  DBG1(F("roll="));  DBG1(counterRollValue);
  DBG1(F(" ptch=")); DBG1(counterPitchValue);
  DBG1(F(" sR="));   DBG1(roll_speed);
  DBG1(F(" sP="));   DBG1(pitch_speed);
  DBG1(F(" fR="));   DBG1(forces[MEM_ROLL]);
  DBG1(F(" fP="));   DBG1(forces[MEM_PITCH]);
  DBG1LN(F(""));
}

/***************************
  Calibration
****************************/
void Calibrate() {
  delay(1000);

  if (!mux.MotorPower()) {
    beepManager.CalibrationError();
    delay(BEEP_CODE_DELAY);
    beepManager.NoMotorPower();
    return;
  }

  beepManager.CalibrationStart();
  delay(BEEP_CODE_DELAY);

  rollAxis.Calibrate();
  if (rollAxis.CheckError(true)) return;

  delay(500);

  pitchAxis.Calibrate();
  if (pitchAxis.CheckError(false)) return;

  SetRangeJoystick();
}
