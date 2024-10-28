#include "AxisCalibration.h"

Axis::Axis(int motorLeftPin, int motorRightPin, bool isRoll, Encoder* encoder, Multiplexer* multiplexerPtr)
    : motorPinLeft(motorLeftPin), motorPinRight(motorRightPin), blIsRoll(isRoll), encoder(encoder), speed(1), lastMovementTime(millis()), multiplexer(multiplexerPtr)
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
        // Serial.print(", direction:");
        // Serial.print(direction);
        // Serial.print(", lastMovementTime:");
        // Serial.print(lastMovementTime);
        // Serial.print(", lastEncoderValue:");
        // Serial.print(lastEncoderValue);
        // Serial.print(", speedIncreased:");
        // Serial.print(speedIncreased);
        // Serial.print(", speed:");
        // Serial.println(speed);

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
        // Serial.print(", direction:");
        // Serial.print(direction);
        // Serial.print(", lastMovementTime:");
        // Serial.print(lastMovementTime);
        // Serial.print(", lastEncoderValue:");
        // Serial.print(lastEncoderValue);
        // Serial.print(", speedIncreased:");
        // Serial.print(speedIncreased);
        // Serial.print(", speed:");
        // Serial.println(speed);

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
        // Serial.print(", direction:");
        // Serial.print(direction);
        // Serial.print(", lastMovementTime:");
        // Serial.print(lastMovementTime);
        // Serial.print(", lastEncoderValue:");
        // Serial.print(lastEncoderValue);
        // Serial.print(", speedIncreased:");
        // Serial.print(speedIncreased);
        // Serial.print(", speed:");
        // Serial.println(speed);

        ManageMovement( direction, lastMovementTime, lastEncoderValue, speedIncreased);

        if (millis() - lastMovementTime >= timeout) {
            StopMotor();
            #ifdef SERIAL_DEBUG
              Serial.println("Error: No movement detected for 4 seconds. Calibration aborted.");
            #endif
            config.blError = true;
            return;
        }

        if (millis() - calibrationStartTime >= calibrationTimeout) {
            StopMotor();
            #ifdef SERIAL_DEBUG
              Serial.println("Error: Calibration timeout (20 seconds). Aborted.");
            #endif
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
