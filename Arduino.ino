// Max pwm byte for pwm speed
#define min_pitch_pwm_speed  70     // defines the minimum force given to the pitch, beyond you will not feel it and the deathpoitn will be too high
#define max_pitch_pwm_speed  120    // defines the maximum force given to the pitch to reduce the power to the movement
#define min_roll_pwm_speed  40      // defines the minimum force given to the roll, beyond you will not feel it and the deathpoitn will be too high
#define max_roll_pwm_speed  90      // defines the maximum force given to the roll to reduce the power to the movement

// Max Force for Max PWM Speed
#define max_pitch_force 10000        // maximum pitch force excepted from the system (like XPForce), beyond will set to the max value
#define max_roll_force 10000         // maximum roll force excepted from the system (like XPForce), beyond will set to the max value

// Calibration Button and LED
#define CALIBRATION_LED_PIN 0
#define CALIBRATION_BUTTON_PIN A3

// Test Stuff
// #define TEST_POTI_PIN A2

// Pitch Motordriver pins
#define PITCH_EN 11
#define PITCH_R_PWM 6
#define PITCH_L_PWM 13
#define PITCH_EndSwitch 1

// Roll Motordriver pins
#define ROLL_EN 8
#define ROLL_R_PWM 9
#define ROLL_L_PWM 10
#define ROLL_EndSwitch 12

//Potis
#define POTI_ROLL A0
#define POTI_PITCH A1

// multiplexer for buttons
#define MUX_S0 4
#define MUX_S1 5
#define MUX_S2 3
#define MUX_S3 7
#define MUX_EN 2
#define MUX_SIGNAL A5

// variables for Speed calculation
int roll_speed = 0;
int pitch_speed = 0;

// maximum and minimum poti values for roll
int poti_roll_min = JOYSTICK_minY;
int poti_roll_max = JOYSTICK_maxY;
bool last_roll_endswitch=false;

// maximum and minimum poti values for pitch
int poti_pitch_min = JOYSTICK_minX;
int poti_pitch_max = JOYSTICK_maxY;
bool last_pitch_endswitch=false;

/***************
// Pin setup
****************/
void ArduinoSetup()
{
  // Power and Modus LED
  pinMode(CALIBRATION_LED_PIN, OUTPUT);
  digitalWrite(CALIBRATION_LED_PIN, HIGH);

  
  // Pitch Pins
  pinMode(POTI_PITCH, INPUT);
  pinMode(PITCH_EN, OUTPUT);
  pinMode(PITCH_R_PWM, OUTPUT);
  pinMode(PITCH_L_PWM, OUTPUT);
  pinMode(PITCH_EndSwitch, INPUT_PULLUP);
    
  // Roll Pins
  pinMode(POTI_ROLL, INPUT);
  pinMode(ROLL_EN, OUTPUT);
  pinMode(ROLL_R_PWM, OUTPUT);
  pinMode(ROLL_L_PWM, OUTPUT);
  pinMode(ROLL_EndSwitch, INPUT_PULLUP);
    
  // Buttons Pins (Multiplexer)
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(MUX_EN, OUTPUT);
  pinMode(MUX_SIGNAL, INPUT_PULLUP);
  
  // define pin default states
  // Pitch
  digitalWrite(PITCH_EN, LOW);
  digitalWrite(PITCH_R_PWM, LOW);
  digitalWrite(PITCH_L_PWM, LOW);
  //Roll
  digitalWrite(ROLL_EN, LOW);
  digitalWrite(ROLL_R_PWM, LOW);
  digitalWrite(ROLL_L_PWM, LOW);
  // Multiplexer
  digitalWrite(MUX_S0, LOW);
  digitalWrite(MUX_S1, LOW);
  digitalWrite(MUX_S2, LOW);
  digitalWrite(MUX_S3, LOW);
  digitalWrite(MUX_EN, HIGH);

  // not for all Arduinos!
  // This sets the PWM Speed to maximun for noise reduction

  // timer 1B: pin 9 & 10
  TCCR1B = _BV(CS10); // change the PWM frequencey to 31.25kHz   - pins 9 & 10

  // timer 0B : pin 13 & 6
  TCCR4B = _BV(CS40); // change the PWM frequencey to 31.25 kHz  - pin 6 & 13
}

