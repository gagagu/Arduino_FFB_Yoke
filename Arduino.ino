/***************
  Pin setup
****************/

void ArduinoSetup() {

  // Pitch Pins
  pinMode(PITCH_EN, OUTPUT);
  pinMode(PITCH_R_PWM, OUTPUT);
  pinMode(PITCH_L_PWM, OUTPUT);

  // Roll Pins
  pinMode(ROLL_EN, OUTPUT);
  pinMode(ROLL_R_PWM, OUTPUT);
  pinMode(ROLL_L_PWM, OUTPUT);

  // Buttons Pins (Multiplexer)
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);

  pinMode(MUX_EN_YOKE, OUTPUT);
  pinMode(MUX_SIGNAL_YOKE, INPUT);

  pinMode(MUX_EN_INPUT, OUTPUT);
  pinMode(MUX_SIGNAL_INPUT, INPUT);

  // define pin default states
  // Pitch
  digitalWrite(PITCH_EN, LOW);
  digitalWrite(PITCH_R_PWM, LOW);
  digitalWrite(PITCH_L_PWM, LOW);
  //Roll
  digitalWrite(ROLL_EN, LOW);
  digitalWrite(ROLL_R_PWM, LOW);
  digitalWrite(ROLL_L_PWM, LOW);
  // Multiplexer
  digitalWrite(MUX_S0, LOW);
  digitalWrite(MUX_S1, LOW);
  digitalWrite(MUX_S2, LOW);
  digitalWrite(MUX_S3, LOW);
  digitalWrite(MUX_EN_YOKE, HIGH);
  digitalWrite(MUX_EN_INPUT, HIGH);

  // not for all Arduinos!
  // This sets the PWM Speed to maximun for noise reduction

  // Timer1: pins 9 & 10
  TCCR1B = _BV(CS10);  // change the PWM frequencey to 31.25kHz - pins 9 & 10

  // Timer4: pin 13 & 6
  TCCR4B = _BV(CS40);  // change the PWM frequencey to 31.25kHz - pin 13 & 6
}  //ArduinoSetup

/**************************
  Enables the motordrivers
****************************/
void EnableMotors() {
  digitalWrite(PITCH_EN, HIGH);
  digitalWrite(ROLL_EN, HIGH);
}  //EnableMotors

/***************************
  Disables the motordrivers
****************************/
void DisableMotors() {
  digitalWrite(PITCH_EN, LOW);
  digitalWrite(ROLL_EN, LOW);

  analogWrite(ROLL_L_PWM, 0);  // stop left
  analogWrite(ROLL_R_PWM, 0);  // stop right
  roll_speed = 0;              // speed to 0

  analogWrite(PITCH_L_PWM, 0);  // stop left
  analogWrite(PITCH_R_PWM, 0);  // stop right
  pitch_speed = 0;              // speed to 0
}  //DisableMotors

/******************************************************
  Prepare motors for movement
******************************************************/
void PrepareMotors() {
  // prepare motor for pitch direction
  MoveMotorByForce(pitch_speed,
                   (iSensorPinStates & (1 << ADJ_ENDSWITCH_PITCH_DOWN)),
                   (iSensorPinStates & (1 << ADJ_ENDSWITCH_PITCH_UP)),
                   PITCH_R_PWM,
                   PITCH_L_PWM,
                   forces[MEM_PITCH],
                   adjForceMax[MEM_PITCH],
                   adjPwmMin[MEM_PITCH],
                   adjPwmMax[MEM_PITCH]);

  // prepare motor for roll direction
  MoveMotorByForce(roll_speed,
                   (iSensorPinStates & (1 << ADJ_ENDSWITCH_ROLL_LEFT)),
                   (iSensorPinStates & (1 << ADJ_ENDSWITCH_ROLL_RIGHT)),
                   ROLL_L_PWM,
                   ROLL_R_PWM,
                   forces[MEM_ROLL],
                   adjForceMax[MEM_ROLL],
                   adjPwmMin[MEM_ROLL],
                   adjPwmMax[MEM_ROLL]);
}  //PrepareMotors

