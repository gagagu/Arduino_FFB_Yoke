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

#ifndef AXIS_CALIBRATION_H
#define AXIS_CALIBRATION_H

#include <Arduino.h>
#include "Multiplexer.h"
#include "BeepManager.h"
#include "defines.h"
#include <Encoder.h>

// Structure to hold the configuration of the axis
typedef struct {
    bool blError;           // true if a timeout occurred
    int16_t iMin;          // Minimum value
    int16_t iMax;          // Maximum value
    bool blMotorInverted;   // true if the motor direction is inverted
    bool blEncoderInverted;    // true if the encoder counting direction is inverted
    bool blAxisTimeout;
    bool blTimeout;
} AxisConfiguration;

class Axis {
private:
    Multiplexer* multiplexer;  // Pointer to a multiplexer object
    
    byte motorPinLeft;          // Pin for left motor control
    byte motorPinRight;         // Pin for right motor control
    bool blEndSwitchLeft;     // Pointer to left end switch status
    bool blEndSwitchRight;    // Pointer to right end switch status
    bool blIsRoll;            // is Roll or Pitch
    Encoder* encoder;          // Pointer to the encoder object
    byte speed;                 // Current speed of the motor
    unsigned long lastMovementTime;  // Timestamp of the last movement
    BeepManager *beepManager;

    // Configuration constants
    static const byte maxSpeed = CALIBRATION_MAX_SPEED;                                     // Maximum speed
    static const unsigned long timeout = CALIBRATION_AXIS_MOVEMENT_TIMEOUT;                 // Timeout of 4 seconds for no movement
    static const unsigned long calibrationTimeout = CALIBRATION_TIMEOUT;                    // Timeout of 20 seconds for calibration
    static const byte speedIncrement = CALIBRATION_SPEED_INCREMENT;                         // Speed increment value
    static const byte whileDelay = CALIBRATION_WHILE_DELAY;                                   // delay inside while to give Arduino time to work 
    static const byte waitDelayMotorStops =CALIBRATION_WHILE_DELAY_MOTOR_STOPS;               // delay inside while to give Arduino time to work 
    static const byte waitDelayAfterMoveOutEndstop =CALIBRATION_DELAY_MOVE_OUT_OF_ENDSTOP;     // not too fast

    // Calibration start time
    unsigned long calibrationStartTime; 

    // Axis configuration object
    AxisConfiguration config;

    // Helper method to manage motor movement
    void ManageMovement(bool direction, unsigned long& lastMovementTime, int& lastEncoderValue, bool& speedIncreased);
    void ReadMultiplexer();
    int ResetEncoder();
    bool CheckTimeouts(unsigned long lastMovementTime, unsigned long calibrationStartTime);
public:
    // Constructor to initialize motor pins and end switches
    Axis(int motorLeftPin, int motorRightPin, bool isRoll, Encoder* encoderPtr, Multiplexer* multiplexerPtr, BeepManager* beepManagerPtr);

    // Method to move the motor in a given direction
    void MoveMotor(bool direction);

    // Method to stop the motor
    void StopMotor();

    // Calibration method for the axis
    void Calibrate();

    // Method to get the current axis configuration
    AxisConfiguration GetConfiguration();
    bool CheckError(bool isRoll);
};

#endif
