/******************************************
   Check if serial data are available and 
   command received
*******************************************/
void SerialEvent() {
  if (Serial.available() > 0) {                       // data available?
    char cStart = (char)Serial.read();                // is first char a "!"?
     if (cStart == '!' && Serial.available() > 0) {                             // still data available?
      int16_t cmd = Serial.parseInt();                // read command parameter as int value (see command const above)
      if (Serial.available() > 0) {                   // value available?
        int16_t value = Serial.parseInt();            // read value
        if (cmd <= 100) {                             // all above 100 is a write command
          SerialProcessReadCommand(cmd, value);       // process read command
        } else {
          SerialProcessWriteCommand(cmd, value);      // process write command
        }                                             // <=100
      }                                               // available
    }                                                 // start flag !
  }                                                   // available

  // if serial debug mode is enabled the send data
  if (blSerialDebug == true) {
    SerialWriteStart(SERIAL_CMD_DEBUG_VALUES);    // send start command
    CMD_READ_ALL_VALUES();       // send all values
    CMD_READ_ALL_PARAMS();    // send all parameters
    SerialWriteEnd();     // send end flag
  }
}  //SerialEvent

/******************************************
  Process Read Command
*******************************************/
void SerialProcessReadCommand(int16_t cmd, int16_t val) {
  // checks
  if (cmd == 0) return;

  // choose
  SerialWriteStart(SERIAL_CMD_READ_ALL_VALUES); // return info to sender
  switch (cmd) {
    case SERIAL_CMD_DEBUG_START:                    // start debug mode
      blSerialDebug = true;                         // set flag
      break;
    case SERIAL_CMD_DEBUG_STOP:                     // stop debug mode
      blSerialDebug = false;                        // reset flag
      break;
    case SERIAL_CMD_READ_ALL_VALUES:                // return all measured values like speed, counter,...
      CMD_READ_ALL_VALUES();                        // send values
      break;
    case SERIAL_CMD_READ_ALL_PARAMS:                // return all parameters like gains and effects
      CMD_READ_ALL_PARAMS();                        // send parameters
      break;
  }
  SerialWriteEnd();                             // end flag
} //SerialProcessReadCommand



/******************************************
  Prozessbefehle zum Schreiben
*******************************************/
void SerialProcessWriteCommand(int16_t cmd, int16_t value) {
    if (cmd == 0) return;

    switch (cmd) {
        case SERIAL_CMD_WRITE_DATA_EEPROM:
            CMD_WRITE_DATA_EEPROM();
            break;
        case SERIAL_CMD_WRITE_EEPROM_CLEAR:
            CMD_WRITE_EEPROM_CLEAR();
            break;
        default:
            handleGainCommands(cmd, value);
            break;
    }

    SetGains();
    SerialWriteStart(cmd);
    SerialWriteEnd();
}

/******************************************
  Hilfe-Funktion für Gain-Befehle
*******************************************/
void handleGainCommands(int16_t cmd, int16_t value) {
    uint8_t memIndex = (cmd < SERIAL_CDM_WRITE_PITCH_FORCE_MAX) ? MEM_ROLL : MEM_PITCH;
    switch (cmd) {
        case SERIAL_CDM_WRITE_ROLL_FORCE_MAX:
        case SERIAL_CDM_WRITE_PITCH_FORCE_MAX:
            adjForceMax[memIndex] = value;
            break;
        case SERIAL_CDM_WRITE_ROLL_PWM_MIN:
        case SERIAL_CDM_WRITE_PITCH_PWM_MIN:
            adjPwmMin[memIndex] = value;
            break;
        case SERIAL_CDM_WRITE_ROLL_PWM_MAX:
        case SERIAL_CDM_WRITE_PITCH_PWM_MAX:
            adjPwmMax[memIndex] = value;
            break;
        case SERIAL_CDM_WRITE_ROLL_FRICTION_MAX_POSITION_CHANGE:
        case SERIAL_CDM_WRITE_PITCH_FRICTION_MAX_POSITION_CHANGE:
            effects[memIndex].frictionMaxPositionChange = value;
            break;
        case SERIAL_CDM_WRITE_ROLL_INERTIA_MAX_ACCELERATION:
        case SERIAL_CDM_WRITE_PITCH_INERTIA_MAX_ACCELERATION:
            effects[memIndex].inertiaMaxAcceleration = value;
            break;
        case SERIAL_CDM_WRITE_ROLL_DAMPER_MAX_VELOCITY:
        case SERIAL_CDM_WRITE_PITCH_DAMPER_MAX_VELOCITY:
            effects[memIndex].damperMaxVelocity = value;
            break;
        default:
            setGainCommands(memIndex, cmd, value);
            break;
    }
}