/******************************************************
  calculates the motor speeds and move
******************************************************/
void MoveMotorByForce(byte &rSpeed,
                      bool blEndswitchDown,
                      bool blEndswitchUp,
                      byte pinLPWM,
                      byte pinRPWM,
                      int16_t gForce,
                      int forceMax,
                      byte pwmMin,
                      byte pwmMax) {
  // if position is on end switch then stop the motor
  if (blEndswitchDown == 0 || blEndswitchUp == 0) {
    analogWrite(pinLPWM, 0);  // stop left
    analogWrite(pinRPWM, 0);  // stop right
    rSpeed = 0;               // speed to 0
  } else {
    // cut force to maximum value
    int pForce = constrain(abs(gForce), 0, forceMax);
    // calculate motor speed (pwm) by force between min pwm and max pwm speed
    rSpeed = map(pForce, 0, forceMax, pwmMin, pwmMax);

    // which direction?
    if (gForce > 0) {
      analogWrite(pinRPWM, 0);       // stop right
      analogWrite(pinLPWM, rSpeed);  // speed up left
    } else {
      analogWrite(pinLPWM, 0);       // stop left
      analogWrite(pinRPWM, rSpeed);  // speed up right
    }
  }
}  //MoveMotor

/******************************************
   Reads the button states over multiplexer
*******************************************/
void ReadMux() {
  iYokeButtonPinStates = 0;
  iSensorPinStates = 0;

  // for every 16 imput lines of a multiplexer
  for (byte x = 0; x < 16; x++) {
    // direct port manipulation is faster than digitalwrite!
    // s0 = Pin A0 - PF7
    if (x & B00000001)  // if bit 0 of counter is high
    {
      PORTF = PORTF | B10000000;  // set bit 7 of port (A0)
    } else {
      PORTF = PORTF & B01111111;  // clear bit 7 of port (A0)
    }

    // s1 =  Pin A1 - PF6
    if (x & B00000010)  // if bit 1 of counter is high
    {
      PORTF = PORTF | B01000000;  // set bit 6 of port (A1)
    } else {
      PORTF = PORTF & B10111111;  // clear bit 6 of port (A1)
    }

    // s2 = Pin A2 PF5
    if (x & B00000100)  // if bit 2 of counter is high
    {
      PORTF = PORTF | B00100000;  // set bit 5 of port (A2)
    } else {
      PORTF = PORTF & B11011111;  // clear bit 5 of port (A2)
    }

    // s3 = Pin A3 - PF4
    if (x & B00001000)  // if bit 3 of counter is high
    {
      PORTF = PORTF | B00010000;  // set bit 4 of port (A3)
    } else {
      PORTF = PORTF & B11101111;  // clear bit 4 of port (A3)
    }

    // enable mux 1
    PORTC = PORTC & B10111111;  // Digital Pin 5 - PortC6

    // wait for capacitors of mux to react
    delayMicroseconds(1);

    // read value
    if (x == 0) {
      iYokeButtonPinStates = digitalRead(MUX_SIGNAL_YOKE) << x;
    } else {
      iYokeButtonPinStates |= digitalRead(MUX_SIGNAL_YOKE) << x;
    }

    // disable mux1
    PORTC = PORTC | B01000000;  // Digital Pin 5 - PortC6

    // enable mux 2
    PORTD = PORTD & B11101111;  // Digital Pin 4 - PortD4

    // wait for capacitors of mux to react
    delayMicroseconds(1);

    // read value
    iSensorPinStates |= digitalRead(MUX_SIGNAL_INPUT) << x;

    if (x == 0) {
      iSensorPinStates = digitalRead(MUX_SIGNAL_INPUT) << x;
    } else {
      iSensorPinStates |= digitalRead(MUX_SIGNAL_INPUT) << x;
    }

    // disblae mux 2
    PORTD = PORTD | B00010000;  // Digital Pin 4 - PortD4

  }  //for
}  // ReadMux

