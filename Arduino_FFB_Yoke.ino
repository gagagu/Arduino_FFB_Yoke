
#include "src/Joystick.h"
#include "config.h"
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

/**** BEGIN DEBUG INFO **********************
  Info: Please use only one debub info simultaneously
  Enable: remove the "NO" letters
  Disable: add "NO" letter before
*************************/


#define DEBUG       // gerneral debug info for Joystick, Please use the Arduino_FFB_Yoke_App for read out
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
  LCD Strings
****************************/
const char string_0[] PROGMEM = "       Arduino      ";
const char string_1[] PROGMEM = "Force-Feedback-Yoke ";
const char string_2[] PROGMEM = "Created by GaGaGu   ";
const char string_3[] PROGMEM = "                v1.0";
const char string_4[] PROGMEM = "Roll  Force:";
const char string_5[] PROGMEM = "  PWM Mi-Ma:";
const char string_6[] PROGMEM = "Pitch Force:";
const char string_7[] PROGMEM = "  PWM Mi-Ma:";
const char string_8[] PROGMEM = "-";
const char string_9[] PROGMEM = "Calibration started.";
const char string_10[] PROGMEM = "Please move all axes";
const char string_11[] PROGMEM = "to the end positions";
const char string_12[] PROGMEM = "Left  end:";
const char string_13[] PROGMEM = "Right end:";
const char string_14[] PROGMEM = "Up    end:";
const char string_15[] PROGMEM = "Down  end:";
const char string_16[] PROGMEM = "Put all axes in the ";
const char string_17[] PROGMEM = "middle position and ";
const char string_18[] PROGMEM = "press the button    ";
const char string_19[] PROGMEM = "again               ";
const char string_20[] PROGMEM = " ";

// Initialisiere die Tabelle von Strings
const char* const string_table[] PROGMEM = { string_0, string_1, string_2, string_3, string_4, string_5, string_6, string_7, string_8, string_9, string_10
                                             , string_11, string_12, string_13, string_14, string_15, string_16, string_17, string_18, string_19, string_20
                                           };
char stringbuffer[20]; // Der Buffer zum Lesen der Daten muss so lang sein wie der längste String!

/*****************************
  variables
****************************/
unsigned long lastEffectsUpdate;    // count millis for next effect calculation
unsigned long nextJoystickMillis;   // count millis for next joystick update
unsigned long nextEffectsMillis;    // count millis for next Effect update
unsigned long waitMillis = 0;         // count millis for wait loops
unsigned long lcdMillis;        // millis for the lcd display
unsigned long currentMillis;        // millis for the current loop

byte myLcdCounter = 0;
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

LiquidCrystal lcd(0, 12,4,5,3,7);

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

  lcd.begin(20 , 4);
  LcdPrintIntro();
  delay(3000);
  lcd.clear();
}

/******************************
      main loop
****************************/
void loop() {
  currentMillis = millis();                                             // number of milliseconds passed since the Arduino board began running the current program

  if (currentMillis >= nextJoystickMillis) {                            // do not run more frequently than these many milliseconds, the window system needs time to process
    ReadMux();

    CheckCalibrationMode();
    UpdateJoystickPos();                                              // read Joystick Potentiometers and send it to system
    UpdateJoystickButtons();                                          // read Joystick Buttons and sent it to system

    if (currentMillis >= nextEffectsMillis || pos_updated) {          // we calculate condition forces every 100ms or more frequently if we get position updates
      updateEffects(true);                                            // update/calculate new effect paraeters
      nextEffectsMillis = currentMillis + 100;                        // set time for new effect loop
      pos_updated = false;
    } else {
      updateEffects(false);                                           // calculate forces without recalculating condition forces
      // this helps having smoother spring/damper/friction
      // if our update rate matches our input device
    } //nextEffectsMillis  || pos_updated

    DriveMotors();                                                    // move motors
    nextJoystickMillis = currentMillis + 2;                           // set time for new joystick loop

    // Updating LCD is too long and will caused in heavy movements
    // so the display will updated line by line to reduce time
    if (currentMillis > lcdMillis) {
      LcdAdjustmendValues(myLcdCounter);
      myLcdCounter++;
      if (myLcdCounter == 4) {
        myLcdCounter = 0;
      }
      lcdMillis = currentMillis + 250;
    }

#ifdef DEBUG
    Serial.println("");
#endif
  } //nextJoystickMillis
} // loop
