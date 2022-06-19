

void setupJoystick() {
    Joystick.setXAxisRange(JOYSTICK_minX, JOYSTICK_maxX);
    Joystick.setYAxisRange(JOYSTICK_minY, JOYSTICK_maxY);
    Joystick.begin();
    Gains gains[FFB_AXIS_COUNT];
    gains[0].frictionGain = friction_gain;
    gains[1].frictionGain = friction_gain;
    Joystick.setGains(gains);
}

void updateEffects(bool recalculate){
    for (int i =0; i < 2; i++) {
        effects[i].frictionMaxPositionChange = frictionMaxPositionChangeCfg;
        effects[i].inertiaMaxAcceleration = inertiaMaxAccelerationCfg;
        effects[i].damperMaxVelocity = damperMaxVelocityCfg;
    }

    effects[0].springMaxPosition = JOYSTICK_maxX;
    effects[1].springMaxPosition = JOYSTICK_maxY;
    effects[0].springPosition = pos[0];
    effects[1].springPosition = pos[1];

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
    
        effects[0].frictionPositionChange = velX;
        effects[1].frictionPositionChange = velY;
        effects[0].inertiaAcceleration = accelX;
        effects[1].inertiaAcceleration = accelY;
        effects[0].damperVelocity = velX;
        effects[1].damperVelocity = velY;

        #ifdef DEBUG
        //write_order(LOG);
        Serial.print(currentMillis);
        Serial.print("\tX:");
        Serial.print(pos[0]);
        Serial.print("\tY:");
        Serial.print(pos[1]);
        Serial.print("\tCx:");
        Serial.print(positionChangeX);
        Serial.print("\tCy");
        Serial.print(positionChangeY);
        Serial.print("\tVx:");
        Serial.print(velX);
        Serial.print("\tVy:");
        Serial.print(velY);
        Serial.print("\tAx:");
        Serial.print(accelX);
        Serial.print("\tAy:");
        Serial.print(accelY);
        #endif

        lastX = pos[0];
        lastY = pos[1];
        lastVelX = velX;
        lastVelY = velY;
        lastAccelX = accelX;
        lastAccelY = accelY;
    } else {
        effects[0].frictionPositionChange = lastVelX;
        effects[1].frictionPositionChange = lastVelY;
        effects[0].inertiaAcceleration = lastAccelX;
        effects[1].inertiaAcceleration = lastAccelY;
        effects[0].damperVelocity = lastVelX;
        effects[1].damperVelocity = lastVelY;
    }

    Joystick.setEffectParams(effects);
    Joystick.getForce(forces);

    #ifdef DEBUG
    if (diffTime > 0 && recalculate) {
        Serial.print("\tF0:");
        Serial.print(forces[0]);
        Serial.print("\tF1:");
        Serial.print(forces[1]);
    }
    #endif
}
