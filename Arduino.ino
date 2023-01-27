// Pitch Motordriver pins
#define PITCH_EN 11
#define PITCH_R_PWM 6
#define PITCH_L_PWM 13

// Roll Motordriver pins
#define ROLL_EN 8
#define ROLL_R_PWM 9
#define ROLL_L_PWM 10

// multiplexer for buttons
#define MUX_S0 A0
#define MUX_S1 A1
#define MUX_S2 A2
#define MUX_S3 A3

// Multiplexer Yoke Buttons
#define MUX_EN_YOKE 5
#define MUX_SIGNAL_YOKE A5

// Multiplexer Adjustemts for Calib Button, Force Potis, End Switches
#define MUX_EN_INPUT 4
#define MUX_SIGNAL_INPUT A4

// Adjustments array positions
#define ADJ_ENDSWITCH_PITCH_DOWN 16
#define ADJ_ENDSWITCH_PITCH_UP 17
#define ADJ_ENDSWITCH_ROLL_LEFT 18
#define ADJ_ENDSWITCH_ROLL_RIGHT 19

#define ADJ_BUTTON_CALIBRATION 20

#define ADJ_POTI_PITCH_FORCE_MAX 0
#define ADJ_POTI_PITCH_PWM_MAX 1
#define ADJ_POTI_PITCH_PWM_MIN 2

#define ADJ_POTI_ROLL_FORCE_MAX 3
#define ADJ_POTI_ROLL_PWM_MAX 4
#define ADJ_POTI_ROLL_PWM_MIN 5

// variables for Speed calculation
int roll_speed = 0;
int pitch_speed = 0;

// maximum and minimum poti values for roll
int poti_roll_min = 0;                                //Potentiometer min for Init;
int poti_roll_max = 1023;                             //Potentiometer max for Init;

// maximum and minimum poti values for pitch
int poti_pitch_min = 0;                               //Potentiometer min for Init;
int poti_pitch_max = 1023;                            //Potentiometer max for Init;

// potis
int adjustments_input[6] = {0, 0, 0, 0, 0, 0};

// flags for calibration mode
bool calibration_mode_roll_min = false;
bool calibration_mode_roll_max = false;
bool calibration_mode_pitch_min = false;
bool calibration_mode_pitch_max = false;

// buffer for buttons
bool buttonPinStates[] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                          false, false, false, false, false, false, false, false, false, false
                         };
/***************
  Pin setup
****************/
void ArduinoSetup()
{

  // Pitch Pins
  pinMode(PITCH_EN, OUTPUT);
  pinMode(PITCH_R_PWM, OUTPUT);
  pinMode(PITCH_L_PWM, OUTPUT);

  // Roll Pins
  pinMode(ROLL_EN, OUTPUT);
  pinMode(ROLL_R_PWM, OUTPUT);
  pinMode(ROLL_L_PWM, OUTPUT);

  // Buttons Pins (Multiplexer)
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);

  pinMode(MUX_EN_YOKE, OUTPUT);
  pinMode(MUX_SIGNAL_YOKE, INPUT);

  pinMode(MUX_EN_INPUT, OUTPUT);
  pinMode(MUX_SIGNAL_INPUT, INPUT);

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
  digitalWrite(MUX_EN_YOKE, HIGH);
  digitalWrite(MUX_EN_INPUT, HIGH);

  // not for all Arduinos!
  // This sets the PWM Speed to maximun for noise reduction

  // Timer1: pins 9 & 10
  TCCR1B = _BV(CS10); // change the PWM frequencey to 31.25kHz - pins 9 & 10

  // Timer4: pin 13 & 6
  TCCR4B = _BV(CS40); // change the PWM frequencey to 31.25kHz - pin 13 & 6
}

/**************************
  Enables the motordrivers
****************************/
void EnableMotors() {
  digitalWrite(PITCH_EN, HIGH);
  digitalWrite(ROLL_EN, HIGH);
}

/***************************
  Disables the motordrivers
****************************/
void DisableMotors() {
  digitalWrite(PITCH_EN, LOW);
  digitalWrite(ROLL_EN, LOW);
  analogWrite(ROLL_L_PWM, 0);  // stop left
  analogWrite(ROLL_R_PWM, 0);  // stop right
  roll_speed = 0;              // speed to 0
  analogWrite(PITCH_L_PWM, 0);  // stop left
  analogWrite(PITCH_R_PWM, 0);  // stop right
  pitch_speed = 0;              // speed to 0
}

