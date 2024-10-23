

/******************************************
  setup joystick and initialisation
*******************************************/
void SetupJoystick() {
  if(IsEepromDataAvailable()==1)
  {
    ReadDataFromEeprom();
  }else{
    SetupDefaults();
  }

  SetGains();                           // set gains
  SetRangeJoystick();                   // set range
  Joystick.begin(false);                // start joystick emulation (no auto send updates);
}

// default values
void SetupDefaults(){
  gains[MEM_ROLL].totalGain = default_gain;
  gains[MEM_ROLL].constantGain = default_gain;
  gains[MEM_ROLL].rampGain = default_gain;
  gains[MEM_ROLL].squareGain = default_gain;
  gains[MEM_ROLL].sineGain = default_gain;
  gains[MEM_ROLL].triangleGain = default_gain;
  gains[MEM_ROLL].sawtoothdownGain = default_gain;
  gains[MEM_ROLL].sawtoothupGain = default_gain;
  gains[MEM_ROLL].springGain = default_gain;
  gains[MEM_ROLL].damperGain = default_gain;
  gains[MEM_ROLL].inertiaGain = default_gain;
  gains[MEM_ROLL].frictionGain = default_friction_gain;

  gains[MEM_PITCH].totalGain = default_gain;
  gains[MEM_PITCH].constantGain = default_gain;
  gains[MEM_PITCH].rampGain = default_gain;
  gains[MEM_PITCH].squareGain = default_gain;
  gains[MEM_PITCH].sineGain = default_gain;
  gains[MEM_PITCH].triangleGain = default_gain;
  gains[MEM_PITCH].sawtoothdownGain = default_gain;
  gains[MEM_PITCH].sawtoothupGain = default_gain;
  gains[MEM_PITCH].springGain = default_gain;
  gains[MEM_PITCH].damperGain = default_gain;
  gains[MEM_PITCH].inertiaGain = default_gain;
  gains[MEM_PITCH].frictionGain = default_friction_gain;

  effects[MEM_ROLL].frictionMaxPositionChange = default_frictionMaxPositionChange_ROLL;
  effects[MEM_ROLL].inertiaMaxAcceleration = default_inertiaMaxAcceleration_ROLL;
  effects[MEM_ROLL].damperMaxVelocity = default_damperMaxVelocity_ROLL;

  effects[MEM_PITCH].frictionMaxPositionChange = default_frictionMaxPositionChange_PITCH;
  effects[MEM_PITCH].inertiaMaxAcceleration = default_inertiaMaxAcceleration_PITCH;
  effects[MEM_PITCH].damperMaxVelocity = default_damperMaxVelocity_PITCH;

  adjForceMax[MEM_ROLL]=default_ROLL_FORCE_MAX;
  adjPwmMin[MEM_ROLL]=default_ROLL_PWM_MIN;
  adjPwmMax[MEM_ROLL]=default_ROLL_PWM_MAX;

  adjForceMax[MEM_PITCH]=default_PITCH_FORCE_MAX;
  adjPwmMin[MEM_PITCH]=default_PITCH_PWM_MIN;
  adjPwmMax[MEM_PITCH]=default_PITCH_PWM_MAX;
}

void SetRangeJoystick() {
  Joystick.setXAxisRange(ROLL_AxisConfiguration.iMin, ROLL_AxisConfiguration.iMax);
  Joystick.setYAxisRange(PITCH_AxisConfiguration.iMin, PITCH_AxisConfiguration.iMax);
}

void SetGains() {
  Joystick.setGains(gains);
}

void UpdateEffects(bool recalculate) {
  //If you need to use the spring effect, set the following parameters.`Position` is the current position of the force feedback axis.
  //For example, connect the encoder with the action axis,the current encoder value is `Positon` and the max encoder value is `MaxPosition`.
  effects[MEM_ROLL].springMaxPosition = ROLL_AxisConfiguration.iMax;
  effects[MEM_PITCH].springMaxPosition = PITCH_AxisConfiguration.iMax;

  effects[MEM_ROLL].springPosition = counterRoll.read();
  effects[MEM_PITCH].springPosition = counterPitch.read();

  unsigned long currentMillis = millis();
  int16_t diffTime = currentMillis - lastEffectsUpdate;

  if (diffTime > 0 && recalculate) {
    lastEffectsUpdate = currentMillis;
    int16_t positionChangeX = counterRoll.read() - lastX;
    int16_t positionChangeY = counterPitch.read() - lastY;
    int16_t velX = positionChangeX / diffTime;
    int16_t velY = positionChangeY / diffTime;
    int16_t accelX = ((velX - lastVelX) * 10) / diffTime;
    int16_t accelY = ((velY - lastVelY) * 10) / diffTime;

    //If you need to use the friction effect, set the following parameters.`PositionChange`
    //is the position difference of the force feedback axis.
    effects[MEM_ROLL].frictionPositionChange = velX;
    effects[MEM_PITCH].frictionPositionChange = velY;

    //If you need to use the damper effect, set the following parameters.`Velocity` is the current velocity of the force feedback axis.
    effects[MEM_ROLL].inertiaAcceleration = accelX;
    effects[MEM_PITCH].inertiaAcceleration = accelY;

    //If you need to use the inertia effect, set the following parameters.`Acceleration` is the current acceleration of the force feedback axis.
    effects[MEM_ROLL].damperVelocity = velX;
    effects[MEM_PITCH].damperVelocity = velY;

    lastX = counterRoll.read();
    lastY = counterPitch.read();
    lastVelX = velX;
    lastVelY = velY;
    lastAccelX = accelX;
    lastAccelY = accelY;
  } else {
    effects[MEM_ROLL].frictionPositionChange = lastVelX;
    effects[MEM_PITCH].frictionPositionChange = lastVelY;
    effects[MEM_ROLL].inertiaAcceleration = lastAccelX;
    effects[MEM_PITCH].inertiaAcceleration = lastAccelY;
    effects[MEM_ROLL].damperVelocity = lastVelX;
    effects[MEM_PITCH].damperVelocity = lastVelY;
  }

  if(ROLL_AxisConfiguration.blAxisInverted)
  {
    Joystick.setXAxis(counterRoll.read() * (-1));

  }else{
    Joystick.setXAxis(counterRoll.read());
  }

  if(PITCH_AxisConfiguration.blAxisInverted)
  {
    Joystick.setYAxis(counterPitch.read() * (-1));
  }else{
    Joystick.setYAxis(counterPitch.read());
  }

  Joystick.setEffectParams(effects);
  Joystick.getForce(forces);
}
