// Pitch Motordriver pins
#define PITCH_EN 11
#define PITCH_R_PWM 6
#define PITCH_L_PWM 13

// Roll Motordriver pins
#define ROLL_EN 8
#define ROLL_R_PWM 9
#define ROLL_L_PWM 10

//Potis
#define POTI_ROLL A0
#define POTI_PITCH A1

// multiplexer for buttons
#define MUX_S0 4
#define MUX_S1 5
#define MUX_S2 3
#define MUX_S3 7

// Multiplexer Yoke Buttons
#define MUX_EN_YOKE 2
#define MUX_SIGNAL_YOKE A5

// Multiplexer Adjustemts for Calib Button, Force Potis, End Switches
#define MUX_EN_INPUT 1
#define MUX_SIGNAL_INPUT A4

// Adjustments array positions
#define ADJ_ENDSWITCH_PITCH_DOWN 0
#define ADJ_ENDSWITCH_PITCH_UP 1
#define ADJ_ENDSWITCH_ROLL_LEFT 2
#define ADJ_ENDSWITCH_ROLL_RIGHT 3

#define ADJ_BUTTON_CALIBRATION 4

#define ADJ_POTI_PITCH_FORCE_MAX 5
#define ADJ_POTI_PITCH_PWM_MAX 6
#define ADJ_POTI_PITCH_PWM_MIN 7

#define ADJ_POTI_ROLL_FORCE_MAX 8
#define ADJ_POTI_ROLL_PWM_MAX 9
#define ADJ_POTI_ROLL_PWM_MIN 10

// Shift Register for LEDs
#define SHIFT_DATA_PIN A3
#define SHIFT_CLOCK_PIN 12
#define SHIFT_LATCH_PIN 0

// LEDs Bits
#define LED_BIT_POWER 6
#define LED_BIT_CALIBRATION 5
#define LED_BIT_ROLL_MIN 4
#define LED_BIT_ROLL_MAX 3
#define LED_BIT_PITCH_MIN 2
#define LED_BIT_PITCH_MAX 1
#define LED_BIT_RESERVED 0

// data output value for shift register (LEDs)
byte shift_register_data = 0;

// variables for Speed calculation
int roll_speed = 0;
int pitch_speed = 0;

// maximum and minimum poti values for roll
int poti_roll_min = 0;                                //Potentiometer min for Init;
int poti_roll_max = 1023;                             //Potentiometer max for Init;
int poti_roll_middle = 512;                           //Potentiometer middle;
byte poti_roll_deadzone = 10;                         //Potentiometer deadzone for middle

// maximum and minimum poti values for pitch
int poti_pitch_min = 0;                               //Potentiometer min for Init;
int poti_pitch_max = 1023;                            //Potentiometer max for Init;
int poti_pitch_middle = 512;                          //Potentiometer middle;
byte poti_pitch_deadzone = 10;                        //Potentiometer deadzone for middle

// input values array from Multiplexer Calib Button,
// Force Potis, End Switches
int adjustments_input[16];

// flags for calibration mode
bool calibration_mode = false;
bool calibration_mode_roll_min = false;
bool calibration_mode_roll_max = false;
bool calibration_mode_pitch_min = false;
bool calibration_mode_pitch_max = false;

// flag to show if motors are diables
bool are_motors_disabled = true;

/***************
  Pin setup
****************/
void ArduinoSetup()
{

  // Pitch Pins
  pinMode(POTI_PITCH, INPUT);
  pinMode(PITCH_EN, OUTPUT);
  pinMode(PITCH_R_PWM, OUTPUT);
  pinMode(PITCH_L_PWM, OUTPUT);

  // Roll Pins
  pinMode(POTI_ROLL, INPUT);
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

  // Shift Register for LEDs
  pinMode(SHIFT_DATA_PIN, OUTPUT);
  pinMode(SHIFT_CLOCK_PIN, OUTPUT);
  pinMode(SHIFT_LATCH_PIN, OUTPUT);

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

  // Shift Register for LEDs
  digitalWrite(SHIFT_DATA_PIN, LOW);
  digitalWrite(SHIFT_CLOCK_PIN, LOW);
  digitalWrite(SHIFT_LATCH_PIN, LOW);

  // not for all Arduinos!
  // This sets the PWM Speed to maximun for noise reduction

  // Timer1: pins 9 & 10
  TCCR1B = _BV(CS10); // change the PWM frequencey to 31.25kHz - pins 9 & 10

  // Timer4: pin 13 & 6
  TCCR4B = _BV(CS40); // change the PWM frequencey to 31.25kHz - pin 13 & 6

  // Enable Power LED
  SetLED(LED_BIT_POWER);
}

