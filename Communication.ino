/******************************************
   Communication Command Constants
*******************************************/
//Debug
#define SERIAL_CMD_DEBUG_START 1
#define SERIAL_CMD_DEBUG_STOP 2
#define SERIAL_CMD_DEBUG_STATUS 3
#define SERIAL_CMD_DEBUG_VALUES 4
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

/******************************************
   Check if serial data are available and 
   command received
*******************************************/
void SerialEvent() {
  // data available?
  if (Serial.available() > 0) {
    // is first char a "!"?
    char cStart = (char)Serial.read();
    if (cStart == '!') {
      // still data available?
      if (Serial.available() > 0) {
        // read command parameter as int value (see command const above)
        int16_t cmd = Serial.parseInt();
        // value available?
        if (Serial.available() > 0) {
          // read value
          int16_t value = Serial.parseInt();
          // all above 100 is a write command
          if (cmd <= 100) {
            // process read command
            SerialProcessReadCommand(cmd, value);
          } else {
            // process write command
            SerialProcessWriteCommand(cmd, value);
          } // <=100
        }// available
      }// available
    }  // start flag !
  } // available

  // if serial debug mode is enabled the send data
  if (blSerialDebug == true) {
    // send start command
    SerialWriteStart(SERIAL_CMD_DEBUG_VALUES);
    // send all values
    CMD_READ_ALL_VALUES();
    // send all parameters
    CMD_READ_ALL_PARAMS();
    // send end flag
    SerialWriteEnd();
  }
}  //SerialEvent

/******************************************
  Process Read Command
*******************************************/
void SerialProcessReadCommand(int16_t cmd, int16_t val) {
  // checks
  if (cmd == 0)
    return;

  // choose
  switch (cmd) {
    case SERIAL_CMD_DEBUG_START:                    // start debug mode
      blSerialDebug = true;                         // set flag
      LCDPrintDebugMode();                          // set display info
      SerialWriteStart(SERIAL_CMD_READ_ALL_VALUES); // return info to sender
      SerialWriteEnd();                             // end flag
      break;
    case SERIAL_CMD_DEBUG_STOP:                     // stop debug mode
      blSerialDebug = false;                        // reset flag
      SerialWriteStart(SERIAL_CMD_DEBUG_STOP);      // return info to sender
      SerialWriteEnd();                             // end flag
      LcdPrintAdjustmendValues();                   // reset display info
      break;
    case SERIAL_CMD_READ_ALL_VALUES:                // return all measured values like speed, counter,...
      SerialWriteStart(SERIAL_CMD_READ_ALL_VALUES); // return info to sender
      CMD_READ_ALL_VALUES();                        // send values
      SerialWriteEnd();                             // end flag
      break;
    case SERIAL_CMD_READ_ALL_PARAMS:                // return all parameters like gains and effects
      SerialWriteStart(SERIAL_CMD_READ_ALL_PARAMS); // return info to sender
      CMD_READ_ALL_PARAMS();                        // send parameters
      SerialWriteEnd();                             // end flag
      break;
  }
} //SerialProcessReadCommand