/******************************************
   set button states to Joystick library
*******************************************/
void UpdateJoystickButtons() {
  // detect hat switch position
  if (iYokeButtonPinStates << 12 == 0B0000000000000000)
    Joystick.setHatSwitch(0, -1);  // no direction

  if (iYokeButtonPinStates << 12 == 0B0100000000000000)
    Joystick.setHatSwitch(0, 0);  // up

  if (iYokeButtonPinStates << 12 == 0B0101000000000000)
    Joystick.setHatSwitch(0, 45);  // up right

  if (iYokeButtonPinStates << 12 == 0B0001000000000000)
    Joystick.setHatSwitch(0, 90);  // right

  if (iYokeButtonPinStates << 12 == 0B0011000000000000)
    Joystick.setHatSwitch(0, 135);  // down right

  if (iYokeButtonPinStates << 12 == 0B0010000000000000)
    Joystick.setHatSwitch(0, 180);  // down

  if (iYokeButtonPinStates << 12 == 0B1010000000000000)
    Joystick.setHatSwitch(0, 225);  // down left

  if (iYokeButtonPinStates << 12 == 0B1000000000000000)
    Joystick.setHatSwitch(0, 270);  // left

  if (iYokeButtonPinStates << 12 == 0B1100000000000000)
    Joystick.setHatSwitch(0, 315);  // up left

  // read button states from multiplexer
  for (byte channel = 4; channel < 16; channel++) {
    Joystick.setButton(channel - 4, (iYokeButtonPinStates & (1 << channel)) >> channel);
  }
}  //updateJoystickButtons


void DoCalibration() {
  // move axis in one direction
  CalibrateAxis(ROLL_AxisConfiguration, counterRoll, ADJ_ENDSWITCH_ROLL_LEFT, ADJ_ENDSWITCH_ROLL_RIGHT, ROLL_L_PWM, ROLL_R_PWM);
  delay(1000);
  CalibrateAxis(PITCH_AxisConfiguration, counterPitch, ADJ_ENDSWITCH_PITCH_UP, ADJ_ENDSWITCH_PITCH_DOWN, PITCH_L_PWM, PITCH_R_PWM);
  
  // Serial.print("ROLL_AxisConfiguration.iMin:");
  // Serial.println(ROLL_AxisConfiguration.iMin);

  // Serial.print("ROLL_AxisConfiguration.iMax:");
  // Serial.println(ROLL_AxisConfiguration.iMax);

  // Serial.print("ROLL_AxisConfiguration.blError:");
  // Serial.println(ROLL_AxisConfiguration.blError);

  // Serial.print("ROLL_AxisConfiguration.blMotorInverted:");
  // Serial.println(ROLL_AxisConfiguration.blMotorInverted);

  // Serial.print("ROLL_AxisConfiguration.blAxisInverted:");
  // Serial.println(ROLL_AxisConfiguration.blAxisInverted);

  // Serial.print("PITCH_AxisConfiguration.iMin:");
  // Serial.println(PITCH_AxisConfiguration.iMin);

  // Serial.print("PITCH_AxisConfiguration.iMax:");
  // Serial.println(PITCH_AxisConfiguration.iMax);

  // Serial.print("PITCH_AxisConfiguration.blError:");                                                                                                                                   
  // Serial.println(PITCH_AxisConfiguration.blError);

  // Serial.print("PITCH_AxisConfiguration.blMotorInverted:");
  // Serial.println(PITCH_AxisConfiguration.blMotorInverted);

  // Serial.print("PITCH_AxisConfiguration.blAxisInverted:");
  // Serial.println(PITCH_AxisConfiguration.blAxisInverted);
  // Serial.println("");
  SetRangeJoystick();
}


