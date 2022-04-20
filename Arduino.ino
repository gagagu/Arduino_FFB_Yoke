// I would define the Arduino pins here at the top

// Pitch
#define PITCH_EN 11
#define PITCH_R_PWM 12
#define PITCH_L_PWM 13

// Roll
#define ROLL_EN 8
#define ROLL_R_PWM 9
#define ROLL_L_PWM 10

// Pedal
#define PEDAL_EN 5
#define PEDAL_R_PWM 6
#define PEDAL_L_PWM 7

//Potis
#define POTI_ROLL A2
#define POTI_PITCH A1
#define POTI_PEDAL A0

void ArduinoSetup()
{
  // set up the Arduino pins or other requirements
  pinMode(PITCH_EN, OUTPUT);
  pinMode(PITCH_R_PWM, OUTPUT);
  pinMode(PITCH_L_PWM, OUTPUT);

  pinMode(ROLL_EN, OUTPUT);
  pinMode(ROLL_R_PWM, OUTPUT);
  pinMode(ROLL_L_PWM, OUTPUT);

  pinMode(PEDAL_EN, OUTPUT);
  pinMode(PEDAL_R_PWM, OUTPUT);
  pinMode(PEDAL_L_PWM, OUTPUT);
      
  pinMode(POTI_ROLL,INPUT);
  pinMode(POTI_PITCH,INPUT);
  pinMode(POTI_PEDAL,INPUT);
}

void ReadPots()
{
    // read positions
    pos[0] = map(analogRead(POTI_PITCH), 0, 1023, minX, maxX);
    pos[1] = map(analogRead(POTI_ROLL), 0, 1023, minY, maxY);
    pos[2] = 0; //map(analogRead(POTI_PEDAL), 0, 1023, minY, maxY);
    pos_updated = true;
    
}

void DriveMotors() {
  // Pitch forces
  if(forces[0]>0){
    digitalWrite(PITCH_EN,HIGH);
    analogWrite(PITCH_R_PWM,map(abs(forces[0]), 0, 10000, 1, 244));
  }else{
    digitalWrite(PITCH_EN,HIGH);
    analogWrite(PITCH_L_PWM,map(abs(forces[0]), 0, 10000, 1, 244));
  }

  // Roll forces
  if(forces[1]>0){
    digitalWrite(ROLL_EN,HIGH);
    analogWrite(ROLL_R_PWM,map(abs(forces[1]), 0, 10000, 1, 244));
  }else{
    digitalWrite(ROLL_EN,HIGH);
    analogWrite(ROLL_L_PWM,map(abs(forces[1]), 0, 10000, 1, 244));
  }

//  // Pedal forces
//  if(forces[2]>0){
//    digitalWrite(PEDAL_EN,HIGH);
//    analogWrite(PEDAL_R_PWM,map(abs(forces[2]), 0, 10000, 1, 244));
//  }else{
//    digitalWrite(PEDAL_EN,HIGH);
//    analogWrite(PEDAL_L_PWM,map(abs(forces[2]), 0, 10000, 1, 244));
//  }
       
  digitalWrite(PITCH_EN,LOW);
  digitalWrite(ROLL_EN,LOW);
//  digitalWrite(PEDAL_EN,LOW);
}