/******************************************************
  calculates the motor speeds and controls the motors
******************************************************/
void DriveMotors() {

  // if pitch is on end switch then stop the motor
  if (buttonPinStates[ADJ_ENDSWITCH_PITCH_DOWN] == 0 || buttonPinStates[ADJ_ENDSWITCH_PITCH_UP] == 0)
  {
    analogWrite(PITCH_L_PWM, 0);  // stop left
    analogWrite(PITCH_R_PWM, 0);  // stop right
    pitch_speed = 0;              // speed to 0
  }
  else {
    int32_t pForce = constrain(abs(forces[MEM_PITCH]), 0, adjustments_input[ADJ_POTI_PITCH_FORCE_MAX]);     // cut force to maximum value
    pitch_speed = map(pForce, 0,
                      adjustments_input[ADJ_POTI_PITCH_FORCE_MAX],
                      adjustments_input[ADJ_POTI_PITCH_PWM_MIN],
                      adjustments_input[ADJ_POTI_PITCH_PWM_MAX]);                                           // calculate motor speed (pwm) by force between min pwm and max pwm speed

    // which direction?
    if (forces[MEM_PITCH] > 0) {
      analogWrite(PITCH_R_PWM, 0);            // stop right
      analogWrite(PITCH_L_PWM, pitch_speed);  // speed up left
    }
    else {
      analogWrite(PITCH_L_PWM, 0);            // stop left
      analogWrite(PITCH_R_PWM, pitch_speed);  // speed up right
    }
  }

  //  if roll is on end switch then stop the motor
  if (buttonPinStates[ADJ_ENDSWITCH_ROLL_LEFT] == 0 || buttonPinStates[ADJ_ENDSWITCH_ROLL_RIGHT] == 0) // ir sensor
  //if (buttonPinStates[ADJ_ENDSWITCH_ROLL_LEFT] > 0 || buttonPinStates[ADJ_ENDSWITCH_ROLL_RIGHT] > 0) // microswitch
  {
    analogWrite(ROLL_L_PWM, 0);  // stop left
    analogWrite(ROLL_R_PWM, 0);  // stop right
    roll_speed = 0;              // speed to 0
  }
  else {
    int32_t rForce = constrain(abs(forces[MEM_ROLL]), 0, adjustments_input[ADJ_POTI_ROLL_FORCE_MAX]);     // cut force to maximum value
    roll_speed = map(rForce, 0,
                     adjustments_input[ADJ_POTI_ROLL_FORCE_MAX],
                     adjustments_input[ADJ_POTI_ROLL_PWM_MIN],
                     adjustments_input[ADJ_POTI_ROLL_PWM_MAX]);                                  // calculate motor speed (pwm) by force between min pwm and max pwm speed

    // which direction?
    if (forces[MEM_ROLL] > 0) {
      analogWrite(ROLL_R_PWM, 0);           // stop right
      analogWrite(ROLL_L_PWM, roll_speed);  // speed up left
    }
    else {
      analogWrite(ROLL_L_PWM, 0);           // stop left
      analogWrite(ROLL_R_PWM, roll_speed);  // speed up right
    }
  }

#ifdef DEBUG
  log(19, roll_speed);
  log(20, pitch_speed);
#endif
} //DriveMotors

