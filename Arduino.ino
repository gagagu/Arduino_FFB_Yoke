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

// Shift Register for LEDs
#define SHIFT_DATA_PIN A3
#define SHIFT_CLOCK_PIN 12
#define SHIFT_LATCH_PIN 0

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
int adjustments_input[6] = {0, 0, 0, 0, 0, 0};

// flags for calibration mode
bool calibration_mode_roll_min = false;
bool calibration_mode_roll_max = false;
bool calibration_mode_pitch_min = false;
bool calibration_mode_pitch_max = false;

byte controlPins[] = {B00000000, B00000001, B00000010, B00000011, B00000100, B00000101, B00000110, B00000111, B00001000, B00001001, B00001010, B00001011, B00001100, B00001101, B00001110, B00001111};
bool buttonPinStates[] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
                          , false, false, false, false, false
                         };

int analogInputValues[] = {0, 0, 0, 0, 0, 0};

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
  digitalWrite(PITCH_EN, HIGH);
  digitalWrite(ROLL_EN, HIGH);
}

/***************************
  Disables the motordrivers
****************************/
void DisableMotors() {
  digitalWrite(PITCH_EN, HIGH);
  digitalWrite(ROLL_EN, HIGH);
}

/******************************************************
  calculates the motor speeds and controls the motors
******************************************************/
void DriveMotors() {

  // read Endswitch
  if (buttonPinStates[ADJ_ENDSWITCH_PITCH_DOWN] > 0 || buttonPinStates[ADJ_ENDSWITCH_PITCH_UP] > 0)
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
  if (buttonPinStates[ADJ_ENDSWITCH_ROLL_LEFT] > 0 || buttonPinStates[ADJ_ENDSWITCH_ROLL_RIGHT] > 0)
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
  if (buttonPinStates[ADJ_BUTTON_CALIBRATION] == true) {
    while (buttonPinStates[ADJ_BUTTON_CALIBRATION] == true) {
      delay(1);
      ReadMux();
    }
    calibration_mode_roll_min = true;
    calibration_mode_roll_max = true;
    calibration_mode_pitch_min = true;
    calibration_mode_pitch_max = true;

    DisableMotors();
    LcdCalibrationStart();

    while (calibration_mode_roll_min
           || calibration_mode_roll_max
           || calibration_mode_pitch_min
           || calibration_mode_pitch_max)
    {
      ReadMux();
      if (buttonPinStates[ADJ_ENDSWITCH_PITCH_DOWN] > 0 && calibration_mode_pitch_min) {
        poti_pitch_min = analogRead(POTI_PITCH);
        calibration_mode_pitch_min = false;
        LcdCalibrationUpdate();
      }

      if (buttonPinStates[ADJ_ENDSWITCH_PITCH_UP] > 0 && calibration_mode_pitch_max) {
        poti_pitch_max = analogRead(POTI_PITCH);
        calibration_mode_pitch_max = false;
        LcdCalibrationUpdate();
      }

      if (buttonPinStates[ADJ_ENDSWITCH_ROLL_LEFT] > 0 && calibration_mode_roll_min) {
        poti_roll_min = analogRead(POTI_ROLL);
        calibration_mode_roll_min = false;
        LcdCalibrationUpdate();
      }

      if (buttonPinStates[ADJ_ENDSWITCH_ROLL_RIGHT] > 0 && calibration_mode_roll_max) {
        poti_roll_max = analogRead(POTI_ROLL);
        calibration_mode_roll_max = false;
        LcdCalibrationUpdate();
      }
    } // while
    // wait to read the values
    delay(1000);
    // show middle text to lcd
    LcdCalibrationMiddle();
    // wait for button press
    while (buttonPinStates[ADJ_BUTTON_CALIBRATION] == false) {
      delay(1);
      ReadMux();
    }

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

    // wait for button release
    while (buttonPinStates[ADJ_BUTTON_CALIBRATION] == true) {
      delay(1);
      ReadMux();
    }
    lcd.clear();
    EnableMotors();
  } // button pressed
} // CheckCalibrationMode


