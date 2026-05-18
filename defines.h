/* 
 Created by A.Eckers aka Gagagu – improved version
 http://www.gagagu.de / https://github.com/gagagu/Arduino_FFB_Yoke
*/

#ifndef DEFINES_H
#define DEFINES_H

/*****************************
  Debug levels – uncomment ONE or NONE
  
  DBG_LEVEL 0 = off (default, release build)
  DBG_LEVEL 1 = main loop values (encoder, forces, speeds) + timestamps
  DBG_LEVEL 2 = level 1 + calibration step-by-step progress
  DBG_LEVEL 3 = level 2 + raw multiplexer pin states
*****************************/
//#define DBG_LEVEL 1
//#define DBG_LEVEL 2
//#define DBG_LEVEL 3

// Keep SERIAL_DEBUG for backwards compatibility (maps to level 3)
//#define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
  #ifndef DBG_LEVEL
    #define DBG_LEVEL 3
  #endif
#endif

// Convenience macros – compile to nothing when debug is off
#if defined(DBG_LEVEL) && DBG_LEVEL >= 1
  #define DBG1(x)  Serial.print(x)
  #define DBG1LN(x) Serial.println(x)
  #define DBG_TS()  do { Serial.print(F("["));  Serial.print(millis()); Serial.print(F("] ")); } while(0)
#else
  #define DBG1(x)
  #define DBG1LN(x)
  #define DBG_TS()
#endif

#if defined(DBG_LEVEL) && DBG_LEVEL >= 2
  #define DBG2(x)   Serial.print(x)
  #define DBG2LN(x) Serial.println(x)
#else
  #define DBG2(x)
  #define DBG2LN(x)
#endif

#if defined(DBG_LEVEL) && DBG_LEVEL >= 3
  #define DBG3(x)   Serial.print(x)
  #define DBG3LN(x) Serial.println(x)
#else
  #define DBG3(x)
  #define DBG3LN(x)
#endif

/*****************************
 - Uncomment for Arduino Pro Micro
 - Comment out for Arduino Micro (original)
*****************************/
#define ARDUINO_PRO_MICRO 1

#define SERIAL_BAUD 115200

#ifdef ARDUINO_PRO_MICRO
  #define BUZZER_PIN 4
#else
  #define BUZZER_PIN 12
#endif

// Roll encoder pins
#define ROLL_ENC_A 0
#define ROLL_ENC_B 1

// Pitch encoder pins
#define PITCH_ENC_A 3
#define PITCH_ENC_B 2

#ifdef ARDUINO_PRO_MICRO
  #define PITCH_EN    7
  #define PITCH_U_PWM 5
  #define PITCH_D_PWM 6
#else
  #define PITCH_EN    11
  #define PITCH_U_PWM 6
  #define PITCH_D_PWM 13
#endif

#define ROLL_EN    8
#define ROLL_R_PWM 9
#define ROLL_L_PWM 10

// Multiplexer – Yoke buttons (Pro Micro)
#define MUX_YOKE_OUT A3
#define MUX_YOKE_PL  A2
#define MUX_YOKE_CLK A1

// Multiplexer – Internal sensors (Pro Micro)
#define MUX_INT_OUT 16
#define MUX_INT_PL  15
#define MUX_INT_CLK 14

// RoxMux array sizes (not pins!)
#define MUX_TOTAL_INT  1
#define MUX_TOTAL_YOKE 2

// Multiplexer pins (original Arduino Micro)
#define MUX_S0 A0
#define MUX_S1 A1
#define MUX_S2 A2
#define MUX_S3 A3
#define MUX_EN_YOKE     5
#define MUX_SIGNAL_YOKE A5
#define MUX_EN_INPUT     4
#define MUX_SIGNAL_INPUT A4

/*****************************
  Memory / effect array indices
****************************/
#define MEM_ROLL  0
#define MEM_PITCH 1
#define MEM_AXES  2

// Sensor array positions
#define ADJ_ENDSWITCH_PITCH_DOWN  0
#define ADJ_ENDSWITCH_PITCH_UP    1
#define ADJ_ENDSWITCH_ROLL_LEFT   2
#define ADJ_ENDSWITCH_ROLL_RIGHT  3
#define ADJ_CALIBRATION_BUTTON    4
#define ADJ_MOTOR_POWER           5

/******************************************
   Serial command constants
*******************************************/
#define SERIAL_CMD_DEBUG_START        1
#define SERIAL_CMD_DEBUG_STOP         2
#define SERIAL_CMD_DEBUG_STATUS       3
#define SERIAL_CMD_DEBUG_VALUES       4
#define SERIAL_CMD_DEBUG_FORCE_VALUES 8
#define SERIAL_CMD_READ_ALL_PARAMS   10
#define SERIAL_CMD_READ_ALL_VALUES   20