/***************
//  Turn calibration LED on
****************/
void LedOn()
{
  digitalWrite(CALIBRATION_LED_PIN, HIGH);
}

/***************
//  Turn calibration LED off
****************/
void LedOff()
{
   digitalWrite(CALIBRATION_LED_PIN, LOW);
}

/***************
//  switch calibration LED on when off and off when on
****************/
void LedSwitch(){
   digitalWrite(CALIBRATION_LED_PIN, !digitalRead(CALIBRATION_LED_PIN));
}

/****************************************
// Read the axes poti values and save it
****************************************/
void UpdateJoystickPos()
{
  // check if roll poti values are initialized for safety reason
  if(poti_roll_min==0 && poti_roll_max==0)
  {
    pos[0]=0; // not initialized than 0
    //calibration_init = true; // start calibration mode
  } else{
    int pot_roll=analogRead(POTI_ROLL); // read poti value
  
    if(pot_roll < poti_roll_min) // check min
      pot_roll=poti_roll_min;
     
    if(pot_roll > poti_roll_max) // check max
      pot_roll=poti_roll_max;
  
    pos[0] = map(pot_roll, poti_roll_min, poti_roll_max, JOYSTICK_minY, JOYSTICK_maxY); // convert values to -32768 till 32767 Joystick value
  }

  // check if pitch poti values are initialized for safety reason
  if(poti_pitch_min==0 && poti_pitch_max==0)
  {
    pos[1]=0;  // not initialized than 0
    //calibration_init = true; // start calibration mode    
  } else{
    int pot_pitch=analogRead(POTI_PITCH); //read poti value
  
    if(pot_pitch < poti_pitch_min) // check min
      pot_pitch=poti_pitch_min;
     
    if(pot_pitch > poti_pitch_max) // check max
      pot_pitch=poti_pitch_max;    
    
    pos[1] = map(pot_pitch, poti_pitch_min, poti_pitch_max, JOYSTICK_minX, JOYSTICK_maxX); // convert values to -32768 till 32767 Joystick value
  }
  
 // set Joystick axis
  Joystick.setXAxis(pos[0]);
  Joystick.setYAxis(pos[1]);
  
  pos_updated = true;
  
  #ifdef DEBUG
    Serial.print("\tPOTI_ROLL:");
    Serial.print(analogRead(POTI_ROLL));
    Serial.print("\tPOTI_Pitch:");
    Serial.print(analogRead(POTI_PITCH));
  #endif 
}//ReadPots


/**************************
// Enables the motordrivers
****************************/
void EnableMotors(){
  digitalWrite(PITCH_EN, HIGH);    
  digitalWrite(ROLL_EN, HIGH); 
}

/***************************
// Disables the motordrivers
****************************/
void DisableMotors(){
  digitalWrite(PITCH_EN, HIGH);    
  digitalWrite(ROLL_EN, HIGH); 
}

/******************************************************
// calculates the motor speeds and controls the motors
******************************************************/
void DriveMotors() {
  
 
  // read Endswitch
  if(abs(forces[1])<=10 || digitalRead(PITCH_EndSwitch)==0)
  {
    analogWrite(PITCH_L_PWM, 0);  // stop left
    analogWrite(PITCH_R_PWM, 0);  // stop right
    pitch_speed = 0;              // speed to 0
  }
  else {
     int32_t pForce=constrain(abs(forces[1]),0, max_pitch_force);                             // cut force to maximum value
     pitch_speed = map(pForce, 0, max_pitch_force, min_pitch_pwm_speed, max_pitch_pwm_speed); // calculate motor speed (pwm) by force between min pwm and max pwm speed
  
    // which direction?
    if (forces[1] > 0) {
      analogWrite(PITCH_R_PWM, 0);            // stop right
      analogWrite(PITCH_L_PWM, pitch_speed);  // speed up left
    }
    else{
      analogWrite(PITCH_L_PWM, 0);            // stop left
      analogWrite(PITCH_R_PWM, pitch_speed);  // speed up right
    }
  }
  
  // Roll forces
  if (abs(forces[0]) <= 10 || digitalRead(ROLL_EndSwitch)==0) // between dead points no motor
  {
    analogWrite(ROLL_L_PWM, 0);  // stop left
    analogWrite(ROLL_R_PWM, 0);  // stop right
    roll_speed = 0;              // speed to 0
  }
  else {
    int32_t rForce=constrain(abs(forces[0]),0, max_roll_force);                          // cut force to maximum value
    roll_speed = map(rForce, 0, max_roll_force, min_roll_pwm_speed, max_roll_pwm_speed); // calculate motor speed (pwm) by force between min pwm and max pwm speed
    
    // which direction?
    if (forces[0] > 0) {
      analogWrite(ROLL_R_PWM, 0);           // stop right
      analogWrite(ROLL_L_PWM, roll_speed);  // speed up left
    }
    else{
      analogWrite(ROLL_L_PWM, 0);           // stop left
      analogWrite(ROLL_R_PWM, roll_speed);  // speed up right
    }
  }

  #ifdef DEBUG
    Serial.print("\troll_sp:");
    Serial.print(roll_speed);                                                                                                                                 
    Serial.print("\tpitch_sp:");
    Serial.print(pitch_speed);
  #endif
} //DriveMotors


