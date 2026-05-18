/*
 Created by A.Eckers aka Gagagu – improved version
 http://www.gagagu.de / https://github.com/gagagu/Arduino_FFB_Yoke
*/

// Check for incoming serial data and dispatch commands.
// Protocol: !<cmd><value>\n   e.g.  !101 255
void SerialEvent() {
  if (Serial.available() > 0) {
    char cStart = (char)Serial.read();
    if (cStart == '!' && Serial.available() > 0) {
      int16_t cmd = Serial.parseInt();
      if (Serial.available() > 0) {
        int16_t value = Serial.parseInt();
        if (cmd <= 100) {
          SerialProcessReadCommand(cmd, value);
        } else {
          SerialProcessWriteCommand(cmd, value);
        }
      }
    }
  }
  if (blSerialDebug) {
    SerialWriteStart(SERIAL_CMD_DEBUG_VALUES);
    CMD_READ_ALL_VALUES();
    CMD_READ_ALL_PARAMS();
    SerialWriteEnd();
  }
}

void SerialProcessReadCommand(int16_t cmd, int16_t val) {
  if (cmd == 0) return;
  SerialWriteStart(cmd);
  switch (cmd) {
    case SERIAL_CMD_DEBUG_START:  blSerialDebug = true;  break;
    case SERIAL_CMD_DEBUG_STOP:   blSerialDebug = false; break;
    case SERIAL_CMD_READ_ALL_VALUES: CMD_READ_ALL_VALUES(); break;
    case SERIAL_CMD_READ_ALL_PARAMS: CMD_READ_ALL_PARAMS(); break;
  }
  SerialWriteEnd();
}

void SerialProcessWriteCommand(int16_t cmd, int16_t value) {
  if (cmd == 0) return;

  // Determine axis: commands < SERIAL_CMD_WRITE_PITCH_FORCE_MAX are Roll
  uint8_t ax = (cmd < SERIAL_CMD_WRITE_PITCH_FORCE_MAX) ? MEM_ROLL : MEM_PITCH;

  switch (cmd) {
    // ── EEPROM ──────────────────────────────────────────────────────────────
    case SERIAL_CMD_WRITE_DATA_EEPROM:  CMD_WRITE_DATA_EEPROM();  break;
    case SERIAL_CMD_WRITE_EEPROM_CLEAR: CMD_WRITE_EEPROM_CLEAR(); break;

    // ── Motor / force limits ─────────────────────────────────────────────────
    case SERIAL_CMD_WRITE_ROLL_FORCE_MAX:
    case SERIAL_CMD_WRITE_PITCH_FORCE_MAX:               adjForceMax[ax] = value; break;
    case SERIAL_CMD_WRITE_ROLL_PWM_MIN:
    case SERIAL_CMD_WRITE_PITCH_PWM_MIN:                 adjPwmMin[ax]   = value; break;
    case SERIAL_CMD_WRITE_ROLL_PWM_MAX:
    case SERIAL_CMD_WRITE_PITCH_PWM_MAX:                 adjPwmMax[ax]   = value; break;

    // ── Effect params ────────────────────────────────────────────────────────
    case SERIAL_CMD_WRITE_ROLL_FRICTION_MAX_POS_CHANGE:
    case SERIAL_CMD_WRITE_PITCH_FRICTION_MAX_POS_CHANGE: effects[ax].frictionMaxPositionChange = value; break;
    case SERIAL_CMD_WRITE_ROLL_INERTIA_MAX_ACCEL:
    case SERIAL_CMD_WRITE_PITCH_INERTIA_MAX_ACCEL:       effects[ax].inertiaMaxAcceleration    = value; break;
    case SERIAL_CMD_WRITE_ROLL_DAMPER_MAX_VELOCITY:
    case SERIAL_CMD_WRITE_PITCH_DAMPER_MAX_VELOCITY:     effects[ax].damperMaxVelocity         = value; break;

    // ── Gains ────────────────────────────────────────────────────────────────
    case SERIAL_CMD_WRITE_ROLL_TOTAL_GAIN:
    case SERIAL_CMD_WRITE_PITCH_TOTAL_GAIN:         gains[ax].totalGain        = value; break;
    case SERIAL_CMD_WRITE_ROLL_CONSTANT_GAIN:
    case SERIAL_CMD_WRITE_PITCH_CONSTANT_GAIN:      gains[ax].constantGain     = value; break;
    case SERIAL_CMD_WRITE_ROLL_RAMP_GAIN:
    case SERIAL_CMD_WRITE_PITCH_RAMP_GAIN:          gains[ax].rampGain         = value; break;
    case SERIAL_CMD_WRITE_ROLL_SQUARE_GAIN:
    case SERIAL_CMD_WRITE_PITCH_SQUARE_GAIN:        gains[ax].squareGain       = value; break;
    case SERIAL_CMD_WRITE_ROLL_SINE_GAIN:
    case SERIAL_CMD_WRITE_PITCH_SINE_GAIN:          gains[ax].sineGain         = value; break;
    case SERIAL_CMD_WRITE_ROLL_TRIANGLE_GAIN:
    case SERIAL_CMD_WRITE_PITCH_TRIANGLE_GAIN:      gains[ax].triangleGain     = value; break;
    case SERIAL_CMD_WRITE_ROLL_SAWTOOTH_DOWN_GAIN:
    case SERIAL_CMD_WRITE_PITCH_SAWTOOTH_DOWN_GAIN: gains[ax].sawtoothdownGain = value; break;
    case SERIAL_CMD_WRITE_ROLL_SAWTOOTH_UP_GAIN:
    case SERIAL_CMD_WRITE_PITCH_SAWTOOTH_UP_GAIN:   gains[ax].sawtoothupGain   = value; break;
    case SERIAL_CMD_WRITE_ROLL_SPRING_GAIN:
    case SERIAL_CMD_WRITE_PITCH_SPRING_GAIN:        gains[ax].springGain       = value; break;
    case SERIAL_CMD_WRITE_ROLL_DAMPER_GAIN:
    case SERIAL_CMD_WRITE_PITCH_DAMPER_GAIN:        gains[ax].damperGain       = value; break;
    case SERIAL_CMD_WRITE_ROLL_INERTIA_GAIN:
    case SERIAL_CMD_WRITE_PITCH_INERTIA_GAIN:       gains[ax].inertiaGain      = value; break;
    case SERIAL_CMD_WRITE_ROLL_FRICTION_GAIN:
    case SERIAL_CMD_WRITE_PITCH_FRICTION_GAIN:      gains[ax].frictionGain     = value; break;
  }

  SetGains();
  SerialWriteStart(cmd);
  SerialWriteEnd();
}