/****************************************
  Read the axes poti values and save it
****************************************/
void UpdateJoystickPos()
{
  // check if roll poti values are initialized for safety reason
  if (poti_roll_min == 0 && poti_roll_max == 0)
  {
    pos[MEM_ROLL] = 0; // not initialized than 0
  } else {
    int pot_roll = analogRead(POTI_ROLL); // read poti value

    // on middle +- deadzone than 0
    if ((pot_roll >= (poti_roll_middle - poti_roll_deadzone)) && (pot_roll <= (poti_roll_middle + poti_roll_deadzone)))
    {
      pos[MEM_ROLL] = 0; // not initialized than 0
    }
    else { // else calculate roll joystick value
      if (pot_roll < poti_roll_min) // check min
      {
        pot_roll = poti_roll_min;
      }
      if (pot_roll > poti_roll_max) // check max
      {
        pot_roll = poti_roll_max;
      }
      if (pot_roll > poti_roll_middle)
        pos[MEM_ROLL] = map(pot_roll, poti_roll_middle + poti_roll_deadzone, poti_roll_max, 0, JOYSTICK_maxY); // convert values to 0 till 32767 Joystick value
      else
        pos[MEM_ROLL] = map(pot_roll, poti_roll_min, poti_roll_middle - poti_roll_deadzone, JOYSTICK_minY, 0); // convert values to -32768 till 0 Joystick value
    }
  }


  // check if pitch poti values are initialized for safety reason
  if (poti_pitch_min == 0 && poti_pitch_max == 0)
  {
    pos[MEM_PITCH] = 0; // not initialized than 0
  } else {
    int pot_pitch = analogRead(POTI_PITCH); //read poti value
    // on middle +- deadzone than 0
    if ((pot_pitch >= (poti_pitch_middle - poti_pitch_deadzone)) && (pot_pitch <= (poti_pitch_middle + poti_pitch_deadzone)))
    {
      pos[MEM_PITCH] = 0; // not initialized than 0
    }
    else { // else calculate roll joystick value
      if (pot_pitch < poti_pitch_min) // check min
      {
        pot_pitch = poti_pitch_min;
      }
      if (pot_pitch > poti_pitch_max) // check max
      {
        pot_pitch = poti_pitch_max;
      }
      if (pot_pitch > poti_pitch_middle)
        pos[MEM_PITCH] = map(pot_pitch, poti_pitch_middle + poti_pitch_deadzone, poti_pitch_max, 0, JOYSTICK_maxY); // convert values to 0 till 32767 Joystick value
      else
        pos[MEM_PITCH] = map(pot_pitch, poti_pitch_min, poti_pitch_middle - poti_pitch_deadzone, JOYSTICK_minY, 0); // convert values to -32768 till 0 Joystick value
              
    }
  }

  // set Joystick axis
  Joystick.setXAxis(pos[MEM_ROLL]);
  Joystick.setYAxis(pos[MEM_PITCH]);

  pos_updated = true;

#ifdef DEBUG
  log(11, poti_roll_min);
  log(12, poti_roll_max);
  log(49, poti_roll_middle);
  log(50, poti_roll_deadzone);
  log(13, analogRead(POTI_ROLL));
  log(14, pos[MEM_ROLL]);

  log(15, poti_pitch_min);
  log(16, poti_pitch_max);
  log(51, poti_pitch_middle);
  log(52, poti_pitch_deadzone);
  log(17, analogRead(POTI_PITCH));
  log(18, pos[MEM_PITCH]);
#endif
}//ReadPots


/**************************
  Enables the motordrivers
****************************/
void EnableMotors() {
  if (are_motors_disabled) {
    digitalWrite(PITCH_EN, HIGH);
    digitalWrite(ROLL_EN, HIGH);
    are_motors_disabled = false;
  }
}