/******************************************
  Hilfe-Funktion für Gain-Befehle
*******************************************/
void setGainCommands(uint8_t memIndex, int16_t cmd, int16_t value) {
    Gains& g = gains[memIndex];
    switch (cmd) {
        case SERIAL_CDM_WRITE_ROLL_TOTAL_GAIN:
        case SERIAL_CDM_WRITE_PITCH_TOTAL_GAIN:
            g.totalGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_CONSTANT_GAIN:
        case SERIAL_CDM_WRITE_PITCH_CONSTANT_GAIN:
            g.constantGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_RAMP_GAIN:
        case SERIAL_CDM_WRITE_PITCH_RAMP_GAIN:
            g.rampGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_SQUARE_GAIN:
        case SERIAL_CDM_WRITE_PITCH_SQUARE_GAIN:
            g.squareGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_SINE_GAIN:
        case SERIAL_CDM_WRITE_PITCH_SINE_GAIN:
            g.sineGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_TRIANGLE_GAIN:
        case SERIAL_CDM_WRITE_PITCH_TRIANGLE_GAIN:
            g.triangleGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_SAWTOOTH_DOWN_GAIN:
        case SERIAL_CDM_WRITE_PITCH_SAWTOOTH_DOWN_GAIN:
            g.sawtoothdownGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_SAWTOOTH_UP_GAIN:
        case SERIAL_CDM_WRITE_PITCH_SAWTOOTH_UP_GAIN:
            g.sawtoothupGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_SPRING_GAIN:
        case SERIAL_CDM_WRITE_PITCH_SPRING_GAIN:
            g.springGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_DAMPER_GAIN:
        case SERIAL_CDM_WRITE_PITCH_DAMPER_GAIN:
            g.damperGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_INERTIA_GAIN:
        case SERIAL_CDM_WRITE_PITCH_INERTIA_GAIN:
            g.inertiaGain = (byte)value;
            break;
        case SERIAL_CDM_WRITE_ROLL_FRICTION_GAIN:
        case SERIAL_CDM_WRITE_PITCH_FRICTION_GAIN:
            g.frictionGain = (byte)value;
            break;
    }
}

/******************************************
  send all values to Serial
*******************************************/
void CMD_READ_ALL_VALUES() {
    // Alle Buttonzustände
    for (byte channel = 0; channel < 16; channel++) {
        //SerialWriteValue(iYokeButtonPinStates & (1 << channel)); // Index 0 - 15
    }

    // Alle Sensordaten
    for (byte channel = 0; channel < 16; channel++) {
        //SerialWriteValue(iSensorPinStates & (1 << channel)); // Index 16 - 31
    }

    // Roll-Achsenkonfiguration und Messwerte
    SerialWriteValue(rollConfig.iMin); // 32
    SerialWriteValue(rollConfig.iMax); // 33
    SerialWriteValue(counterRoll.read()); // 34
    SerialWriteValue(roll_speed); // 35
    SerialWriteValue(forces[MEM_ROLL]); // 36

    // Pitch-Achsenkonfiguration und Messwerte
    SerialWriteValue(pitchConfig.iMin); // 37
    SerialWriteValue(pitchConfig.iMax); // 38
    SerialWriteValue(counterPitch.read()); // 39
    SerialWriteValue(pitch_speed); // 40
    SerialWriteValue(forces[MEM_PITCH]); // 41
}

/******************************************
  send all parameters to Serial
*******************************************/
void CMD_READ_ALL_PARAMS() {
    // Anpassungswerte für Pitch
    SerialWriteValue(adjForceMax[MEM_PITCH]); // 0
    SerialWriteValue(adjPwmMin[MEM_PITCH]);   // 1
    SerialWriteValue(adjPwmMax[MEM_PITCH]);   // 2

    // Anpassungswerte für Roll
    SerialWriteValue(adjForceMax[MEM_ROLL]); // 3
    SerialWriteValue(adjPwmMin[MEM_ROLL]);   // 4
    SerialWriteValue(adjPwmMax[MEM_ROLL]);   // 5

    // Gains für Roll
    for (byte i = 0; i < sizeof(gains[MEM_ROLL]) / sizeof(gains[MEM_ROLL].totalGain); i++) {
        SerialWriteValue(*((byte*)&gains[MEM_ROLL] + i)); // 6 - 17
    }

    // Effekte für Roll
    SerialWriteValue(effects[MEM_ROLL].frictionMaxPositionChange); // 18
    SerialWriteValue(effects[MEM_ROLL].inertiaMaxAcceleration);    // 19
    SerialWriteValue(effects[MEM_ROLL].damperMaxVelocity);         // 20

    // Gains für Pitch
    for (byte i = 0; i < sizeof(gains[MEM_PITCH]) / sizeof(gains[MEM_PITCH].totalGain); i++) {
        SerialWriteValue(*((byte*)&gains[MEM_PITCH] + i)); // 21 - 32
    }

    // Effekte für Pitch
    SerialWriteValue(effects[MEM_PITCH].frictionMaxPositionChange); // 33
    SerialWriteValue(effects[MEM_PITCH].inertiaMaxAcceleration);    // 34
    SerialWriteValue(effects[MEM_PITCH].damperMaxVelocity);         // 35
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
