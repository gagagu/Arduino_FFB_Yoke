void setupJoystick() {
    Joystick.setXAxisRange(minX, maxX);
    Joystick.setYAxisRange(minY, maxY);
    Joystick.begin();
    Gains gains[FFB_AXIS_COUNT];
    gains[0].frictionGain = friction_gain;
    gains[1].frictionGain = friction_gain;
    gains[2].frictionGain = friction_gain;
    Joystick.setGains(gains);
}

void updateJoystickPos() {
    Joystick.setXAxis(pos[0]);
    Joystick.setYAxis(pos[1]);
    Joystick.setZAxis(pos[2]);
}

void updateEffects(bool recalculate){
    for (int i =0; i < 3; i++) {
        effects[i].frictionMaxPositionChange = frictionMaxPositionChangeCfg;
        effects[i].inertiaMaxAcceleration = inertiaMaxAccelerationCfg;
        effects[i].damperMaxVelocity = damperMaxVelocityCfg;
    }

    effects[0].springMaxPosition = maxX;
    effects[1].springMaxPosition = maxY;
    effects[2].springMaxPosition = maxZ;
    
    effects[0].springPosition = pos[0];
    effects[1].springPosition = pos[1];
    effects[2].springPosition = pos[2];

    unsigned long currentMillis;
    currentMillis = millis();
    int16_t diffTime = currentMillis - lastEffectsUpdate;

    if (diffTime > 0 && recalculate) {
        lastEffectsUpdate = currentMillis;
        int16_t positionChangeX = pos[0] - lastX;
        int16_t positionChangeY = pos[1] - lastY;
        int16_t positionChangeZ = pos[2] - lastZ;
        int16_t velX = positionChangeX / diffTime;
        int16_t velY = positionChangeY / diffTime;
        int16_t velZ = positionChangeZ / diffTime;
        int16_t accelX = ((velX - lastVelX) * 10) / diffTime;
        int16_t accelY = ((velY - lastVelY) * 10) / diffTime;
        int16_t accelZ = ((velZ - lastVelZ) * 10) / diffTime;
    
        effects[0].frictionPositionChange = velX;
        effects[1].frictionPositionChange = velY;
        effects[2].frictionPositionChange = velZ;
        
        effects[0].inertiaAcceleration = accelX;
        effects[1].inertiaAcceleration = accelY;
        effects[2].inertiaAcceleration = accelZ;
        
        effects[0].damperVelocity = velX;
        effects[1].damperVelocity = velY;
        effects[2].damperVelocity = velZ;

        #ifdef DEBUG
        write_order(LOG);
        Serial.print(currentMillis);
        Serial.print("X");
        Serial.print(pos[0]);
        Serial.print("Y");
        Serial.print(pos[1]);
        Serial.print("Z");
        Serial.print(pos[2]);        
        Serial.print("C");
        Serial.print(positionChangeX);
        Serial.print(",");
        Serial.print(positionChangeY);
        Serial.print(",");
        Serial.print(positionChangeZ);        
        Serial.print("V");
        Serial.print(velX);
        Serial.print(",");
        Serial.print(velY);
        Serial.print(",");
        Serial.print(velZ);        
        Serial.print("A");
        Serial.print(accelX);
        Serial.print(",");
        Serial.print(accelY);
        Serial.print(",");
        Serial.print(accelZ);        
        #endif

        lastX = pos[0];
        lastY = pos[1];
        lastZ = pos[2];
        
        lastVelX = velX;
        lastVelY = velY;
        lastVelZ = velZ;
        
        lastAccelX = accelX;
        lastAccelY = accelY;
        lastAccelZ = accelZ;
    } else {
        effects[0].frictionPositionChange = lastVelX;
        effects[1].frictionPositionChange = lastVelY;
        effects[2].frictionPositionChange = lastVelZ;
        
        effects[0].inertiaAcceleration = lastAccelX;
        effects[1].inertiaAcceleration = lastAccelY;
        effects[2].inertiaAcceleration = lastAccelZ;
        
        effects[0].damperVelocity = lastVelX;
        effects[1].damperVelocity = lastVelY;
        effects[2].damperVelocity = lastVelZ;
    }

    Joystick.setEffectParams(effects);
    Joystick.getForce(forces);

    #ifdef DEBUG
    if (diffTime > 0 && recalculate) {
        Serial.print("F");
        Serial.print(forces[0]);
        Serial.print(",");
        Serial.println(forces[1]);
        Serial.print(",");
        Serial.println(forces[2]);        
    }
    #endif
}
