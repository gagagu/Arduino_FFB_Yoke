#ifndef DEFINES_H
#define DEFINES_H

#define SERIAL_BAUD 115200  // Communication Speed

//#define SERIAL_DEBUG 1

#define BUZZER_PIN 12

/*****************************
  Memory array positions for Effects
****************************/
#define MEM_ROLL  0
#define MEM_PITCH  1
#define MEM_AXES  2

// Pitch Motordriver pins
#define PITCH_EN 11
#define PITCH_U_PWM 6
#define PITCH_D_PWM 13

// Roll Motordriver pins
#define ROLL_EN 8
#define ROLL_R_PWM 9
#define ROLL_L_PWM 10

// multiplexer for buttons
#define MUX_S0 A0
#define MUX_S1 A1
#define MUX_S2 A2
#define MUX_S3 A3

// Multiplexer Yoke Buttons
#define MUX_EN_YOKE 5
#define MUX_SIGNAL_YOKE A5

// Multiplexer Adjustemts for Calib Button, Force Potis, End Switches
#define MUX_EN_INPUT 4
#define MUX_SIGNAL_INPUT A4

// Adjustments array positions
#define ADJ_ENDSWITCH_PITCH_DOWN 0
#define ADJ_ENDSWITCH_PITCH_UP 1
#define ADJ_ENDSWITCH_ROLL_LEFT 2
#define ADJ_ENDSWITCH_ROLL_RIGHT 3
#define ADJ_CALIBRATION_BUTTON 4
#define ADJ_MOTOR_POWER 5


/******************************************
   Communication Command Constants
*******************************************/
//Debug
#define SERIAL_CMD_DEBUG_START 1
#define SERIAL_CMD_DEBUG_STOP 2
#define SERIAL_CMD_DEBUG_STATUS 3
#define SERIAL_CMD_DEBUG_VALUES 4

#define SERIAL_CMD_DEBUG_FORCE_VALUES 8


// read
#define SERIAL_CMD_READ_ALL_PARAMS 10
#define SERIAL_CMD_READ_ALL_VALUES 20

// Write Roll
#define SERIAL_CDM_WRITE_ROLL_FORCE_MAX 101
#define SERIAL_CDM_WRITE_ROLL_PWM_MIN 102
#define SERIAL_CDM_WRITE_ROLL_PWM_MAX 103

#define SERIAL_CDM_WRITE_ROLL_FRICTION_MAX_POSITION_CHANGE 104
#define SERIAL_CDM_WRITE_ROLL_INERTIA_MAX_ACCELERATION 105
#define SERIAL_CDM_WRITE_ROLL_DAMPER_MAX_VELOCITY 106

#define SERIAL_CDM_WRITE_ROLL_TOTAL_GAIN 107
#define SERIAL_CDM_WRITE_ROLL_CONSTANT_GAIN 108
#define SERIAL_CDM_WRITE_ROLL_RAMP_GAIN 109
#define SERIAL_CDM_WRITE_ROLL_SQUARE_GAIN 110
#define SERIAL_CDM_WRITE_ROLL_SINE_GAIN 111
#define SERIAL_CDM_WRITE_ROLL_TRIANGLE_GAIN 112
#define SERIAL_CDM_WRITE_ROLL_SAWTOOTH_DOWN_GAIN 113
#define SERIAL_CDM_WRITE_ROLL_SAWTOOTH_UP_GAIN 114
#define SERIAL_CDM_WRITE_ROLL_SPRING_GAIN 115
#define SERIAL_CDM_WRITE_ROLL_DAMPER_GAIN 116
#define SERIAL_CDM_WRITE_ROLL_INERTIA_GAIN 117
#define SERIAL_CDM_WRITE_ROLL_FRICTION_GAIN 118

// write pitch
#define SERIAL_CDM_WRITE_PITCH_FORCE_MAX 119
#define SERIAL_CDM_WRITE_PITCH_PWM_MIN 120
#define SERIAL_CDM_WRITE_PITCH_PWM_MAX 121

