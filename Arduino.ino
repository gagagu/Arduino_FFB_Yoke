// I would define the Arduino pins here at the top

#define ROLL_R_EN 8
#define PITCH_L_EN 16

void ArduinoSetup()
{
    // set up the Arduino pins or other requirements

    /*
    pinMode(PITCH_R_EN, OUTPUT);
    pinMode(PITCH_L_EN, OUTPUT);
    */
}

void ReadPots()
{
    // this example supposes A2 is X and A3 is Y axis

    /*
    pos[0] = map(analogRead(A2), 0, 1023, minX, maxX);
    pos[1] = map(analogRead(A3), 0, 1023, minY, maxY);
    pos_updated = true;
    */
}

void DriveMotors() {
    // X Axis Function
    // Replace this with code specific to the motor you have connected
    // forces[0] and forces[1] are the X,Y forces
    // in the example below 10 is the minimum analog value to send if your motor does
    //   not react with a number less than 10
    //   244 would be the maximum value you want to send

    /*
    if(forces[0] > 0) {
        digitalWrite(ROLL_L_EN,HIGH);
        digitalWrite(ROLL_R_EN,HIGH);
        analogWrite(ROLL_R_PWM,map(abs(forces[0]), 0, 10000, 10, 244));
    } else {
        digitalWrite(ROLL_L_EN,HIGH);
        digitalWrite(ROLL_R_EN,HIGH);
        analogWrite(ROLL_L_PWM,map(abs(forces[0]), 0, 10000, 10, 244));
    }
    */
}