/*
   Check if calibration mode is started
*/
void CheckCalibrationMode() {
  // is button pressed?
  if (buttonPinStates[ADJ_BUTTON_CALIBRATION] == true) {
    // read again for debouncing
    while (buttonPinStates[ADJ_BUTTON_CALIBRATION] == true) {
      delay(1);
      ReadMux();
    }
    // set flag for all directions
    calibration_mode_roll_min = true;
    calibration_mode_roll_max = true;
    calibration_mode_pitch_min = true;
    calibration_mode_pitch_max = true;
    // dsiable motor
    DisableMotors();
    // Print message to display for center position
    LcdPrintCalibrationMiddle();

    // wait for button press
    while (buttonPinStates[ADJ_BUTTON_CALIBRATION] == false) {
      delay(1);
      ReadMux();
    }
    // reset counters
    counterRoll.readAndReset();
    counterPitch.readAndReset();
    // small wait
    delay(1000);
    // print message to display for moving to all axes end
    LcdPrintCalibrationAxesStart();
    // wait until all axes are received
    while (calibration_mode_roll_min
           || calibration_mode_roll_max
           || calibration_mode_pitch_min
           || calibration_mode_pitch_max)
    {
      // read values
      ReadMux();
      // if pitch down end switch is reached save value
      if (buttonPinStates[ADJ_ENDSWITCH_PITCH_DOWN] == 0 && calibration_mode_pitch_min) {
        poti_pitch_min = counterPitch.read();
        calibration_mode_pitch_min = false;
        LcdPrintCalibrationAxesUpdate();
      }
      // if pitch up end switch is reached save value
      if (buttonPinStates[ADJ_ENDSWITCH_PITCH_UP] == 0 && calibration_mode_pitch_max) {
        poti_pitch_max = counterPitch.read();
        calibration_mode_pitch_max = false;
        LcdPrintCalibrationAxesUpdate();
      }
      // if roll left end switch is reached save value
      if (buttonPinStates[ADJ_ENDSWITCH_ROLL_LEFT] == 0 && calibration_mode_roll_min) {  //ir sensor
      //if (buttonPinStates[ADJ_ENDSWITCH_ROLL_LEFT] > 0 && calibration_mode_roll_min) {  //microswitch
        poti_roll_min = counterRoll.read();
        calibration_mode_roll_min = false;
        LcdPrintCalibrationAxesUpdate();
      }
      // if roll right end switch is reached save value
      if (buttonPinStates[ADJ_ENDSWITCH_ROLL_RIGHT] == 0 && calibration_mode_roll_max) {   // ir sensor
      //if (buttonPinStates[ADJ_ENDSWITCH_ROLL_RIGHT] > 0 && calibration_mode_roll_max) {  // microswitch
        poti_roll_max = counterRoll.read();
        calibration_mode_roll_max = false;
        LcdPrintCalibrationAxesUpdate();
      }
    } // while

    // calculate max values for Joystick axes 
    // this uses the absolute values
    // e.g. (-201) and (+200) means 201 for max valuev
    // e.g. (-198) and (+200) means 200 for max value
    if (abs(poti_pitch_min) > abs(poti_pitch_max))
    {
      JOYSTICK_minY = abs(poti_pitch_min) * (-1);
      JOYSTICK_maxY = abs(poti_pitch_min);
    } else {
      JOYSTICK_minY = abs(poti_pitch_max) * (-1);
      JOYSTICK_maxY = abs(poti_pitch_max);
    }

    if (abs(poti_roll_min) > abs(poti_roll_max))
    {
      JOYSTICK_minX = abs(poti_roll_min) * (-1);
      JOYSTICK_maxX = abs(poti_roll_min);
    } else {
      JOYSTICK_minX = abs(poti_roll_max) * (-1);
      JOYSTICK_maxX = abs(poti_roll_max);
    }
    // wait to give the user time so read the last value
    delay(3000);
    // set the new Joystick range
    setRangeJoystick();
    // clear display
    lcd.clear();
    // enable motors
    EnableMotors();
  } else {
    // no calibration than show adjustment values to display
    LcdPrintAdjustmendValues();
  }
} // CheckCalibrationMode


/******************************************
   Reads the button states over multiplexer
*******************************************/
void ReadMux() {
  // A0=PF7 = S0
  // A1=PF6 = S1
  // A2=PF5 = S2
  // A3=PF4 =  S3

  for (byte x = 0; x < 16; x++) {
    // s0 = Pin A0 - PortD4
    if (x & B00000001)
    {
      PORTF = PORTF | B10000000;
    } else {
      PORTF = PORTF & B01111111;
    }

    // s1 =  Pin A1 - PortC6
    if (x & B00000010)
    {
      PORTF = PORTF | B01000000;
    } else {
      PORTF = PORTF & B10111111;
    }

    // s2 = Pin A2 PortD0
    if (x & B00000100)
    {
      PORTF = PORTF | B00100000;
    } else {
      PORTF = PORTF & B11011111;
    }

    // s3 = Pin A3 - PortE6
    if (x & B00001000)
    {
      PORTF = PORTF | B00010000;
    } else {
      PORTF = PORTF & B11101111;
    }

    // enable mux 1
    PORTC = PORTC & B10111111; // Digital Pin 5 - PortC6
    // wait
    delayMicroseconds(1);
    // read value
    buttonPinStates[x] = digitalRead(MUX_SIGNAL_YOKE);
    // disblae mux1
    PORTC = PORTC | B01000000; // Digital Pin 5 - PortC6

    // enable mux 2
    PORTD = PORTD & B11101111; // Digital Pin 4 - PortD4
    // wait
    delayMicroseconds(1);

    if (x < 10) {
      buttonPinStates[x + 16] = digitalRead(MUX_SIGNAL_INPUT);
    } else {
      switch(x-10){
        case 0:
          adjustments_input[ADJ_POTI_PITCH_FORCE_MAX] = map(analogRead(MUX_SIGNAL_INPUT), 0, 1023, 0, 32767);
          break;
        case 1:
          adjustments_input[ADJ_POTI_PITCH_PWM_MAX] = map(analogRead(MUX_SIGNAL_INPUT), 0, 1023, 0, 255);
          break;
        case 2:
          adjustments_input[ADJ_POTI_PITCH_PWM_MIN] = map(analogRead(MUX_SIGNAL_INPUT), 0, 1023, 0, 255);
          break;
        case 3:
          adjustments_input[ADJ_POTI_ROLL_FORCE_MAX] = map(analogRead(MUX_SIGNAL_INPUT), 0, 1023, 0, 32767);
          break;
        case 4:
          adjustments_input[ADJ_POTI_ROLL_PWM_MAX] = map(analogRead(MUX_SIGNAL_INPUT), 0, 1023, 0, 255);
          break;
        case 5:
          adjustments_input[ADJ_POTI_ROLL_PWM_MIN] = map(analogRead(MUX_SIGNAL_INPUT), 0, 1023, 0, 255);
          break;             
      }
    }
    // disblae mux
    PORTD = PORTD | B00010000; // Digital Pin 4 - PortD4
  }//for

  // debug
#ifdef DEBUG
  log(21, buttonPinStates[ADJ_ENDSWITCH_PITCH_DOWN]);
  log(22, buttonPinStates[ADJ_ENDSWITCH_PITCH_UP]);
  log(23, buttonPinStates[ADJ_ENDSWITCH_ROLL_LEFT]);
  log(24, buttonPinStates[ADJ_ENDSWITCH_ROLL_RIGHT]);
  log(25, buttonPinStates[ADJ_BUTTON_CALIBRATION]);
  log(26, 0);//adjustments_input[ADJ_POTI_PITCH_FORCE_MAX]);
  log(27, 0);//adjustments_input[ADJ_POTI_PITCH_PWM_MAX]);
  log(28, 0);//adjustments_input[ADJ_POTI_PITCH_PWM_MIN]);
  log(29, 0);//adjustments_input[ADJ_POTI_ROLL_FORCE_MAX]);
  log(30, 0);//adjustments_input[ADJ_POTI_ROLL_PWM_MAX]);
  log(31, 0);//adjustments_input[ADJ_POTI_ROLL_PWM_MIN]);
#endif
}// ReadMux

