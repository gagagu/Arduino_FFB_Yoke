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

#include "AxisCalibration.h"

Axis::Axis(int motorLeftPin, int motorRightPin, bool isRoll, Encoder* encoderPtr, Multiplexer* multiplexerPtr, BeepManager* beepManagerPtr)
    : motorPinLeft(motorLeftPin), motorPinRight(motorRightPin), blIsRoll(isRoll), encoder(encoderPtr), speed(1), lastMovementTime(millis()), multiplexer(multiplexerPtr), beepManager(beepManagerPtr)
{
}

// Method to move the motor in a given direction
void Axis::MoveMotor(bool direction) {
    if (direction) {
        analogWrite(motorPinLeft, 0);
        analogWrite(motorPinRight, speed);
    } else {
        analogWrite(motorPinLeft, speed);
        analogWrite(motorPinRight, 0);
    }
}

// Method to stop the motor
void Axis::StopMotor() {
    analogWrite(motorPinLeft, 0);
    analogWrite(motorPinRight, 0);
    delay(waitDelayMotorStops);  // Small delay to ensure motor completely stops
}

// Helper method to manage motor movement
void Axis::ManageMovement( bool direction, unsigned long& lastMovementTime, int& lastEncoderValue, bool& speedIncreased) {
    ReadMultiplexer(); // Update end switch states
    int currentEncoderValue = encoder->read();

    // Check for speed increase
    if (abs(currentEncoderValue - lastEncoderValue)<=20) {
        if (speed < maxSpeed) speed++;  // Increment speed by 1
    } else {
        lastMovementTime = millis();
        if (!speedIncreased) {
            speed += speedIncrement;  // Increase speed by defined increment
            if (speed > maxSpeed) speed = maxSpeed;  // Ensure speed doesn't exceed maxSpeed
            speedIncreased = true;  // Flag for speed increase
        }
    }
    lastEncoderValue = currentEncoderValue;
    MoveMotor(direction);
}

// read multiplexer for end stops
void Axis::ReadMultiplexer(){
    multiplexer->ReadMux();  // Update end switch states
    if(blIsRoll)
    {
      blEndSwitchLeft=multiplexer->EndSwitchRollLeft();
      blEndSwitchRight=multiplexer->EndSwitchRollRight();
    }else{
      blEndSwitchLeft=multiplexer->EndSwitchPitchUp();
      blEndSwitchRight=multiplexer->EndSwitchPitchDown();
    }
}

// reset the encoder counter
int Axis::ResetEncoder(){
  encoder->write(0);
  return 0;
}

// Check Timeouts
bool Axis::CheckTimeouts(unsigned long lastMovementTime, unsigned long calibrationStartTime)
{
  // Movement Timeout?
  if (millis() - lastMovementTime >= timeout) {
      StopMotor();
      config.blError = true;
      config.blAxisTimeout = true;
      return true;
  }

  // General Timeout?
  if (millis() - calibrationStartTime >= calibrationTimeout) {
      StopMotor();
      config.blError = true;
      config.blTimeout=true;
      return true;
  }

  return false;
}