/*************************************
// Reads a Values from the multiplexer
*************************************/
bool ReadMux(bool s0, bool s1, bool s2, bool s3)
{
  // set mux channel
  digitalWrite(MUX_S0, s0);
  digitalWrite(MUX_S1, s1);
  digitalWrite(MUX_S2, s2);
  digitalWrite(MUX_S3, s3);
  // enable mux (pleparation for additional muxers for e.g. force adjustments or so)   
  digitalWrite(MUX_EN, LOW);
  // read value
  bool val = digitalRead(MUX_SIGNAL);
  // disblae mux
  digitalWrite(MUX_EN, HIGH);
   
  return val;
} //readMux

/******************************************
// Reads the button states over multiplexer
*******************************************/
void UpdateJoystickButtons() {
    // read hat states from multiplexer channel 0-3
    bool hat_right = ReadMux(0,0,0,0);
    bool hat_down = ReadMux(1,0,0,0);
    bool hat_up = ReadMux(0,1,0,0);
    bool hat_left = ReadMux(1,1,0,0);
    
    #ifdef BUTTONDEBUG
        Serial.print("\thr:");
        Serial.print(hat_right);
        Serial.print("\thd:");
        Serial.print(hat_down);
        Serial.print("\thu:");
        Serial.print(hat_up);
        Serial.print("\thl:");
        Serial.print(hat_left);
    #endif

    //decode HatSwitch Positions
    if(hat_up == false &&  hat_right == false && hat_down == false && hat_left == false)
        Joystick.setHatSwitch(0, -1); // no direction

    if(hat_up == true &&  hat_right == false && hat_down == false && hat_left == false)
       Joystick.setHatSwitch(0, 0);     // up

    if(hat_up == true &&  hat_right == true && hat_down == false && hat_left == false)
       Joystick.setHatSwitch(0, 45);    // up right

    if(hat_up == false &&  hat_right == true && hat_down == false && hat_left == false)
      Joystick.setHatSwitch(0, 90);    // right
 
    if(hat_up == false &&  hat_right == true && hat_down == true && hat_left == false)
      Joystick.setHatSwitch(0, 135);   // down right
 
    if(hat_up == false &&  hat_right == false && hat_down == true && hat_left == false)
      Joystick.setHatSwitch(0, 180);   // down
 
    if(hat_up == false &&  hat_right == false && hat_down == true && hat_left == true)
      Joystick.setHatSwitch(0, 225);   // down left
      
    if(hat_up == false &&  hat_right == false && hat_down == false && hat_left == true)
       Joystick.setHatSwitch(0, 270);   // left

    if(hat_up == true &&  hat_right == false && hat_down == false && hat_left == true)
      Joystick.setHatSwitch(0, 315);  // up left 

    // read button states from multiplexer
    for (int channel = 4; channel < 16; channel++)
    {
      bool val = ReadMux(bitRead(channel, 0), bitRead(channel, 1), bitRead(channel, 2), bitRead(channel, 3));
      #ifdef BUTTONDEBUG
        Serial.print("\tb:");
        Serial.print(channel);
        Serial.print(",");
        Serial.print(val);
      #endif
      Joystick.setButton(channel -4, val);
   }

} //updateJoystickButtons