void UpdateJoystickButtons() {
  //decode HatSwitch Positions
  if (buttonPinStates[2] == false &&  buttonPinStates[0] == false && buttonPinStates[1] == false && buttonPinStates[3] == false)
    Joystick.setHatSwitch(0, -1); // no direction

  if (buttonPinStates[2] == true &&  buttonPinStates[0] == false && buttonPinStates[1] == false && buttonPinStates[3] == false)
    Joystick.setHatSwitch(0, 0);     // up

  if (buttonPinStates[2] == true &&  buttonPinStates[0] == true && buttonPinStates[1] == false && buttonPinStates[3] == false)
    Joystick.setHatSwitch(0, 45);    // up right

  if (buttonPinStates[2] == false &&  buttonPinStates[0] == true && buttonPinStates[1] == false && buttonPinStates[3] == false)
    Joystick.setHatSwitch(0, 90);    // right

  if (buttonPinStates[2] == false &&  buttonPinStates[0] == true && buttonPinStates[1] == true && buttonPinStates[3] == false)
    Joystick.setHatSwitch(0, 135);   // down right

  if (buttonPinStates[2] == false &&  buttonPinStates[0] == false && buttonPinStates[1] == true && buttonPinStates[3] == false)
    Joystick.setHatSwitch(0, 180);   // down

  if (buttonPinStates[2] == false &&  buttonPinStates[0] == false && buttonPinStates[1] == true && buttonPinStates[3] == true)
    Joystick.setHatSwitch(0, 225);   // down left

  if (buttonPinStates[2] == false &&  buttonPinStates[0] == false && buttonPinStates[1] == false && buttonPinStates[3] == true)
    Joystick.setHatSwitch(0, 270);   // left

  if (buttonPinStates[2] == true &&  buttonPinStates[0] == false && buttonPinStates[1] == false && buttonPinStates[3] == true)
    Joystick.setHatSwitch(0, 315);  // up left

  // read button states from multiplexer
  for (byte channel = 0; channel < 16; channel++)
  {
    if (channel >= 4)
    {
      Joystick.setButton(channel - 4, buttonPinStates[channel]);
    }
#ifdef DEBUG
    log(32 + channel,  buttonPinStates[channel]);
#endif
  }
} //updateJoystickButtons



void log(byte key, int16_t value) {
  Serial.print(key);
  Serial.print(":");
  Serial.print(value);
  Serial.print(" ");
}

void log(byte key, bool value) {
  Serial.print(key);
  Serial.print(":");
  Serial.print(value);
  Serial.print(" ");
}

void log(byte key, int32_t value) {
  Serial.print(key);
  Serial.print(":");
  Serial.print(value);
  Serial.print(" ");
}