void CalibrateAxis(AxisConfiguration &result, Encoder &axisEncoder, byte endSwitchBitLeft, byte endSwitchBitRight, byte motorPinLeft, byte motorPinRight) {
  byte maxMovementForFrame = 25;                  // maximim counter steps per frame
  byte maxSpeed = 75;                             // maximum speed value
  int waitDelay = 5;                              // time delay per frame
  unsigned long startTimeCalibration = millis();  // save time for the whole calibration of this axis
  unsigned long maxTimeWithoutMovement = 4000;    // maximum millis without movement
  unsigned long maxTimeForCalibration = 10000;    // maximum millis foor whole calimbration for this axis

  result.blError = false;
  result.blMotorInverted = false;
  result.blAxisInverted = false;
  result.iMin = 0;
  result.iMax = 0;

  axisEncoder.readAndReset();  // reset counter

  // Check if axis is already in end position.
  CheckIfInEndPosition(result,
                    axisEncoder,
                    endSwitchBitLeft,
                    endSwitchBitRight,
                    motorPinLeft,
                    motorPinRight,
                    startTimeCalibration,
                    maxTimeForCalibration,
                    maxTimeWithoutMovement,
                    maxSpeed,
                    waitDelay,
                    maxMovementForFrame);

  if (result.blError) {  // on error exit routine
    return;
  }


  delay(2000);


  // ***** move motor in right direction until ->any<- endswitch is reached
  CalibrationMove(result,
                  axisEncoder,
                  endSwitchBitLeft,
                  endSwitchBitRight,
                  motorPinLeft,
                  motorPinRight,
                  startTimeCalibration,
                  maxTimeForCalibration,
                  maxTimeWithoutMovement,
                  maxSpeed,
                  waitDelay,
                  maxMovementForFrame);


  if (result.blError) {  // on error exit routine
    return;
  }

  if ((iSensorPinStates & (1 << endSwitchBitLeft)) == 0)  // is the left endswitch reached instead of right?
  {                                                       // yes
    result.blMotorInverted = true;                        // than set flag to indicate morot inverted
  }

  axisEncoder.readAndReset();  // set the axis counter to zero.

  delay(500);  // give arduino some time

  // ********************* move in opposit direction until any endswitch is reached and save the counter.




  if (result.blMotorInverted) {  // when inverted switch endswitch left is right and right is left

    CalibrationMove(result,
                    axisEncoder,
                    255,  // ignore left end switch because the axis is there
                    endSwitchBitRight,
                    motorPinRight,  // twisted for opposit direction
                    motorPinLeft,   // twisted for opposit direction
                    startTimeCalibration,
                    maxTimeForCalibration,
                    maxTimeWithoutMovement,
                    maxSpeed,
                    waitDelay,
                    maxMovementForFrame);
  } else {
    CalibrationMove(result,
                    axisEncoder,
                    endSwitchBitLeft,
                    255,            // ignore right endswitch because the axis is there
                    motorPinRight,  // twisted for opposit direction
                    motorPinLeft,   // twisted for opposit direction
                    startTimeCalibration,
                    maxTimeForCalibration,
                    maxTimeWithoutMovement,
                    maxSpeed,
                    waitDelay,
                    maxMovementForFrame);
  }

  if (result.blError) {  // on error exit routine
    return;
  }


  // checkings of endstop
  if (result.blMotorInverted && ((iSensorPinStates & (1 << endSwitchBitRight)) != 0))  // when inverted the right endswitch should triggered
  {
    result.blError = true;                                                                    // error
    return;                                                                                   // break
  } else if (!result.blMotorInverted && (iSensorPinStates & (1 << endSwitchBitLeft)) != 0) {  // not inverted then left should triggered
    result.blError = true;                                                                    // not?
    return;                                                                                   // break
  }


  if (axisEncoder.read() > 0)  // is counter invetred?
  {
    result.blAxisInverted = true;  // yes
  }

  if (result.blMotorInverted) {
    result.iMin = abs(axisEncoder.read() / 2);
    result.iMax = (-1) * result.iMin;
  } else {
    result.iMax = abs(axisEncoder.read() / 2);
    result.iMin = (-1) * result.iMax;
  }

  axisEncoder.write(result.iMax);

  delay(500);

  // *************  move to center in opposit direction

  // ***** move motor in right direction until ->any<- endswitch is reched
  if (result.blMotorInverted) {  // when inverted switch endswitch left is right and right is left
    CalibrationMoveCounter(result,
                           axisEncoder,
                           endSwitchBitLeft,
                           255,            // ignore left end switch because the axis is there
                           motorPinLeft,   // twisted for opposit direction
                           motorPinRight,  // twisted for opposit direction
                           startTimeCalibration,
                           maxTimeForCalibration,
                           maxTimeWithoutMovement,
                           maxSpeed,
                           maxMovementForFrame,
                           0);
  } else {
    CalibrationMoveCounter(result,
                           axisEncoder,
                           255,  // ignore right endswitch because the axis is there
                           endSwitchBitRight,
                           motorPinLeft,   // twisted for opposit direction
                           motorPinRight,  // twisted for opposit direction
                           startTimeCalibration,
                           maxTimeForCalibration,
                           maxTimeWithoutMovement,
                           maxSpeed,
                           maxMovementForFrame,
                           0);
  }

}