#define SERIAL_CDM_WRITE_PITCH_FRICTION_MAX_POSITION_CHANGE 122
#define SERIAL_CDM_WRITE_PITCH_INERTIA_MAX_ACCELERATION 123
#define SERIAL_CDM_WRITE_PITCH_DAMPER_MAX_VELOCITY 124

#define SERIAL_CDM_WRITE_PITCH_TOTAL_GAIN 125
#define SERIAL_CDM_WRITE_PITCH_CONSTANT_GAIN 126
#define SERIAL_CDM_WRITE_PITCH_RAMP_GAIN 127
#define SERIAL_CDM_WRITE_PITCH_SQUARE_GAIN 128
#define SERIAL_CDM_WRITE_PITCH_SINE_GAIN 129
#define SERIAL_CDM_WRITE_PITCH_TRIANGLE_GAIN 130
#define SERIAL_CDM_WRITE_PITCH_SAWTOOTH_DOWN_GAIN 131
#define SERIAL_CDM_WRITE_PITCH_SAWTOOTH_UP_GAIN 132
#define SERIAL_CDM_WRITE_PITCH_SPRING_GAIN 133
#define SERIAL_CDM_WRITE_PITCH_DAMPER_GAIN 134
#define SERIAL_CDM_WRITE_PITCH_INERTIA_GAIN 135
#define SERIAL_CDM_WRITE_PITCH_FRICTION_GAIN 136

// eeprom
#define SERIAL_CMD_WRITE_DATA_EEPROM 250
#define SERIAL_CMD_WRITE_EEPROM_CLEAR 251

#define EEPROM_DATA_AVAILABLE_INDEX 0     // eeprom address to indicate data available
                                          // space between reserved
#define EEPROM_DATA_INDEX 10              // eeprom start address for data

// Default vaules for gains and effect if nothing saved into eeprom
#define default_gain 100
#define default_friction_gain 25

#define default_frictionMaxPositionChange_ROLL 125;
#define default_inertiaMaxAcceleration_ROLL 100;
#define default_damperMaxVelocity_ROLL 350;

#define default_frictionMaxPositionChange_PITCH 125;
#define default_inertiaMaxAcceleration_PITCH 100;
#define default_damperMaxVelocity_PITCH 350;

#define default_PITCH_FORCE_MAX 10000;
#define default_PITCH_PWM_MAX 150;
#define default_PITCH_PWM_MIN 40;

#define default_ROLL_FORCE_MAX 10000;
#define default_ROLL_PWM_MAX 150;
#define default_ROLL_PWM_MIN 40;


/******************************************
   Calibration Constants
*******************************************/
  #define  CALIBRATION_MAX_SPEED 100;                        // Maximum speed
  #define CALIBRATION_AXIS_MOVEMENT_TIMEOUT 4000;           // Timeout of 4 seconds for no movement
  #define CALIBRATION_TIMEOUT 20000;                        // Timeout of 20 seconds for calibration
  #define CALIBRATION_SPEED_INCREMENT 10;                   // the speed is increased until asix movement, this is added to speed then move´ment indicates 
  #define CALIBRATION_WHILE_DELAY 20;                        // waitdelay inside while of movement to give Arduino time. Change will change speed!
  #define CALIBRATION_WHILE_DELAY_MOTOR_STOPS 30;           // waitdelay when motor stops to give him time to stops
  #define CALIBRATION_DELAY_MOVE_OUT_OF_ENDSTOP 100;        // If asix is on endstop on start od´f calibration it will move out of and wait shot before continue

/******************************************
   Beep
*******************************************/
#define BEEP_SHORT_TONE 200
#define BEEP_LONG_TONE 600
#define BEEP_CODE_FREQUENCY 1000
#define BEEP_CODE_DELAY 1000
#define BEEP_CODE_COUNT 3 //start with 0

#endif