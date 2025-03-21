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
  SerialWriteStart(cmd); // return info to sender
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
      
        SerialWriteValue(mux.GetYokeButtonPinStates() & (1 << channel)); // Index 0 - 15
    }

    // Alle Sensordaten
    for (byte channel = 0; channel < 16; channel++) {
        SerialWriteValue(mux.GetSensorPinStates() & (1 << channel)); // Index 16 - 31
    }

    // Roll-Achsenkonfiguration und Messwerte
    SerialWriteValue( rollAxis.GetConfiguration().iMin); // 32
    SerialWriteValue( rollAxis.GetConfiguration().iMax); // 33
    SerialWriteValue(counterRoll.read()); // 34
    SerialWriteValue(roll_speed); // 35
    SerialWriteValue(forces[MEM_ROLL]); // 36

    // Pitch-Achsenkonfiguration und Messwerte
    SerialWriteValue( pitchAxis.GetConfiguration().iMin); // 37
    SerialWriteValue( pitchAxis.GetConfiguration().iMax); // 38
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

    // Effekte für Roll
    SerialWriteValue(effects[MEM_ROLL].frictionMaxPositionChange); // 18
    SerialWriteValue(effects[MEM_ROLL].inertiaMaxAcceleration);    // 19
    SerialWriteValue(effects[MEM_ROLL].damperMaxVelocity);         // 20


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
