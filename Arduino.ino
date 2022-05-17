// I would define the Arduino pins here at the top

// Pitch
#define PITCH_EN 11
#define PITCH_R_PWM 12
#define PITCH_L_PWM 13

// Roll
#define ROLL_EN 8
#define ROLL_R_PWM 9
#define ROLL_L_PWM 10

//Potis
#define POTI_ROLL A2
#define POTI_PITCH A1
#define POTI_PEDAL A0

// Mux
#define MUX_S0 0
#define MUX_S1 1
#define MUX_S2 2
#define MUX_S3 3
#define MUX_SIGNAL A5

int muxControlPin[]= {MUX_S0, MUX_S1, MUX_S2, MUX_S3};
int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

void ArduinoSetup()
{
  // set up the Arduino pins
  pinMode(PITCH_EN, OUTPUT);
  pinMode(PITCH_R_PWM, OUTPUT);
  pinMode(PITCH_L_PWM, OUTPUT);

  pinMode(ROLL_EN, OUTPUT);
  pinMode(ROLL_R_PWM, OUTPUT);
  pinMode(ROLL_L_PWM, OUTPUT);

  pinMode(POTI_ROLL,INPUT);
  pinMode(POTI_PITCH,INPUT);

  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(MUX_SIGNAL,INPUT);
  
  // define pin default states
  digitalWrite(PITCH_EN, LOW);
  digitalWrite(PITCH_R_PWM, LOW);
  digitalWrite(PITCH_L_PWM, LOW);
  
  digitalWrite(ROLL_EN, LOW);
  digitalWrite(ROLL_R_PWM, LOW);
  digitalWrite(ROLL_L_PWM, LOW);
  
  digitalWrite(MUX_S0, LOW);
  digitalWrite(MUX_S1, LOW);
  digitalWrite(MUX_S2, LOW);
  digitalWrite(MUX_S3, LOW);
}

void ReadPots()
{
    // read positions
    pos[0] = map(analogRead(POTI_ROLL), 0, 1023, minY, maxY);
    pos[1] = 0; //map(analogRead(POTI_PITCH), 0, 1023, minX, maxX);
    pos_updated = true;
    
}

void DriveMotors() {
  // Pitch forces
 
  if(forces[1]>0){
    digitalWrite(PITCH_EN,HIGH);
    analogWrite(PITCH_R_PWM,map(abs(forces[1]), 0, 10000, 1, 244));
  }else{
    digitalWrite(PITCH_EN,HIGH);
    analogWrite(PITCH_L_PWM,map(abs(forces[1]), 0, 10000, 1, 244));
  }

  // Roll forces
  if(forces[0]==0)
  {
    digitalWrite(ROLL_EN,LOW);
  }
  else{
    if(forces[0]>50){
      digitalWrite(ROLL_EN,HIGH);
      analogWrite(ROLL_L_PWM,map(abs(forces[0]), 0, 10000, 1, 244));
    }
    if(forces[0]<(-50)){    
      digitalWrite(ROLL_EN,HIGH);
      analogWrite(ROLL_R_PWM,map(abs(forces[0]), 0, 10000, 1, 244));
    }
  }

}

void ReadMux(){
   // not finished yet
   int channel =0;
   
  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(muxControlPin[i], muxChannel[channel][i]);
  }
  int val = analogRead(MUX_SIGNAL);
  
}