/******************************************
   Reads the button states over multiplexer
*******************************************/
void ReadMux() {
  for (byte x = 0; x < 16; x++) {
    // s0 = digital pin 4 - PortD4
    if (controlPins[x] & B00000001)
    {
      PORTD = PORTD | B00010000;
    } else {
      PORTD = PORTD & B11101111;
    }

    // s1 =  digital pin 5 - PortC6
    if (controlPins[x] & B00000010)
    {
      PORTC = PORTC | B01000000;
    } else {
      PORTC = PORTC & B10111111;
    }

    // s2 = digital pin 3 PortD0
    if (controlPins[x] & B00000100)
    {
      PORTD = PORTD | B00000001;
    } else {
      PORTD = PORTD & B11111110;
    }

    // s3 = digital pin 7 - PortE6
    if (controlPins[x] & B00001000)
    {
      PORTE = PORTE | B01000000;
    } else {
      PORTE = PORTE & B10111111;
    }

    // enable mux 1
    PORTD = PORTD & B11111101; // Digital Pin 2 - PortD1
    // wait
    delayMicroseconds(1);
    // read value
    buttonPinStates[x] = digitalRead(MUX_SIGNAL_YOKE);
    // disblae mux1
    PORTD = PORTD | B00000010; // Digital Pin 2 - PortD1
    // enable mux 2
    PORTD = PORTD & B11110111; // Digital Pin 1 - PortD3
    // wait
    delayMicroseconds(1);

    if (x < 5) {
      buttonPinStates[x + 16] = digitalRead(MUX_SIGNAL_INPUT);
    } else {
      analogInputValues[x - 5] = analogRead(MUX_SIGNAL_INPUT);
    }
    // disblae mux
    PORTD = PORTD | B00001000; // Digital Pin 1 - PortD3
  }//for

  // Pitch
  adjustments_input[ADJ_POTI_PITCH_FORCE_MAX] = map(analogInputValues[ADJ_POTI_PITCH_FORCE_MAX], 0, 1023, 0, 32767);
  adjustments_input[ADJ_POTI_PITCH_PWM_MAX] = map(analogInputValues[ADJ_POTI_PITCH_PWM_MAX], 0, 1023, 0, 255);
  adjustments_input[ADJ_POTI_PITCH_PWM_MIN] = map(analogInputValues[ADJ_POTI_PITCH_PWM_MIN], 0, 1023, 0, 255);
  // Roll
  adjustments_input[ADJ_POTI_ROLL_FORCE_MAX] = map(analogInputValues[ADJ_POTI_ROLL_FORCE_MAX], 0, 1023, 0, 32767);
  adjustments_input[ADJ_POTI_ROLL_PWM_MAX] = map(analogInputValues[ADJ_POTI_ROLL_PWM_MAX], 0, 1023, 0, 255);
  adjustments_input[ADJ_POTI_ROLL_PWM_MIN] = map(analogInputValues[ADJ_POTI_ROLL_PWM_MIN], 0, 1023, 0, 255);

  // debug
#ifdef DEBUG
  log(21, buttonPinStates[ADJ_ENDSWITCH_PITCH_DOWN]);
  log(22, buttonPinStates[ADJ_ENDSWITCH_PITCH_UP]);
  log(23, buttonPinStates[ADJ_ENDSWITCH_ROLL_LEFT]);
  log(24, buttonPinStates[ADJ_ENDSWITCH_ROLL_RIGHT]);
  log(25, buttonPinStates[ADJ_BUTTON_CALIBRATION]);
  log(26, adjustments_input[ADJ_POTI_PITCH_FORCE_MAX]);
  log(27, adjustments_input[ADJ_POTI_PITCH_PWM_MAX]);
  log(28, adjustments_input[ADJ_POTI_PITCH_PWM_MIN]);
  log(29, adjustments_input[ADJ_POTI_ROLL_FORCE_MAX]);
  log(30, adjustments_input[ADJ_POTI_ROLL_PWM_MAX]);
  log(31, adjustments_input[ADJ_POTI_ROLL_PWM_MIN]);
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


/*
   Shows the adjustemt values to the lcd display
*/
void LcdAdjustmendValues(byte counter) {
  if (counter == 0)
  {
    lcd.setCursor(0, 0);
    LcdPrintFromStringTable(4); //Roll  Force:
    lcd.print(adjustments_input[ADJ_POTI_ROLL_FORCE_MAX]);
    return;
  }

  if (counter == 1)
  {
    lcd.setCursor(0, 1);
    LcdPrintFromStringTable(5); //PWM Mi-Ma:
    lcd.print(adjustments_input[ADJ_POTI_ROLL_PWM_MIN]);
    LcdPrintFromStringTable(8); //-
    lcd.print(adjustments_input[ADJ_POTI_ROLL_PWM_MAX]);
    return;
  }
  if (counter == 2)
  {
    lcd.setCursor(0, 2);
    LcdPrintFromStringTable(6); //Pitch Force:
    lcd.print(adjustments_input[ADJ_POTI_PITCH_FORCE_MAX]);
    return;
  }
  if (counter == 3)
  {
    lcd.setCursor(0, 3);
    LcdPrintFromStringTable(7); //PWM Mi-Ma:
    lcd.print(adjustments_input[ADJ_POTI_PITCH_PWM_MIN]);
    LcdPrintFromStringTable(8); //-
    lcd.print(adjustments_input[ADJ_POTI_PITCH_PWM_MAX]);
    return;
  }
} //LcdAdjustmendValues

/*
   Shows the calibration to LCD
*/
void LcdCalibrationMiddle() {
  lcd.clear();
  lcd.setCursor(0, 0);
  LcdPrintFromStringTable(16); //Put all axes in the
  lcd.setCursor(0, 1);
  LcdPrintFromStringTable(17); //middle position and
  lcd.setCursor(0, 2);
  LcdPrintFromStringTable(18); //press the button
  lcd.setCursor(0, 3);
  LcdPrintFromStringTable(19); //again
} //LcdCalibrationMiddle

/*
    Shows the calibration to LCD
*/
void LcdCalibrationUpdate() {
  lcd.clear();
  lcd.setCursor(0, 0);
  LcdPrintFromStringTable(12); //Left  end
  if (!calibration_mode_roll_min) {
    lcd.print(poti_roll_min);
  }
  lcd.setCursor(0, 1);
  LcdPrintFromStringTable(13); //right end
  if (!calibration_mode_roll_max) {
    lcd.print(poti_roll_max);
  }
  lcd.setCursor(0, 2);
  LcdPrintFromStringTable(14); // up end
  if (!calibration_mode_pitch_max) {
    lcd.print(poti_pitch_max);
  }
  lcd.setCursor(0, 3);
  LcdPrintFromStringTable(15); // down end
  if (!calibration_mode_pitch_min) {
    lcd.print(poti_pitch_min);
  }
} //LcdCalibrationUpdate

void LcdPrintSpaces(byte index, byte count) {
  strcpy_P(stringbuffer, (char*)pgm_read_word(&(string_table[20]))); // Casts und Dereferenzierung des Speichers
  for (byte x = index; x < count; x++)
  {
    lcd.print(stringbuffer);
  }
}

void LcdCalibrationStart()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  LcdPrintFromStringTable(9); //Calibration started.
  lcd.setCursor(0, 1);
  LcdPrintFromStringTable(10); //Please move all axes
  lcd.setCursor(0, 2);
  LcdPrintFromStringTable(11); //to the end positions
}

void LcdPrintIntro() {
  lcd.setCursor(0, 0);
  LcdPrintFromStringTable(0);
  lcd.setCursor(0, 1);
  LcdPrintFromStringTable(1);
  lcd.setCursor(0, 2);
  LcdPrintFromStringTable(2);
  lcd.setCursor(0, 3);
  LcdPrintFromStringTable(3);
}

/*
   Reads the string to print from prog mem
*/
void LcdPrintFromStringTable(byte stringPos) {
  strcpy_P(stringbuffer, (char*)pgm_read_word(&(string_table[stringPos]))); // Casts und Dereferenzierung des Speichers
  lcd.print(stringbuffer);
} //LcdPrintFromStringTable

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