// Calibration method for the axis
void Axis::Calibrate() {
    config = {false, 0, 0, false, false, false, false}; // Reset config
    calibrationStartTime = millis();  // Mark the calibration start time
    int lastEncoderValue = ResetEncoder();
    bool direction = true;  // Start by moving in the positive direction
    bool speedIncreased = false;  // Flag for speed increase

    speed = 1;
    ReadMultiplexer();  // Update end switch states
    StopMotor();  // Stop the motor once an end switch is hit     

    //**********************************************************************
    // Step 1: Move away from the end switch if the axis is at an end switch
    //**********************************************************************
    if (blEndSwitchLeft || blEndSwitchRight) {
        lastMovementTime = millis();
        while (blEndSwitchLeft || blEndSwitchRight) {
            ManageMovement(direction, lastMovementTime, lastEncoderValue, speedIncreased);

            if(CheckTimeouts(lastMovementTime, calibrationStartTime))
              break;

            delay(whileDelay);
        }
        StopMotor();  // Stop the motor after leaving the end switch
        delay(waitDelayAfterMoveOutEndstop);   // Short delay for stabilization
    }

    // On error leave
    if(config.blError)
    {
      return;
    }

    //**********************************************************************
    // Step 2: Move towards the first end switch
    //**********************************************************************
    speed=1;
    lastMovementTime = millis();
    lastEncoderValue = ResetEncoder();
    speedIncreased = false;  // Reset for next loop

    while (!blEndSwitchLeft && !blEndSwitchRight) {
        ManageMovement(direction, lastMovementTime, lastEncoderValue, speedIncreased);

        if(CheckTimeouts(lastMovementTime, calibrationStartTime))
          break;

        delay(whileDelay);
    }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
    StopMotor();  // Stop the motor once an end switch is hit     

    // Determine which end switch was triggered and set motor inversion if necessary
    if(blEndSwitchLeft)
    {
      config.blMotorInverted = true;  
      config.blError = true;
    }

    // On error leave
    if(config.blError)
    {
      return;
    }

    //**********************************************************************
    // Step 3: Move to the opposite end switch
    //**********************************************************************
    direction = !direction;  // Move in the opposite direction

    delay(1000);
    speed=1;
    lastMovementTime = millis();
    lastEncoderValue = ResetEncoder();
    speedIncreased = false;  // Reset for next loop

    while (!blEndSwitchLeft) {
        ManageMovement(direction, lastMovementTime, lastEncoderValue, speedIncreased);

        if(CheckTimeouts(lastMovementTime, calibrationStartTime))
          break;

        delay(whileDelay);
    }
    StopMotor();  // Stop the motor once an end switch is hit


    if(encoder->read()<0) 
    {
      config.blEncoderInverted = true;
      config.blError = true;
    }

    // On error leave
    if(config.blError)
    {
      return;
    }

    // Calculate iMin and iMax based on the encoder value
    config.iMax = (encoder->read() / 2);  // Set iMax as half of the maximum encoder value
    config.iMin = -config.iMax;            // Set iMin as the negative value of iMax
    encoder->write(config.iMax);  // Set the encoder to the iMax value at the end of the calibration

  // Serial.print(", Min1:");      
  // Serial.print(config.iMin);
  // Serial.print(", Max1:");
  // Serial.print(config.iMax);  
  // Serial.println("");

    // Step 4: Move back until the encoder reaches 0
    delay(1000);
    speed = 1;
    direction = !direction;  // Move in the opposite direction
    lastMovementTime = millis();
    lastEncoderValue = config.iMax;

    // Check the sign of the last encoder value
    bool isLastValuePositive = (lastEncoderValue > 0);

    //**********************************************************************
    // 4. Move axis to the middle
    //**********************************************************************
    while (((isLastValuePositive && encoder->read() >= 0) || (!isLastValuePositive && encoder->read() <= 0)) && !blEndSwitchRight) {
        ManageMovement( direction, lastMovementTime, lastEncoderValue, speedIncreased);

        if (millis() - lastMovementTime >= timeout) {
            StopMotor();
            config.blError = true;
            return;
        }

        if (millis() - calibrationStartTime >= calibrationTimeout) {
            StopMotor();
            config.blError = true;
            return;
        }

        delay(whileDelay);
    }

    // Stop the motor when the encoder reaches 0
    StopMotor();
}

// Method to get the current axis configuration
AxisConfiguration Axis::GetConfiguration() {
    return config;
}

bool Axis::CheckError(bool isRoll){
    if (config.blError) {
      // error occured
      beepManager->CalibrationError();  // Calibration error beep
      delay(BEEP_CODE_DELAY);
      // switch error and give beep codes
      if (config.blEncoderInverted)
        beepManager->CalibrationEncoderInverted(isRoll);

      if (config.blMotorInverted)
        beepManager->CalibrationMotorInverted(isRoll);

      if (config.blAxisTimeout)
        beepManager->CalibrationTimeoutMotor(isRoll);

      if (config.blTimeout)
        beepManager->CalibrationTimeoutGeneral(isRoll);
    }

    return config.blError;
}
