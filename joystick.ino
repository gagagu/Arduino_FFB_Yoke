/* 
 Created by A.Eckers aka Gagagu
 http://www.gagagu.de
 https://github.com/gagagu/Arduino_FFB_Yoke
 https://www.youtube.com/@gagagu01
*/

/******************************************
  Set up joystick and initialise defaults
*******************************************/
void SetupJoystick() {
  if (IsEepromDataAvailable() == 1) {
    ReadDataFromEeprom();
  } else {
    SetupDefaults();
  }

  SetGains();
  SetRangeJoystick();
  Joystick.begin(false);  // start joystick emulation (no auto-send)
}

/******************************************
  Helper: apply identical default gains to one axis slot
*******************************************/
static void ApplyDefaultGains(Gains &g) {
  g.totalGain        = DEFAULT_GAIN;
  g.constantGain     = DEFAULT_GAIN;
  g.rampGain         = DEFAULT_GAIN;
  g.squareGain       = DEFAULT_GAIN;
  g.sineGain         = DEFAULT_GAIN;
  g.triangleGain     = DEFAULT_GAIN;
  g.sawtoothdownGain = DEFAULT_GAIN;
  g.sawtoothupGain   = DEFAULT_GAIN;
  g.springGain       = DEFAULT_GAIN;
  g.damperGain       = DEFAULT_GAIN;
  g.inertiaGain      = DEFAULT_GAIN;
  g.frictionGain     = DEFAULT_FRICTION_GAIN;
}

/******************************************
  Fill all parameters with compiled-in defaults.
  Called when no EEPROM data is found.
*******************************************/
void SetupDefaults() {
  // Gains – apply the same defaults to both axes via helper
  ApplyDefaultGains(gains[MEM_ROLL]);
  ApplyDefaultGains(gains[MEM_PITCH]);

  // Effect parameters – Roll
  effects[MEM_ROLL].frictionMaxPositionChange = DEFAULT_FRICTION_MAX_POS_CHANGE_ROLL;
  effects[MEM_ROLL].inertiaMaxAcceleration    = DEFAULT_INERTIA_MAX_ACCEL_ROLL;
  effects[MEM_ROLL].damperMaxVelocity         = DEFAULT_DAMPER_MAX_VELOCITY_ROLL;

  // Effect parameters – Pitch
  effects[MEM_PITCH].frictionMaxPositionChange = DEFAULT_FRICTION_MAX_POS_CHANGE_PITCH;
  effects[MEM_PITCH].inertiaMaxAcceleration    = DEFAULT_INERTIA_MAX_ACCEL_PITCH;
  effects[MEM_PITCH].damperMaxVelocity         = DEFAULT_DAMPER_MAX_VELOCITY_PITCH;

  // Motor / force limits – Roll
  adjForceMax[MEM_ROLL] = DEFAULT_ROLL_FORCE_MAX;
  adjPwmMin[MEM_ROLL]   = DEFAULT_ROLL_PWM_MIN;
  adjPwmMax[MEM_ROLL]   = DEFAULT_ROLL_PWM_MAX;

  // Motor / force limits – Pitch
  adjForceMax[MEM_PITCH] = DEFAULT_PITCH_FORCE_MAX;
  adjPwmMin[MEM_PITCH]   = DEFAULT_PITCH_PWM_MIN;
  adjPwmMax[MEM_PITCH]   = DEFAULT_PITCH_PWM_MAX;
}

void SetRangeJoystick() {
  Joystick.setXAxisRange(rollAxis.GetConfiguration().iMin,  rollAxis.GetConfiguration().iMax);
  Joystick.setYAxisRange(pitchAxis.GetConfiguration().iMin, pitchAxis.GetConfiguration().iMax);
}

void SetGains() {
  Joystick.setGains(gains);
}

/******************************************
  Update FFB effect parameters each loop tick.
  recalculate=true  → recompute velocity/acceleration from encoder deltas
  recalculate=false → reuse last computed values for smoother output
*******************************************/
void UpdateEffects(bool recalculate) {
  // Spring: current position and range
  effects[MEM_ROLL].springMaxPosition  = rollAxis.GetConfiguration().iMax;
  effects[MEM_PITCH].springMaxPosition = pitchAxis.GetConfiguration().iMax;
  effects[MEM_ROLL].springPosition     = counterRollValue;
  effects[MEM_PITCH].springPosition    = counterPitchValue;

  // BUG FIX: was int16_t – overflows after ~32 s and gives wrong velocities.
  //          Using unsigned long and casting the difference preserves the
  //          intended behaviour even across millis() rollovers.
  unsigned long currentMs = millis();
  unsigned long diffTime  = currentMs - lastEffectsUpdate;

  if (diffTime > 0 && recalculate) {
    lastEffectsUpdate = currentMs;

    int16_t posChangeX = counterRollValue  - lastX;
    int16_t posChangeY = counterPitchValue - lastY;

    // Velocity in encoder-counts / ms  (×10 to keep integer precision)
    int16_t velX   = (int16_t)((int32_t)posChangeX * 10 / (int32_t)diffTime);
    int16_t velY   = (int16_t)((int32_t)posChangeY * 10 / (int32_t)diffTime);

    // Acceleration in velocity-units / ms  (×10 again)
    int16_t accelX = (int16_t)((int32_t)(velX - lastVelX) * 10 / (int32_t)diffTime);
    int16_t accelY = (int16_t)((int32_t)(velY - lastVelY) * 10 / (int32_t)diffTime);

    // Friction uses velocity (position-change per time)
    effects[MEM_ROLL].frictionPositionChange  = velX;
    effects[MEM_PITCH].frictionPositionChange = velY;

    // Damper uses velocity
    effects[MEM_ROLL].damperVelocity  = velX;
    effects[MEM_PITCH].damperVelocity = velY;

    // Inertia uses acceleration
    effects[MEM_ROLL].inertiaAcceleration  = accelX;
    effects[MEM_PITCH].inertiaAcceleration = accelY;

    // Save state for next iteration
    lastX      = counterRollValue;
    lastY      = counterPitchValue;
    lastVelX   = velX;
    lastVelY   = velY;
    lastAccelX = accelX;
    lastAccelY = accelY;

  } else {
    // Reuse cached values between recalculation intervals
    effects[MEM_ROLL].frictionPositionChange  = lastVelX;
    effects[MEM_PITCH].frictionPositionChange = lastVelY;
    effects[MEM_ROLL].damperVelocity          = lastVelX;
    effects[MEM_PITCH].damperVelocity         = lastVelY;
    effects[MEM_ROLL].inertiaAcceleration     = lastAccelX;
    effects[MEM_PITCH].inertiaAcceleration    = lastAccelY;
  }

  // Roll axis is inverted in hardware – negate the reported axis value.
  // NOTE: springPosition is NOT negated – it must stay in the same
  // coordinate space as the encoder so that the built-in spring and
  // ConditionForce calculations push in the correct physical direction.
  // The cpOffset from the sim (used by AP following) is in the same
  // physical coordinate space, so no negation is needed here.
  effects[MEM_ROLL].springPosition = counterRollValue;

  Joystick.setXAxis(-counterRollValue);
  Joystick.setYAxis(counterPitchValue);

  Joystick.setEffectParams(effects);
  Joystick.getForce(forces);
}