void CheckIfInEndPosition(AxisConfiguration &result,
                     Encoder &axisEncoder,
                     byte endSwitchBitLeft,
                     byte endSwitchBitRight,
                     byte motorPinLeft,
                     byte motorPinRight,
                     unsigned long startTimeCalibration,
                     unsigned long maxTimeForCalibration,
                     unsigned long maxTimeWithoutMovement,
                     int maxSpeed,
                     int waitDelay,
                     int maxMovementForFrame){

    int16_t counterValue=0;

    if ((iSensorPinStates & (1 << endSwitchBitRight)) == 0)  // if right endswitch is reached?
    {
      result.blError = false;  

      CalibrationMoveOutOfEndstop(result,
                axisEncoder,
                255,
                endSwitchBitRight,
                motorPinRight,
                motorPinLeft,
                startTimeCalibration,
                maxTimeForCalibration,
                maxTimeWithoutMovement,
                maxSpeed,
                waitDelay,
                maxMovementForFrame);

      if(result.blError==true)
      {
        result.blError = false;  
        CalibrationMoveOutOfEndstop(result,
                axisEncoder,
                255,
                endSwitchBitRight,
                motorPinLeft,
                motorPinRight,
                startTimeCalibration,
                maxTimeForCalibration,
                maxTimeWithoutMovement,
                maxSpeed,
                waitDelay,
                maxMovementForFrame);              
      }
    }
    if ((iSensorPinStates & (1 << endSwitchBitLeft)) == 0)  // if left endswitch is reached?
    {
      result.blError = false;  

      CalibrationMoveOutOfEndstop(result,
                axisEncoder,
                endSwitchBitLeft,
                255,
                motorPinLeft,
                motorPinRight,
                startTimeCalibration,
                maxTimeForCalibration,
                maxTimeWithoutMovement,
                maxSpeed,
                waitDelay,
                maxMovementForFrame);

      if(result.blError==true)
      {
        result.blError = false;  
        CalibrationMoveOutOfEndstop(result,
                axisEncoder,
                endSwitchBitLeft,
                255,
                motorPinRight,
                motorPinLeft,
                startTimeCalibration,
                maxTimeForCalibration,
                maxTimeWithoutMovement,
                maxSpeed,
                waitDelay,
                maxMovementForFrame);              
      }      
    }

}

//******** main movemt routine for calibration
void CalibrationMove(AxisConfiguration &result,
                     Encoder &axisEncoder,
                     byte endSwitchBitLeft,
                     byte endSwitchBitRight,
                     byte motorPinLeft,
                     byte motorPinRight,
                     unsigned long startTimeCalibration,
                     unsigned long maxTimeForCalibration,
                     unsigned long maxTimeWithoutMovement,
                     int maxSpeed,
                     int waitDelay,
                     int maxMovementForFrame) {

  byte calibrationSpeed = 1;
  int16_t newPos = 0;
  int16_t oldPos = 0;
  int16_t diffPos = 0;
  unsigned long currentTime = 0;
  unsigned long startTimeMovement = 0;

  startTimeMovement = millis();  // save time for thi smovement

  while (true)  // do antil any endswitch is reached
  {
    delay(waitDelay);
    newPos = axisEncoder.read();     // read counter
    diffPos = abs(newPos - oldPos);  // calc movement
    currentTime = millis();          // how long no movement?


    ReadMux();  // read endswitches

    if (endSwitchBitRight != 255) {
      if ((iSensorPinStates & (1 << endSwitchBitRight)) == 0)  // if right endswitch is reched?
      {
       // Serial.println("right");
        StopMotors(motorPinLeft,motorPinRight);
        return;
      }
    }

    if (endSwitchBitLeft != 255) {
      if ((iSensorPinStates & (1 << endSwitchBitLeft)) == 0)  // if left endswitch is reached?
      {
       // Serial.println("left");
        StopMotors(motorPinLeft,motorPinRight);
        return;
      }
    }

    if (diffPos == 0)  // is there any movement
    {                  // no

      if ((currentTime - startTimeMovement) > maxTimeWithoutMovement)  // is no movement longer than max time
      {                                                                // yes
        StopMotors(motorPinLeft,motorPinRight);
       // Serial.println("no movement");
        result.blError = true;                                         // set error flag
        return;
      } else {                            // time is okay
        calibrationSpeed++;               // raise speed
        if (calibrationSpeed > maxSpeed)  // but until max speed
        {
          calibrationSpeed = maxSpeed;
        }
      }
    } else {                            //There is movement
      startTimeMovement = currentTime;  // reset start time for next frame

      if (diffPos < maxMovementForFrame)  // only x movement per frame, slower?
      {
        calibrationSpeed++;               // raise speed
        if (calibrationSpeed > maxSpeed)  // but until max speed
        {
          calibrationSpeed = maxSpeed;
        }
      } else if (diffPos > maxMovementForFrame)  // only x movement per frame, faster?
      {
        calibrationSpeed--;  // lower speed
        if (calibrationSpeed < 0) {
          calibrationSpeed = 0;
        }
      }
    }
    oldPos = newPos;                               // save pos for new frame
    analogWrite(motorPinLeft, 0);                  // move motor in right direction
    analogWrite(motorPinRight, calibrationSpeed);  // move motor

    if ((currentTime - startTimeCalibration) > maxTimeForCalibration)  // check max time for calibration of this axis
    {                    
             // Serial.println("max time");
                                              // too high
      StopMotors(motorPinLeft,motorPinRight);
      result.blError = true;                                           // set error flag
      return;
    }
  }  // while


  // stop
  StopMotors(motorPinLeft,motorPinRight);
        Serial.println("stop");

}  //CalibrationMove

