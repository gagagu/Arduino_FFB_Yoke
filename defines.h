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

#ifndef DEFINES_H
#define DEFINES_H

/*****************************
 uncomment for Serial Debug, only Sensor Data will be write to Serial Monitor
*****************************/
//#define SERIAL_DEBUG 1

/*****************************
 -uncomment for Ardunio Pro Micro
 -comment out for Arduino Micro (original)
*****************************/
#define ARDUINO_PRO_MICRO 1


#define SERIAL_BAUD 115200  // Communication Speed

#ifdef ARDUINO_PRO_MICRO
  #define BUZZER_PIN 4
#else
  #define BUZZER_PIN 12
#endif

// Roll Encoder Pins
#define ROLL_ENC_A 0
#define ROLL_ENC_B 1

// Pitch Encoder Pins
#define PITCH_ENC_A 3
#define PITCH_ENC_B 2

#ifdef ARDUINO_PRO_MICRO
  // Pitch Motordriver pins
  #define PITCH_EN 7
  #define PITCH_U_PWM 5
  #define PITCH_D_PWM 6
#else
  // Pitch Motordriver pins
  #define PITCH_EN 11
  #define PITCH_U_PWM 6
  #define PITCH_D_PWM 13
#endif

// Roll Motordriver pins
#define ROLL_EN 8
#define ROLL_R_PWM 9
#define ROLL_L_PWM 10


  //ARDUINO_PRO_MICRO
  // Multiplexer Yoke Buttons
  #define MUX_YOKE_OUT A3
  #define MUX_YOKE_PL A2
  #define MUX_YOKE_CLK A1

  // Multiplexer Calibration Button, Power Measure, IR Sensors
  #define MUX_INT_OUT 16
  #define MUX_INT_PL 15
  #define MUX_INT_CLK 14

  // for RoxMux Library
  // used for array sizes not pins!
  #define MUX_TOTAL_INT 1
  #define MUX_TOTAL_YOKE 2

  // Arduino Micro 
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



/*****************************
  Memory array positions for Effects
****************************/
#define MEM_ROLL  0
#define MEM_PITCH  1
#define MEM_AXES  2

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
#define default_PITCH_PWM_MAX 170;
#define default_PITCH_PWM_MIN 40;

#define default_ROLL_FORCE_MAX 10000;
#define default_ROLL_PWM_MAX 170;
#define default_ROLL_PWM_MIN 40;


/******************************************
   Calibration Constants
*******************************************/
#define CALIBRATION_MAX_SPEED 130;                        // Maximum speed
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
#define BEEP_CODE_DELAY 500
#define BEEP_CODE_COUNT 3 //start with 0

#endif
