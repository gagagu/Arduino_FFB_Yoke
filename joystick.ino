

void setupJoystick() {
    Joystick.setXAxisRange(JOYSTICK_minX, JOYSTICK_maxX);
    Joystick.setYAxisRange(JOYSTICK_minY, JOYSTICK_maxY);
    Joystick.begin();
    Gains gains[FFB_AXIS_COUNT];
    gains[MEM_ROLL].frictionGain = friction_gain;
    gains[MEM_PITCH].frictionGain = friction_gain;
    Joystick.setGains(gains);
}

void updateEffects(bool recalculate){
    for (int i =0; i < MEM_AXES; i++) {
        effects[i].frictionMaxPositionChange = frictionMaxPositionChangeCfg;
        effects[i].inertiaMaxAcceleration = inertiaMaxAccelerationCfg;
        effects[i].damperMaxVelocity = damperMaxVelocityCfg;
    }

    //If you need to use the spring effect, set the following parameters.`Position` is the current position of the force feedback axis. 
    //For example, connect the encoder with the action axis,the current encoder value is `Positon` and the max encoder value is `MaxPosition`.
    effects[MEM_ROLL].springMaxPosition = JOYSTICK_maxX;
    effects[MEM_PITCH].springMaxPosition = JOYSTICK_maxY;
    effects[MEM_ROLL].springPosition = pos[MEM_ROLL];
    effects[MEM_PITCH].springPosition = pos[MEM_PITCH];

    unsigned long currentMillis;
    currentMillis = millis();
    int16_t diffTime = currentMillis - lastEffectsUpdate;

    if (diffTime > 0 && recalculate) {
        lastEffectsUpdate = currentMillis;
        int16_t positionChangeX = pos[0] - lastX;
        int16_t positionChangeY = pos[1] - lastY;
        int16_t velX = positionChangeX / diffTime;
        int16_t velY = positionChangeY / diffTime;
        int16_t accelX = ((velX - lastVelX) * 10) / diffTime;
        int16_t accelY = ((velY - lastVelY) * 10) / diffTime;

        //If you need to use the friction effect, set the following parameters.`PositionChange` 
        //is the position difference of the force feedback axis.
        effects[MEM_ROLL].frictionPositionChange = velX;
        effects[MEM_PITCH].frictionPositionChange = velY;
        //effects[MEM_ROLL].frictionPositionChange = positionChangeX;
        //effects[MEM_PITCH].frictionPositionChange = positionChangeY;

        //If you need to use the damper effect, set the following parameters.`Velocity` is the current velocity of the force feedback axis.
        effects[MEM_ROLL].inertiaAcceleration = accelX;
        effects[MEM_PITCH].inertiaAcceleration = accelY;

       //If you need to use the inertia effect, set the following parameters.`Acceleration` is the current acceleration of the force feedback axis.
        effects[MEM_ROLL].damperVelocity = velX;
        effects[MEM_PITCH].damperVelocity = velY;

        #ifdef DEBUG
          log(1,pos[MEM_ROLL]);
          log(2,pos[MEM_PITCH]);
          log(3,positionChangeX);
          log(4,positionChangeY);
          log(5,velX);
          log(6,velY);
          log(7,accelX);
          log(8,accelY);
        #endif

        lastX = pos[MEM_ROLL];
        lastY = pos[MEM_PITCH];
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

    Joystick.setEffectParams(effects);
    Joystick.getForce(forces);

    #ifdef DEBUG
    if (diffTime > 0 && recalculate) {
          log(9,forces[MEM_ROLL]);
          log(10,forces[MEM_PITCH]);
    }
    #endif
}