//******** main movemt routine for calibration
void CalibrationMoveCounter(AxisConfiguration &result,
                            Encoder &axisEncoder,
                            byte endSwitchBitLeft,
                            byte endSwitchBitRight,
                            byte motorPinLeft,
                            byte motorPinRight,
                            unsigned long startTimeCalibration,
                            unsigned long maxTimeForCalibration,
                            unsigned long maxTimeWithoutMovement,
                            int maxSpeed,
                            int maxMovementForFrame,
                            int16_t counterValue) {

  byte calibrationSpeed = 1;
  int16_t newPos = 0;
  int16_t oldPos = 0;
  int16_t diffPos = 0;
  unsigned long currentTime = 0;
  unsigned long startTimeMovement = 0;

  byte minimumSpeed = 0;
  bool blOnce = false;

  startTimeMovement = millis();  // save time for thi smovement

  while (true)  // do antil any endswitch is reached
  {
    newPos = axisEncoder.read();     // read counter
    diffPos = abs(newPos - oldPos);  // calc movement
    currentTime = millis();          // how long no movement?


    if (abs(newPos - counterValue)==0) {
      // Serial.println("==0");
      StopMotors(motorPinLeft,motorPinRight);
       return;
    }

    ReadMux();  // read endswitches

    if (endSwitchBitRight != 255) {
      if ((iSensorPinStates & (1 << endSwitchBitRight)) == 0)  // if right endswitch is reched?
      {
      //  Serial.println("counter right");
        StopMotors(motorPinLeft,motorPinRight);
        return;
      }
    }

    if (endSwitchBitLeft != 255) {
      if ((iSensorPinStates & (1 << endSwitchBitLeft)) == 0)  // if left endswitch is reached?
      {
       // Serial.println("counter left");
        StopMotors(motorPinLeft,motorPinRight);
        return;
      }
    }

    if (diffPos == 0)  // is there any movement
    {                  // no

      if ((currentTime - startTimeMovement) > maxTimeWithoutMovement)  // is no movement longer than max time
      {                  
        //Serial.println("counter no movement");// yes
        StopMotors(motorPinLeft,motorPinRight);
        result.blError = true;                                         // set error flag
        return;
      } else {                                                         // time is okay
        calibrationSpeed++;                                            // raise speed
        if (calibrationSpeed > maxSpeed)                               // but until max speed
        {
          calibrationSpeed = maxSpeed;
        }
      }
    } else {  //There is movement
      if (blOnce == false) {
        minimumSpeed = calibrationSpeed;
        blOnce = true;
      }

      startTimeMovement = currentTime;  // reset start time for next frame

      if (abs(newPos-counterValue) <= 500) {
        calibrationSpeed--;
          if (calibrationSpeed <= minimumSpeed)  // but until max speed
          {
            calibrationSpeed = minimumSpeed;
          }
        }
    }

    oldPos = newPos;                               // save pos for new frame
    analogWrite(motorPinLeft, 0);                  // move motor in right direction
    analogWrite(motorPinRight, calibrationSpeed);  // move motor

    if ((currentTime - startTimeCalibration) > maxTimeForCalibration)  // check max time for calibration of this axis
    {                   
             // Serial.println("counter max time");
                                               // too high
      StopMotors(motorPinLeft,motorPinRight);
      result.blError = true;                                           // set error flag
      return;
    }
  }  // while

  // stop
  StopMotors(motorPinLeft,motorPinRight);
        //Serial.println("counter stop");

  delay(500);

}  //CalibrationMove



