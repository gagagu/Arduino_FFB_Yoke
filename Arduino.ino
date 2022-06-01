// Pitch
#define PITCH_EN 13
#define PITCH_R_PWM 3
#define PITCH_L_PWM 11

// Roll
#define ROLL_EN 8
#define ROLL_R_PWM 9
#define ROLL_L_PWM 10

//Potis
#define POTI_ROLL A0
#define POTI_PITCH A1
#define POTI_PEDAL A2

// Speed calculation
int roll_speed = 0;
int pitch_speed=0;

// multiplexer for buttons, not finished yet!
// Mux
#define MUX_S0 0
#define MUX_S1 1
#define MUX_S2 2
#define MUX_S3 3
#define MUX_SIGNAL A5

int muxControlPin[] = {MUX_S0, MUX_S1, MUX_S2, MUX_S3};
int muxChannel[16][4] = {
  {0, 0, 0, 0}, //channel 0
  {1, 0, 0, 0}, //channel 1
  {0, 1, 0, 0}, //channel 2
  {1, 1, 0, 0}, //channel 3
  {0, 0, 1, 0}, //channel 4
  {1, 0, 1, 0}, //channel 5
  {0, 1, 1, 0}, //channel 6
  {1, 1, 1, 0}, //channel 7
  {0, 0, 0, 1}, //channel 8
  {1, 0, 0, 1}, //channel 9
  {0, 1, 0, 1}, //channel 10
  {1, 1, 0, 1}, //channel 11
  {0, 0, 1, 1}, //channel 12
  {1, 0, 1, 1}, //channel 13
  {0, 1, 1, 1}, //channel 14
  {1, 1, 1, 1} //channel 15
};

// Some other definitions

// Max pwm byte for pwm speed
#define max_pitch_pwm_speed  150
#define max_roll_pwm_speed  150

// Max Force for Max PWM Speed
#define max_pitch_force 10000
#define max_roll_force 10000

// Dead Points for the middle
#define roll_dead_point_min -100
#define roll_dead_point_max 100
#define pitch_dead_point_min -50
#define pitch_dead_point_max 50

void ArduinoSetup()
{
  // set up the Arduino pins
  pinMode(PITCH_EN, OUTPUT);
  pinMode(PITCH_R_PWM, OUTPUT);
  pinMode(PITCH_L_PWM, OUTPUT);

  pinMode(ROLL_EN, OUTPUT);
  pinMode(ROLL_R_PWM, OUTPUT);
  pinMode(ROLL_L_PWM, OUTPUT);

  pinMode(POTI_ROLL, INPUT);
  pinMode(POTI_PITCH, INPUT);

  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(MUX_SIGNAL, INPUT);

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

  // not for all Arduinos!
  // This sets the PWM Speed to maximun for noise reduction
  
  // timer 1B: pin 9 & 10
  TCCR1B = _BV(CS00); // change the PWM frequencey to 31.25kHz   - pins 9 & 10

  // timer 0B : pin 3 & 11
  TCCR0B = _BV(CS00); // change the PWM frequencey to 31.25 kHz  - pin 3 & 11
}

// Read the axes poti values and save it
void ReadPots()
{
  // read poti positions
  pos[0] = map(analogRead(POTI_ROLL), 0, 1023, minY, maxY);
  pos[1] = map(analogRead(POTI_PITCH), 0, 1023, minX, maxX);
  pos_updated = true;
  
  #ifdef DEBUG
    Serial.print(" POTI_ROLL:");
    Serial.print(pos[0]);
    Serial.print(", POTI_PITCH:");
    Serial.print(pos[1]);
    Serial.print(", ");
  #endif     
}//ReadPots


// calculates the motor speeds and controls the motors
void DriveMotors() {
  
  // Pitch forces
  if (forces[1] <= pitch_dead_point_max && forces[1] >= pitch_dead_point_min) // between dead points no motor
  {
    digitalWrite(PITCH_EN, LOW); // disable motor
    pitch_speed = 0;             // speed to 0
  }
  else {
    pitch_speed = map(abs(forces[1]), 0, max_pitch_force, 1, max_pitch_pwm_speed); // calculate motor speed (pwm) by force between 1 and max pwm speed

    // which direction?
    if (forces[1] > pitch_dead_point_max) {
      digitalWrite(PITCH_EN, HIGH);          // enable motor
      analogWrite(PITCH_R_PWM, pitch_speed);  // speed up
    }
    if (forces[1] < pitch_dead_point_min) {
      digitalWrite(PITCH_EN, HIGH);          // enable motor
      analogWrite(PITCH_L_PWM, pitch_speed);  // speed up
    }
  }



  // Roll forces
  if (forces[0] <= roll_dead_point_max && forces[0] >= roll_dead_point_min) // between dead points no motor
  {
    digitalWrite(ROLL_EN, LOW); // disable motor
    roll_speed = 0;             // speed to 0
  }
  else {
    roll_speed = map(abs(forces[0]), 0, max_roll_force, 1, max_roll_pwm_speed); // calculate motor speed (pwm) by force between 1 and max pwm speed

    // which direction?
    if (forces[0] > roll_dead_point_max) {
      digitalWrite(ROLL_EN, HIGH);          // enable motor
      analogWrite(ROLL_L_PWM, roll_speed);  // speed up
    }
    if (forces[0] < roll_dead_point_min) {
      digitalWrite(ROLL_EN, HIGH);          // enable motor
      analogWrite(ROLL_R_PWM, roll_speed);  // speed up
    }
  }
  
  #ifdef DEBUG
    Serial.print(" roll_speed:");
    Serial.print(roll_speed);
    Serial.print(", pitch_speed:");
    Serial.print(pitch_speed);
    Serial.print(", ");
  #endif     
} //DriveMotors

// Reads the button states over multiplexer
// Not finished, needs to be done
void ReadMux() {
  // not finished yet
  int channel = 0;

  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(muxControlPin[i], muxChannel[channel][i]);
  }
  int val = analogRead(MUX_SIGNAL);

} //ReadMux
