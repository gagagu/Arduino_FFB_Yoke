// Version 2.0.0 Beta
#include "src/Joystick.h"  // Joystick and FFB Library (many changes made) by https://github.com/jmriego/Fino
#include <Encoder.h>       // Encoder Library https://github.com/PaulStoffregen/Encoder
#include <EEPROM.h>        // https://docs.arduino.cc/learn/built-in-libraries/eeprom

#include "defines.h"
#include "Multiplexer.h"
#include "AxisCalibration.h"  // Include the header file
#include "BeepManager.h"


int16_t JOYSTICK_minX = -256;
int16_t JOYSTICK_maxX = 256;
int16_t JOYSTICK_minY = -256;
int16_t JOYSTICK_maxY = 256;

unsigned long nextJoystickMillis = 0;  // count millis for next joystick update
unsigned long nextEffectsMillis = 0;   // count millis for next Effect update
unsigned long currentMillis;           // millis for the current loop

bool blSerialDebug = false;  // serial debug mode
bool blCalibration = true;   // start flag
byte bSerialIndex = 0;

int16_t forces[MEM_AXES] = { 0, 0 };  // stored forces
Gains gains[FFB_AXIS_COUNT];          // stored gain parameters
EffectParams effects[MEM_AXES];       // stored effect parameters

int16_t adjForceMax[MEM_AXES] = { 0, 0 };  // stored max adjusted force
byte adjPwmMin[MEM_AXES] = { 0, 0 };       // stored start pwm (motor power) on force != 0
byte adjPwmMax[MEM_AXES] = { 0, 0 };       // stored max pwm (motor power) on max force



Encoder counterRoll(0, 1);   // init encoder library for roll ir sensor
Encoder counterPitch(3, 2);  // init encoder library for pitch ir sensor

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

bool isCalibrationPressed = false;

AxisConfiguration rollConfig = { false, -256, 256, false, false };
AxisConfiguration pitchConfig = { false, -256, 256, false, false };

Joystick_ Joystick(            // define Joystick parameters
  JOYSTICK_DEFAULT_REPORT_ID,  // ID defined in Joystick.h
  JOYSTICK_TYPE_JOYSTICK,      // type Joystick
  12, 1,                       // Button Count, Hat Switch Count
  true, true, false,           // X, Y, Z
  false, false, false,         // Rx, Ry, Rz
  false, false);               // rudder, throttle

Multiplexer mux(&Joystick);
Axis rollAxis(ROLL_L_PWM, ROLL_R_PWM, true, &counterRoll, &mux);
Axis pitchAxis(PITCH_U_PWM, PITCH_D_PWM, false, &counterPitch, &mux);

/********************************
     initial setup
*******************************/
void setup() {
  ArduinoSetup();   // setup for Arduino itself (pins)
  SetupJoystick();  // Joystick setup

  Serial.begin(SERIAL_BAUD);  // init serial

  EnableMotors();             // enable motors
  beepManager.SystemStart();  // Systemstart-Sound
  delay(3000);
}  //setup


/******************************
      main loop
****************************/
void loop() {

  currentMillis = millis();  // number of milliseconds passed since the Arduino board began running the current program

  mux.ReadMux();  // Read values of buttons and end switch sensors (except yoke axes)

  //Serial.println(mux.MotorPower());

  if (blCalibration) {
    Calibrate();
  } else {
    if (mux.CalibrationButtonPushed()) {
      isCalibrationPressed = true;
    } else {
      if (isCalibrationPressed == true) {
        isCalibrationPressed = false;
        blCalibration = true;
      }
    }

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
}  // loop


void Calibrate() {
  delay(1000);
  beepManager.CalibrationStart();  // Kalibrierung startet

  if (mux.MotorPower()) {
    // Start ROLL axis calibration
    rollAxis.Calibrate();
    rollConfig = rollAxis.GetConfiguration();
    if (rollConfig.blError) {
      beepManager.CalibrationError();  // Kalibrierung fehlerhaft
      delay(1000);

      if (rollConfig.blEncoderInverted)
        beepManager.CalibrationEncoderInverted(true);

      if (rollConfig.blMotorInverted)
        beepManager.CalibrationMotorInverted(true);

      if (rollConfig.blAxisTimeout)
        beepManager.CalibrationTimeoutMotor(true);

      if (rollConfig.blTimeout)
        beepManager.CalibrationTimeoutGeneral(true);
    } else {
      delay(500);

      // Start PITCH axis calibration
      pitchAxis.Calibrate();
      pitchConfig = pitchAxis.GetConfiguration();
      if (pitchConfig.blError) {
        beepManager.CalibrationError();  // Kalibrierung fehlerhaft
        delay(1000);

        if (pitchConfig.blEncoderInverted)
          beepManager.CalibrationEncoderInverted(false);

        if (pitchConfig.blMotorInverted)
          beepManager.CalibrationMotorInverted(false);

        if (pitchConfig.blAxisTimeout)
          beepManager.CalibrationTimeoutMotor(false);

        if (pitchConfig.blTimeout)
          beepManager.CalibrationTimeoutGeneral(false);
      } else {
        beepManager.CalibrationSuccess();  // Kalibrierung erfolgreich
        delay(1000);
      }
    }

    SetRangeJoystick();  // Start with Calibration
  }else{
    beepManager.CalibrationError();  // Kalibrierung fehlerhaft
    delay(1000);
    beepManager.NoMotorPower();
    delay(1000);
  }
  blCalibration = false;
}