//******** main movemt routine for calibration
void CalibrationMoveOutOfEndstop(AxisConfiguration &result,
                     Encoder &axisEncoder,
                     byte endSwitchBitLeft,
                     byte endSwitchBitRight,
                     byte motorPinLeft,
                     byte motorPinRight,
                     unsigned long startTimeCalibration,
                     unsigned long maxTimeForCalibration,
                     unsigned long maxTimeWithoutMovement,
                     int maxSpeed,
                     int waitDelay,
                     int maxMovementForFrame) {

  byte calibrationSpeed = 1;
  int16_t newPos = 0;
  int16_t oldPos = 0;
  int16_t diffPos = 0;
  unsigned long currentTime = 0;
  unsigned long startTimeMovement = 0;

  startTimeMovement = millis();  // save time for thi smovement

  while (true)  // do antil any endswitch is reached
  {
    delay(waitDelay);
    newPos = axisEncoder.read();     // read counter
    diffPos = abs(newPos - oldPos);  // calc movement
    currentTime = millis();          // how long no movement?


    ReadMux();  // read endswitches

Serial.print("right");
Serial.println(((iSensorPinStates & (1 << endSwitchBitRight)) != 0));
    if (endSwitchBitRight != 255) {
      if ((iSensorPinStates & (1 << endSwitchBitRight)) != 0)  // if right endswitch is reched?
      {
        StopMotors(motorPinLeft,motorPinRight);
        return;
      }
    }

    if (endSwitchBitLeft != 255) {
      if ((iSensorPinStates & (1 << endSwitchBitLeft)) != 0)  // if left endswitch is reached?
      {
                Serial.println("left");
        StopMotors(motorPinLeft,motorPinRight);
        return;
      }
    }

    if (diffPos == 0)  // is there any movement
    {                  // no

      if ((currentTime - startTimeMovement) > maxTimeWithoutMovement)  // is no movement longer than max time
      {                                                                // yes
       Serial.println("movement");
        StopMotors(motorPinLeft,motorPinRight);
        result.blError = true;                                         // set error flag
        return;
      } else {                            // time is okay
        calibrationSpeed++;               // raise speed
        if (calibrationSpeed > maxSpeed)  // but until max speed
        {
          calibrationSpeed = maxSpeed;
        }
      }
    } else {                            //There is movement
      startTimeMovement = currentTime;  // reset start time for next frame

      if (diffPos < maxMovementForFrame)  // only x movement per frame, slower?
      {
        calibrationSpeed++;               // raise speed
        if (calibrationSpeed > maxSpeed)  // but until max speed
        {
          calibrationSpeed = maxSpeed;
        }
      } else if (diffPos > maxMovementForFrame)  // only x movement per frame, faster?
      {
        calibrationSpeed--;  // lower speed
        if (calibrationSpeed < 0) {
          calibrationSpeed = 0;
        }
      }
    }
    oldPos = newPos;                               // save pos for new frame
    analogWrite(motorPinLeft, 0);                  // move motor in right direction
    analogWrite(motorPinRight, calibrationSpeed);  // move motor

    if ((currentTime - startTimeCalibration) > maxTimeForCalibration)  // check max time for calibration of this axis
    {                                                                  // too high
      StopMotors(motorPinLeft,motorPinRight);
      result.blError = true;                                           // set error flag
      return;
    }
  }  // while


  // stop
  StopMotors(motorPinLeft,motorPinRight);

}  //CalibrationMove

void StopMotors(byte motorPinLeft, byte motorPinRight) {
  analogWrite(motorPinLeft, 0);  // stop right
  analogWrite(motorPinRight, 0);  // stop left
  delay(30);
}