#define SERIAL_CMD_WRITE_ROLL_FORCE_MAX               101
#define SERIAL_CMD_WRITE_ROLL_PWM_MIN                 102
#define SERIAL_CMD_WRITE_ROLL_PWM_MAX                 103
#define SERIAL_CMD_WRITE_ROLL_FRICTION_MAX_POS_CHANGE 104
#define SERIAL_CMD_WRITE_ROLL_INERTIA_MAX_ACCEL       105
#define SERIAL_CMD_WRITE_ROLL_DAMPER_MAX_VELOCITY     106
#define SERIAL_CMD_WRITE_ROLL_TOTAL_GAIN              107
#define SERIAL_CMD_WRITE_ROLL_CONSTANT_GAIN           108
#define SERIAL_CMD_WRITE_ROLL_RAMP_GAIN               109
#define SERIAL_CMD_WRITE_ROLL_SQUARE_GAIN             110
#define SERIAL_CMD_WRITE_ROLL_SINE_GAIN               111
#define SERIAL_CMD_WRITE_ROLL_TRIANGLE_GAIN           112
#define SERIAL_CMD_WRITE_ROLL_SAWTOOTH_DOWN_GAIN      113
#define SERIAL_CMD_WRITE_ROLL_SAWTOOTH_UP_GAIN        114
#define SERIAL_CMD_WRITE_ROLL_SPRING_GAIN             115
#define SERIAL_CMD_WRITE_ROLL_DAMPER_GAIN             116
#define SERIAL_CMD_WRITE_ROLL_INERTIA_GAIN            117
#define SERIAL_CMD_WRITE_ROLL_FRICTION_GAIN           118

#define SERIAL_CMD_WRITE_PITCH_FORCE_MAX               119
#define SERIAL_CMD_WRITE_PITCH_PWM_MIN                 120
#define SERIAL_CMD_WRITE_PITCH_PWM_MAX                 121
#define SERIAL_CMD_WRITE_PITCH_FRICTION_MAX_POS_CHANGE 122
#define SERIAL_CMD_WRITE_PITCH_INERTIA_MAX_ACCEL       123
#define SERIAL_CMD_WRITE_PITCH_DAMPER_MAX_VELOCITY     124
#define SERIAL_CMD_WRITE_PITCH_TOTAL_GAIN              125
#define SERIAL_CMD_WRITE_PITCH_CONSTANT_GAIN           126
#define SERIAL_CMD_WRITE_PITCH_RAMP_GAIN               127
#define SERIAL_CMD_WRITE_PITCH_SQUARE_GAIN             128
#define SERIAL_CMD_WRITE_PITCH_SINE_GAIN               129
#define SERIAL_CMD_WRITE_PITCH_TRIANGLE_GAIN           130
#define SERIAL_CMD_WRITE_PITCH_SAWTOOTH_DOWN_GAIN      131
#define SERIAL_CMD_WRITE_PITCH_SAWTOOTH_UP_GAIN        132
#define SERIAL_CMD_WRITE_PITCH_SPRING_GAIN             133
#define SERIAL_CMD_WRITE_PITCH_DAMPER_GAIN             134
#define SERIAL_CMD_WRITE_PITCH_INERTIA_GAIN            135
#define SERIAL_CMD_WRITE_PITCH_FRICTION_GAIN           136

#define SERIAL_CMD_WRITE_DATA_EEPROM  250
#define SERIAL_CMD_WRITE_EEPROM_CLEAR 251

#define EEPROM_DATA_AVAILABLE_INDEX  0
#define EEPROM_DATA_INDEX           10

// Default values (no trailing semicolons!)
#define DEFAULT_GAIN          100
#define DEFAULT_FRICTION_GAIN  25

#define DEFAULT_FRICTION_MAX_POS_CHANGE_ROLL  125
#define DEFAULT_INERTIA_MAX_ACCEL_ROLL        100
#define DEFAULT_DAMPER_MAX_VELOCITY_ROLL      350

#define DEFAULT_FRICTION_MAX_POS_CHANGE_PITCH 125
#define DEFAULT_INERTIA_MAX_ACCEL_PITCH       100
#define DEFAULT_DAMPER_MAX_VELOCITY_PITCH     350

#define DEFAULT_PITCH_FORCE_MAX 10000
#define DEFAULT_PITCH_PWM_MAX     170
#define DEFAULT_PITCH_PWM_MIN      40

#define DEFAULT_ROLL_FORCE_MAX  10000
#define DEFAULT_ROLL_PWM_MAX      170
#define DEFAULT_ROLL_PWM_MIN       40

// Calibration constants (no trailing semicolons!)
#define CALIBRATION_MAX_SPEED                  130
#define CALIBRATION_AXIS_MOVEMENT_TIMEOUT     4000
#define CALIBRATION_TIMEOUT                  20000
#define CALIBRATION_SPEED_INCREMENT             10
#define CALIBRATION_WHILE_DELAY                 20
#define CALIBRATION_WHILE_DELAY_MOTOR_STOPS     30
#define CALIBRATION_DELAY_MOVE_OUT_OF_ENDSTOP 100

// Beep
#define BEEP_SHORT_TONE       200
#define BEEP_LONG_TONE        600
#define BEEP_CODE_FREQUENCY  1000
#define BEEP_CODE_DELAY       500
#define BEEP_CODE_COUNT         3

#endif

/*****************************
  FFB Effect selection
  
  By default only the effects a yoke actually uses are compiled in:
    Spring, Damper, Inertia, Friction, Constant.
  
  Uncomment ENABLE_PERIODIC_EFFECTS to also include Sine, Square,
  Triangle, Sawtooth-Up/Down and Ramp. These add ~550 B of flash
  and are not needed for typical flight-sim FFB.
*****************************/
#define ENABLE_PERIODIC_EFFECTS