/***************************
  Disables the motordrivers
****************************/
void DisableMotors() {
  if (!are_motors_disabled) {
    digitalWrite(PITCH_EN, HIGH);
    digitalWrite(ROLL_EN, HIGH);
    are_motors_disabled = true;
  }
}

/******************************************************
  calculates the motor speeds and controls the motors
******************************************************/
void DriveMotors() {

  // read Endswitch
  if (adjustments_input[ADJ_ENDSWITCH_PITCH_DOWN] > 0 || adjustments_input[ADJ_ENDSWITCH_PITCH_UP] > 0)
  {
    analogWrite(PITCH_L_PWM, 0);  // stop left
    analogWrite(PITCH_R_PWM, 0);  // stop right
    pitch_speed = 0;              // speed to 0
  }
  else {
    int32_t pForce = constrain(abs(forces[MEM_PITCH]), 0, adjustments_input[ADJ_POTI_PITCH_FORCE_MAX]);    // cut force to maximum value
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

  // Roll forces
  if (adjustments_input[ADJ_ENDSWITCH_ROLL_LEFT] > 0 || adjustments_input[ADJ_ENDSWITCH_ROLL_RIGHT] > 0)
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


/*************************************
  Check if calibration mode is started
*************************************/
bool CheckCalibrationMode() {
  if (calibration_mode) {
    return calibration_mode;
  } else {
    if (adjustments_input[ADJ_BUTTON_CALIBRATION] > 0) {
      delay(1000);
      if (adjustments_input[ADJ_BUTTON_CALIBRATION] > 0) {
        SetLED(LED_BIT_CALIBRATION);    // Enable Calibration LED
        SetLED(LED_BIT_ROLL_MIN);    // Enable Roll Min LED
        SetLED(LED_BIT_ROLL_MAX);    // Enable Roll Max LED
        SetLED(LED_BIT_PITCH_MIN);    // Enable Pitch Min LED
        SetLED(LED_BIT_PITCH_MAX);    // Enable Pitch Max LED
        calibration_mode = true;
        calibration_mode_roll_min = true;
        calibration_mode_roll_max = true;
        calibration_mode_pitch_min = true;
        calibration_mode_pitch_max = true;
#ifdef DEBUG
        // Serial.println("Start calibration:");
#endif
      } // if button
      return calibration_mode;
    } else {
      return false;
    }
  }
} // CheckCalibrationMode

/*************************************
  Measure the Yoke Axes Potentiometers
*************************************/
void UpdateCalibration()
{
  if (calibration_mode_roll_min
      || calibration_mode_roll_max
      || calibration_mode_pitch_min
      || calibration_mode_pitch_max)
  {
    if (adjustments_input[ADJ_ENDSWITCH_PITCH_DOWN] > 0 && calibration_mode_pitch_min) {
      poti_pitch_min = analogRead(POTI_PITCH);
      calibration_mode_pitch_min = false;
      ClearLED(LED_BIT_PITCH_MIN);
#ifdef DEBUG
      //Serial.print("Pitch Down:");
      //Serial.println(poti_pitch_min);
#endif
    }

    if (adjustments_input[ADJ_ENDSWITCH_PITCH_UP] > 0 && calibration_mode_pitch_max) {
      poti_pitch_max = analogRead(POTI_PITCH);
      calibration_mode_pitch_max = false;
      ClearLED(LED_BIT_PITCH_MAX);
#ifdef DEBUG
      //Serial.print("Pitch Up:");
      //Serial.println(poti_pitch_max);
#endif
    }

    if (adjustments_input[ADJ_ENDSWITCH_ROLL_LEFT] > 0 && calibration_mode_roll_min) {
      poti_roll_min = analogRead(POTI_ROLL);
      calibration_mode_roll_min = false;
      ClearLED(LED_BIT_ROLL_MIN);
#ifdef DEBUG
      // Serial.print("Roll Down:");
      //Serial.println(poti_roll_min);
#endif
    }

    if (adjustments_input[ADJ_ENDSWITCH_ROLL_RIGHT] > 0 && calibration_mode_roll_max) {
      poti_roll_max = analogRead(POTI_ROLL);
      calibration_mode_roll_max = false;
      ClearLED(LED_BIT_ROLL_MAX);
#ifdef DEBUG
      // Serial.print("Roll Up:");
      // Serial.println(poti_roll_max);
#endif
    }

  } else {
    if (adjustments_input[ADJ_BUTTON_CALIBRATION] > 0) {
      delay(200);
      if (adjustments_input[ADJ_BUTTON_CALIBRATION] > 0) {
        poti_roll_middle = analogRead(POTI_ROLL);
        poti_pitch_middle = analogRead(POTI_PITCH);

        if (poti_pitch_min > poti_pitch_max)
        {
          int tmp_val = poti_pitch_max;
          poti_pitch_max = poti_pitch_min;
          poti_pitch_min = tmp_val;
        }
        if (poti_roll_min > poti_roll_max)
        {
          int tmp_val = poti_roll_max;
          poti_roll_max = poti_roll_min;
          poti_roll_min = tmp_val;
        }
#ifdef DEBUG
        //Serial.println("Calibration End");
        delay(1000);
#endif
        calibration_mode = false;
        ClearLED(LED_BIT_CALIBRATION);  // clear Calibration LED
      }
    }
  }
} //UpdateCalibration

/*************************************
  Reads an Digital Values from the multiplexer (Analog Pin)
*************************************/
bool DigitalReadMux(bool s0, bool s1, bool s2, bool s3, byte pinEnable, byte pinRead)
{
  // set mux channel
  digitalWrite(MUX_S0, s0);
  digitalWrite(MUX_S1, s1);
  digitalWrite(MUX_S2, s2);
  digitalWrite(MUX_S3, s3);
  // enable mux (pleparation for additional muxers for e.g. force adjustments or so)
  digitalWrite(pinEnable, LOW);
  delay(1);
  // read value
  bool val = false;
  if (analogRead(pinRead) > 100)
    val = true;

  // disblae mux
  digitalWrite(pinEnable, HIGH);

  return val;
} //DigitalReadMux

/*************************************
  Reads an Alalog Values from the multiplexer
*************************************/
int AnalogReadMux(bool s0, bool s1, bool s2, bool s3, byte pinEnable, byte pinRead)
{
  // set mux channel
  digitalWrite(MUX_S0, s0);
  digitalWrite(MUX_S1, s1);
  digitalWrite(MUX_S2, s2);
  digitalWrite(MUX_S3, s3);
  // enable mux (pleparation for additional muxers for e.g. force adjustments or so)
  digitalWrite(pinEnable, LOW);
  // read value
  delay(1);
  int val = analogRead(pinRead);
  // disblae mux
  digitalWrite(pinEnable, HIGH);

  return val;
} //AnalogReadMux

/**************************************
   turn one LED on given by bit number
***************************************/
void SetLED(byte number) {
  bitSet(shift_register_data, number);
}

/****************************************
   switch one LED off given by bit number
****************************************/
void ClearLED(byte number) {
  bitClear(shift_register_data, number);
}

/*******************************
   switch all LEDs off
*******************************/
void ClearAllLEDs() {
  shift_register_data = 0;
}

/*******************************************
  Sets the Shift Register with the values
  saved in shift_register_data
  for LED display
******************************************/
void SetShiftRegister() {
  digitalWrite(SHIFT_LATCH_PIN, LOW);
  // shift out the bits:
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, shift_register_data);
  //take the latch pin high so the LEDs will light up:
  digitalWrite(SHIFT_LATCH_PIN, HIGH);
}//SetShiftRegister

/************************************************
   Reads the adjustemts values from Multiplexer
   like end switches, force potentiometers or
   calibration button
 ***********************************************/
void UpdateAdjustmentsValues() {
  // buttons
  adjustments_input[ADJ_ENDSWITCH_PITCH_DOWN] = DigitalReadMux(bitRead(ADJ_ENDSWITCH_PITCH_DOWN, 0), bitRead(ADJ_ENDSWITCH_PITCH_DOWN, 1), bitRead(ADJ_ENDSWITCH_PITCH_DOWN, 2), bitRead(ADJ_ENDSWITCH_PITCH_DOWN, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT);
  adjustments_input[ADJ_ENDSWITCH_PITCH_UP] = DigitalReadMux(bitRead(ADJ_ENDSWITCH_PITCH_UP, 0), bitRead(ADJ_ENDSWITCH_PITCH_UP, 1), bitRead(ADJ_ENDSWITCH_PITCH_UP, 2), bitRead(ADJ_ENDSWITCH_PITCH_UP, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT);
  adjustments_input[ADJ_ENDSWITCH_ROLL_LEFT] = DigitalReadMux(bitRead(ADJ_ENDSWITCH_ROLL_LEFT, 0), bitRead(ADJ_ENDSWITCH_ROLL_LEFT, 1), bitRead(ADJ_ENDSWITCH_ROLL_LEFT, 2), bitRead(ADJ_ENDSWITCH_ROLL_LEFT, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT);
  adjustments_input[ADJ_ENDSWITCH_ROLL_RIGHT] = DigitalReadMux(bitRead(ADJ_ENDSWITCH_ROLL_RIGHT, 0), bitRead(ADJ_ENDSWITCH_ROLL_RIGHT, 1), bitRead(ADJ_ENDSWITCH_ROLL_RIGHT, 2), bitRead(ADJ_ENDSWITCH_ROLL_RIGHT, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT);
  adjustments_input[ADJ_BUTTON_CALIBRATION] = DigitalReadMux(bitRead(ADJ_BUTTON_CALIBRATION, 0), bitRead(ADJ_BUTTON_CALIBRATION, 1), bitRead(ADJ_BUTTON_CALIBRATION, 2), bitRead(ADJ_BUTTON_CALIBRATION, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT);
  // Pitch
  adjustments_input[ADJ_POTI_PITCH_FORCE_MAX] = map(AnalogReadMux(bitRead(ADJ_POTI_PITCH_FORCE_MAX, 0), bitRead(ADJ_POTI_PITCH_FORCE_MAX, 1), bitRead(ADJ_POTI_PITCH_FORCE_MAX, 2), bitRead(ADJ_POTI_PITCH_FORCE_MAX, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT), 0, 1023, 0, 32767);
  adjustments_input[ADJ_POTI_PITCH_PWM_MAX] = map(AnalogReadMux(bitRead(ADJ_POTI_PITCH_PWM_MAX, 0), bitRead(ADJ_POTI_PITCH_PWM_MAX, 1), bitRead(ADJ_POTI_PITCH_PWM_MAX, 2), bitRead(ADJ_POTI_PITCH_PWM_MAX, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT), 0, 1023, 0, 255);
  adjustments_input[ADJ_POTI_PITCH_PWM_MIN] = map(AnalogReadMux(bitRead(ADJ_POTI_PITCH_PWM_MIN, 0), bitRead(ADJ_POTI_PITCH_PWM_MIN, 1), bitRead(ADJ_POTI_PITCH_PWM_MIN, 2), bitRead(ADJ_POTI_PITCH_PWM_MIN, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT), 0, 1023, 0, 255);
  // Roll
  adjustments_input[ADJ_POTI_ROLL_FORCE_MAX] = map(AnalogReadMux(bitRead(ADJ_POTI_ROLL_FORCE_MAX, 0), bitRead(ADJ_POTI_ROLL_FORCE_MAX, 1), bitRead(ADJ_POTI_ROLL_FORCE_MAX, 2), bitRead(ADJ_POTI_ROLL_FORCE_MAX, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT), 0, 1023, 0, 32767);
  adjustments_input[ADJ_POTI_ROLL_PWM_MAX] = map(AnalogReadMux(bitRead(ADJ_POTI_ROLL_PWM_MAX, 0), bitRead(ADJ_POTI_ROLL_PWM_MAX, 1), bitRead(ADJ_POTI_ROLL_PWM_MAX, 2), bitRead(ADJ_POTI_ROLL_PWM_MAX, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT), 0, 1023, 0, 255);
  adjustments_input[ADJ_POTI_ROLL_PWM_MIN] = map(AnalogReadMux(bitRead(ADJ_POTI_ROLL_PWM_MIN, 0), bitRead(ADJ_POTI_ROLL_PWM_MIN, 1), bitRead(ADJ_POTI_ROLL_PWM_MIN, 2), bitRead(ADJ_POTI_ROLL_PWM_MIN, 3), MUX_EN_INPUT, MUX_SIGNAL_INPUT), 0, 1023, 0, 255);
  // debug
#ifdef DEBUG
  log(21, adjustments_input[ADJ_ENDSWITCH_PITCH_DOWN]);
  log(22, adjustments_input[ADJ_ENDSWITCH_PITCH_UP]);
  log(23, adjustments_input[ADJ_ENDSWITCH_ROLL_LEFT]);
  log(24, adjustments_input[ADJ_ENDSWITCH_ROLL_RIGHT]);
  log(25, adjustments_input[ADJ_BUTTON_CALIBRATION]);
  log(26, adjustments_input[ADJ_POTI_PITCH_FORCE_MAX]);
  log(27, adjustments_input[ADJ_POTI_PITCH_PWM_MAX]);
  log(28, adjustments_input[ADJ_POTI_PITCH_PWM_MIN]);
  log(29, adjustments_input[ADJ_POTI_ROLL_FORCE_MAX]);
  log(30, adjustments_input[ADJ_POTI_ROLL_PWM_MAX]);
  log(31, adjustments_input[ADJ_POTI_ROLL_PWM_MIN]);
#endif
} //UpdateAdjustmentsValues


/******************************************
   Reads the button states over multiplexer
*******************************************/
void UpdateJoystickButtons() {
  // read hat states from multiplexer channel 0-3
  bool hat_right = DigitalReadMux(0, 0, 0, 0, MUX_EN_YOKE, MUX_SIGNAL_YOKE);
  bool hat_down = DigitalReadMux(1, 0, 0, 0, MUX_EN_YOKE, MUX_SIGNAL_YOKE);
  bool hat_up = DigitalReadMux(0, 1, 0, 0, MUX_EN_YOKE, MUX_SIGNAL_YOKE);
  bool hat_left = DigitalReadMux(1, 1, 0, 0, MUX_EN_YOKE, MUX_SIGNAL_YOKE);

  //decode HatSwitch Positions
  if (hat_up == false &&  hat_right == false && hat_down == false && hat_left == false)
    Joystick.setHatSwitch(0, -1); // no direction

  if (hat_up == true &&  hat_right == false && hat_down == false && hat_left == false)
    Joystick.setHatSwitch(0, 0);     // up

  if (hat_up == true &&  hat_right == true && hat_down == false && hat_left == false)
    Joystick.setHatSwitch(0, 45);    // up right

  if (hat_up == false &&  hat_right == true && hat_down == false && hat_left == false)
    Joystick.setHatSwitch(0, 90);    // right

  if (hat_up == false &&  hat_right == true && hat_down == true && hat_left == false)
    Joystick.setHatSwitch(0, 135);   // down right

  if (hat_up == false &&  hat_right == false && hat_down == true && hat_left == false)
    Joystick.setHatSwitch(0, 180);   // down

  if (hat_up == false &&  hat_right == false && hat_down == true && hat_left == true)
    Joystick.setHatSwitch(0, 225);   // down left

  if (hat_up == false &&  hat_right == false && hat_down == false && hat_left == true)
    Joystick.setHatSwitch(0, 270);   // left

  if (hat_up == true &&  hat_right == false && hat_down == false && hat_left == true)
    Joystick.setHatSwitch(0, 315);  // up left

#ifdef DEBUG
  log(32, hat_right);
  log(33, hat_down);
  log(34, hat_up);
  log(35, hat_left);
#endif

  // read button states from multiplexer
  for (int channel = 4; channel < 16; channel++)
  {
    bool val = DigitalReadMux(bitRead(channel, 0), bitRead(channel, 1), bitRead(channel, 2), bitRead(channel, 3), MUX_EN_YOKE, MUX_SIGNAL_YOKE);
    Joystick.setButton(channel - 4, val);
#ifdef DEBUG
    log((36 - 4) + channel, val);
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
  //  Serial.println("hallo");
}