/******************************************
  Process Write Command
*******************************************/
void SerialProcessWriteCommand(int16_t cmd, int16_t value) {
    // checks
  if (cmd == 0)
    return;

  // choose and set the value
  switch (cmd) {
    case SERIAL_CDM_WRITE_ROLL_FORCE_MAX:
      adjForceMax[MEM_ROLL] = value;
      break;
    case SERIAL_CDM_WRITE_ROLL_PWM_MIN:
      adjPwmMin[MEM_ROLL] = value;
      break;
    case SERIAL_CDM_WRITE_ROLL_PWM_MAX:
      adjPwmMax[MEM_ROLL] = value;
      break;

    case SERIAL_CDM_WRITE_ROLL_FRICTION_MAX_POSITION_CHANGE:
      effects[MEM_ROLL].frictionMaxPositionChange = value;
      break;
    case SERIAL_CDM_WRITE_ROLL_INERTIA_MAX_ACCELERATION:
      effects[MEM_ROLL].inertiaMaxAcceleration = value;
      break;
    case SERIAL_CDM_WRITE_ROLL_DAMPER_MAX_VELOCITY:
      effects[MEM_ROLL].damperMaxVelocity = value;
      break;

    case SERIAL_CDM_WRITE_ROLL_TOTAL_GAIN:
      gains[MEM_ROLL].totalGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_CONSTANT_GAIN:
      gains[MEM_ROLL].constantGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_RAMP_GAIN:
      gains[MEM_ROLL].rampGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_SQUARE_GAIN:
      gains[MEM_ROLL].squareGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_SINE_GAIN:
      gains[MEM_ROLL].sineGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_TRIANGLE_GAIN:
      gains[MEM_ROLL].triangleGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_SAWTOOTH_DOWN_GAIN:
      gains[MEM_ROLL].sawtoothdownGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_SAWTOOTH_UP_GAIN:
      gains[MEM_ROLL].sawtoothupGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_SPRING_GAIN:
      gains[MEM_ROLL].springGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_DAMPER_GAIN:
      gains[MEM_ROLL].damperGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_INERTIA_GAIN:
      gains[MEM_ROLL].inertiaGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_ROLL_FRICTION_GAIN:
      gains[MEM_ROLL].frictionGain = (byte)value;
      break;

    case SERIAL_CDM_WRITE_PITCH_FORCE_MAX:
      adjForceMax[MEM_PITCH] = value;
      break;
    case SERIAL_CDM_WRITE_PITCH_PWM_MIN:
      adjPwmMin[MEM_PITCH] = value;
      break;
    case SERIAL_CDM_WRITE_PITCH_PWM_MAX:
      adjPwmMax[MEM_PITCH] = value;
      break;

    case SERIAL_CDM_WRITE_PITCH_FRICTION_MAX_POSITION_CHANGE:
      effects[MEM_PITCH].frictionMaxPositionChange = value;
      break;
    case SERIAL_CDM_WRITE_PITCH_INERTIA_MAX_ACCELERATION:
      effects[MEM_PITCH].inertiaMaxAcceleration = value;
      break;
    case SERIAL_CDM_WRITE_PITCH_DAMPER_MAX_VELOCITY:
      effects[MEM_PITCH].damperMaxVelocity = value;
      break;

    case SERIAL_CDM_WRITE_PITCH_TOTAL_GAIN:
      gains[MEM_PITCH].totalGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_CONSTANT_GAIN:
      gains[MEM_PITCH].constantGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_RAMP_GAIN:
      gains[MEM_PITCH].rampGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_SQUARE_GAIN:
      gains[MEM_PITCH].squareGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_SINE_GAIN:
      gains[MEM_PITCH].sineGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_TRIANGLE_GAIN:
      gains[MEM_PITCH].triangleGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_SAWTOOTH_DOWN_GAIN:
      gains[MEM_PITCH].sawtoothdownGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_SAWTOOTH_UP_GAIN:
      gains[MEM_PITCH].sawtoothupGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_SPRING_GAIN:
      gains[MEM_PITCH].springGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_DAMPER_GAIN:
      gains[MEM_PITCH].damperGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_INERTIA_GAIN:
      gains[MEM_PITCH].inertiaGain = (byte)value;
      break;
    case SERIAL_CDM_WRITE_PITCH_FRICTION_GAIN:
      gains[MEM_PITCH].frictionGain = (byte)value;
      break;
    case SERIAL_CMD_WRITE_DATA_EEPROM:
      CMD_WRITE_DATA_EEPROM();
      break;
    case SERIAL_CMD_WRITE_EEPROM_CLEAR:
      CMD_WRITE_EEPROM_CLEAR();
      break;
  }

  
  setGains();                // set gains for joystick
  
  SerialWriteStart(cmd);     // return info to sender
  SerialWriteEnd();          // End flag
  
  LcdPrintAdjustmendValues(); // update LCD display
} //SerialProcessWriteCommand


/******************************************
  send all values to Serial
*******************************************/
void CMD_READ_ALL_VALUES() {

  for (byte channel = 0; channel < 16; channel++) {
    SerialWriteValue(iYokeButtonPinStates & (1 << channel));    // Index 0 - 15
  }
  for (byte channel = 0; channel < 16; channel++) {
    SerialWriteValue(iSensorPinStates & (1 << channel));        // Index 16 - 31
  }

  SerialWriteValue(JOYSTICK_minX);                              // 32
  SerialWriteValue(JOYSTICK_maxX);                              // 33
  SerialWriteValue(counterRoll.read());                         // 34
  SerialWriteValue(roll_speed);                                 // 35
  SerialWriteValue(forces[MEM_ROLL]);                           // 36

  SerialWriteValue(JOYSTICK_minY);                              // 37
  SerialWriteValue(JOYSTICK_maxY);                              // 38
  SerialWriteValue(counterPitch.read());                        // 39
  SerialWriteValue(pitch_speed);                                // 40
  SerialWriteValue(forces[MEM_PITCH]);                          // 41
}

