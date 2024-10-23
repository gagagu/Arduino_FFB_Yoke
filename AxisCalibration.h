#ifndef AXIS_CALIBRATION_H
#define AXIS_CALIBRATION_H

#include <Arduino.h>
#include "Multiplexer.h"
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
    //Axis(int motorLeftPin, int motorRightPin, bool* endSwitchLeft, bool* endSwitchRight, Encoder* encoder, Multiplexer* multiplexerPtr);
    Axis(int motorLeftPin, int motorRightPin, bool isRoll, Encoder* encoder, Multiplexer* multiplexerPtr);

    // Method to move the motor in a given direction
    void MoveMotor(bool direction);

    // Method to stop the motor
    void StopMotor();

    // Calibration method for the axis
    void Calibrate();

    // Method to get the current axis configuration
    AxisConfiguration GetConfiguration();
};

#endif