void CMD_READ_ALL_VALUES() {
  for (byte ch = 0; ch < 16; ch++) SerialWriteValue(mux.GetYokeButtonPinStates() & (1 << ch));
  for (byte ch = 0; ch < 16; ch++) SerialWriteValue(mux.GetSensorPinStates()     & (1 << ch));
  SerialWriteValue(rollAxis.GetConfiguration().iMin);
  SerialWriteValue(rollAxis.GetConfiguration().iMax);
  SerialWriteValue(counterRoll.read());
  SerialWriteValue(roll_speed);
  SerialWriteValue(forces[MEM_ROLL]);
  SerialWriteValue(pitchAxis.GetConfiguration().iMin);
  SerialWriteValue(pitchAxis.GetConfiguration().iMax);
  SerialWriteValue(counterPitch.read());
  SerialWriteValue(pitch_speed);
  SerialWriteValue(forces[MEM_PITCH]);
}

void CMD_READ_ALL_PARAMS() {
  SerialWriteValue(adjForceMax[MEM_PITCH]);
  SerialWriteValue(adjPwmMin[MEM_PITCH]);
  SerialWriteValue(adjPwmMax[MEM_PITCH]);
  SerialWriteValue(adjForceMax[MEM_ROLL]);
  SerialWriteValue(adjPwmMin[MEM_ROLL]);
  SerialWriteValue(adjPwmMax[MEM_ROLL]);
  for (int i = 0; i < MEM_AXES; i++) {
    SerialWriteValue(gains[i].totalGain);
    SerialWriteValue(gains[i].constantGain);
    SerialWriteValue(gains[i].rampGain);
    SerialWriteValue(gains[i].squareGain);
    SerialWriteValue(gains[i].sineGain);
    SerialWriteValue(gains[i].triangleGain);
    SerialWriteValue(gains[i].sawtoothdownGain);
    SerialWriteValue(gains[i].sawtoothupGain);
    SerialWriteValue(gains[i].springGain);
    SerialWriteValue(gains[i].damperGain);
    SerialWriteValue(gains[i].inertiaGain);
    SerialWriteValue(gains[i].frictionGain);
    SerialWriteValue(effects[i].frictionMaxPositionChange);
    SerialWriteValue(effects[i].inertiaMaxAcceleration);
    SerialWriteValue(effects[i].damperMaxVelocity);
  }
}

void SerialWriteStart(byte command) {
  bSerialIndex = 0;
  Serial.print('!');
  Serial.print(command);
  Serial.print('|');
}

void SerialWriteEnd()              { Serial.println(); }

void SerialWriteValue(int16_t value) {
  Serial.print(bSerialIndex);
  Serial.print(':');
  Serial.print(value);
  Serial.print(',');
  bSerialIndex++;
}