/******************************************
  send all parameters to Serial
*******************************************/
void CMD_READ_ALL_PARAMS() {                                     //Index:
  SerialWriteValue(adjForceMax[MEM_PITCH]);                      //0
  SerialWriteValue(adjPwmMin[MEM_PITCH]);                        //1
  SerialWriteValue(adjPwmMax[MEM_PITCH]);                        //2

  SerialWriteValue(adjForceMax[MEM_ROLL]);                       //3
  SerialWriteValue(adjPwmMin[MEM_ROLL]);                         //4
  SerialWriteValue(adjPwmMax[MEM_ROLL]);                         //5

  SerialWriteValue(gains[MEM_ROLL].totalGain);                    //6
  SerialWriteValue(gains[MEM_ROLL].constantGain);                 //7
  SerialWriteValue(gains[MEM_ROLL].rampGain);                     //8
  SerialWriteValue(gains[MEM_ROLL].squareGain);                   //9
  SerialWriteValue(gains[MEM_ROLL].sineGain);                     //10
  SerialWriteValue(gains[MEM_ROLL].triangleGain);                 //11
  SerialWriteValue(gains[MEM_ROLL].sawtoothdownGain);             //12
  SerialWriteValue(gains[MEM_ROLL].sawtoothupGain);               //13
  SerialWriteValue(gains[MEM_ROLL].springGain);                   //14
  SerialWriteValue(gains[MEM_ROLL].damperGain);                   //15
  SerialWriteValue(gains[MEM_ROLL].inertiaGain);                  //16
  SerialWriteValue(gains[MEM_ROLL].frictionGain);                 //17

  SerialWriteValue(effects[MEM_ROLL].frictionMaxPositionChange);  //18
  SerialWriteValue(effects[MEM_ROLL].inertiaMaxAcceleration);     //19
  SerialWriteValue(effects[MEM_ROLL].damperMaxVelocity);          //20

  SerialWriteValue(gains[MEM_PITCH].totalGain);                   //21
  SerialWriteValue(gains[MEM_PITCH].constantGain);                //22
  SerialWriteValue(gains[MEM_PITCH].rampGain);                    //23
  SerialWriteValue(gains[MEM_PITCH].squareGain);                  //24
  SerialWriteValue(gains[MEM_PITCH].sineGain);                    //25
  SerialWriteValue(gains[MEM_PITCH].triangleGain);                //26
  SerialWriteValue(gains[MEM_PITCH].sawtoothdownGain);            //27
  SerialWriteValue(gains[MEM_PITCH].sawtoothupGain);              //28
  SerialWriteValue(gains[MEM_PITCH].springGain);                  //29
  SerialWriteValue(gains[MEM_PITCH].damperGain);                  //30
  SerialWriteValue(gains[MEM_PITCH].inertiaGain);                 //31
  SerialWriteValue(gains[MEM_PITCH].frictionGain);                //32

  SerialWriteValue(effects[MEM_PITCH].frictionMaxPositionChange); //33
  SerialWriteValue(effects[MEM_PITCH].inertiaMaxAcceleration);    //34
  SerialWriteValue(effects[MEM_PITCH].damperMaxVelocity);         //35
}


/******************************************
  Writes a Start command to the serial interface
*******************************************/
void SerialWriteStart(byte command) {
  bSerialIndex = 0;       // start index i alway 0 on start command
  Serial.print("!");      // a command is indicated by "!"
  Serial.print(command);  // command constant
  Serial.print("|");      // command and values are seperated with "|"
}

/******************************************
  Writes a End command to the serial interface
*******************************************/
void SerialWriteEnd() {
  Serial.println("");
}

/******************************************
  Writes a Value to the serial interface
*******************************************/
void SerialWriteValue(int16_t value) {
  Serial.print(bSerialIndex);
  Serial.print(":");          // index and value seperated with ":"
  Serial.print(value);        
  Serial.print(",");          // value and value seperated with ","
  bSerialIndex++;             // increment index
}
