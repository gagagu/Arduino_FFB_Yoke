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
#define ADJ_ENDSWITCH_PITCH_DOWN 0
#define ADJ_ENDSWITCH_PITCH_UP 1
#define ADJ_ENDSWITCH_ROLL_LEFT 2
#define ADJ_ENDSWITCH_ROLL_RIGHT 3
#define ADJ_BUTTON_CALIBRATION 4

// If you use center sensor use 
// RES_1 connector for Roll Roll Sensor
// RES_2 connector for Pitch Sensor
#if defined(LANGUAGE_EN)
  #define ADJ_MIDDLESWITCH_ROLL 5
  #define ADJ_MIDDLESWITCH_PITCH 6
#endif

// variables for Speed calculation
byte roll_speed = 0;
byte pitch_speed = 0;

// variables for button pin states
uint16_t iYokeButtonPinStates = 0;
uint16_t iSensorPinStates = 0;

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
} //ArduinoSetup

/**************************
  Enables the motordrivers
****************************/
void EnableMotors() {
  digitalWrite(PITCH_EN, HIGH);
  digitalWrite(ROLL_EN, HIGH);
} //EnableMotors

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
} //DisableMotors

/******************************************************
  calculates the motor speeds and controls the motors
******************************************************/
void DriveMotors() {
  // calculate motor speed for pitch direction
  CalculateSpeed(pitch_speed,
                 (iSensorPinStates & (1 << ADJ_ENDSWITCH_PITCH_DOWN)),
                 (iSensorPinStates & (1 << ADJ_ENDSWITCH_PITCH_UP)),
                 PITCH_R_PWM,
                 PITCH_L_PWM,
                 forces[MEM_PITCH],
                 adjForceMax[MEM_PITCH],
                 adjPwmMin[MEM_PITCH],
                 adjPwmMax[MEM_PITCH]);

  // calculate motor speed for roll direction
  CalculateSpeed(roll_speed,
                 (iSensorPinStates & (1 << ADJ_ENDSWITCH_ROLL_LEFT)),
                 (iSensorPinStates & (1 << ADJ_ENDSWITCH_ROLL_RIGHT)),
                 ROLL_L_PWM,
                 ROLL_R_PWM,
                 forces[MEM_ROLL],
                 adjForceMax[MEM_ROLL],
                 adjPwmMin[MEM_ROLL],
                 adjPwmMax[MEM_ROLL]);
} //DriveMotors

/******************************************************
  calculates the motor speeds
******************************************************/
int CalculateSpeed(byte &rSpeed,
                   bool blEndswitchDown,
                   bool blEndswitchUp,
                   byte pinLPWM,
                   byte pinRPWM,
                   int16_t gForce,
                   int forceMax,
                   byte pwmMin,
                   byte pwmMax)
{
  // if position is on end switch then stop the motor
  if (blEndswitchDown == 0 || blEndswitchUp == 0)
  {
    analogWrite(pinLPWM, 0);  // stop left
    analogWrite(pinRPWM, 0);  // stop right
    rSpeed = 0;              // speed to 0
  }
  else {
    // cut force to maximum value
    int pForce = constrain(abs(gForce), 0, forceMax);
    // calculate motor speed (pwm) by force between min pwm and max pwm speed
    rSpeed = map(pForce, 0, forceMax, pwmMin, pwmMax);

    // which direction?
    if (gForce > 0) {
      analogWrite(pinRPWM, 0);       // stop right
      analogWrite(pinLPWM, rSpeed);  // speed up left
    }
    else {
      analogWrite(pinLPWM, 0);       // stop left
      analogWrite(pinRPWM, rSpeed);  // speed up right
    }
  }
} //CalculateSpeed

/******************************************************
  Calibration
******************************************************/
void CheckCalibrationMode() {
  // is calibration button pressed or system started
  if ((iSensorPinStates & (1 << ADJ_BUTTON_CALIBRATION)) || blStart == true) {
    // read again for debouncing button, not needed for start
    if (blStart == false)
    {
      while (iSensorPinStates & (1 << ADJ_BUTTON_CALIBRATION)) {
        delay(1);
        ReadMux();
      }
    } else {
      blStart = false;
    }

    // maximum and minimum counter values for roll
    int poti_roll_min = -32768;                             //counter min for Init;
    int poti_roll_max = -32768;                             //counter max for Init;

    // maximum and minimum counter values for pitch
    int poti_pitch_min = -32768;                             //counter min for Init;
    int poti_pitch_max = -32768;                            //counter max for Init;

    // dsiable motor
    DisableMotors();

#if defined(LANGUAGE_EN)
    // Print message to display for center position
    LcdPrintCalibrationMiddleSensors();
    
    // measure center by ir sensors
    byte byteFlag=0;
    while(byteFlag!= B00000011)
    {
      delay(1);
      ReadMux();
      
      // If Center Roll Sensor detects reset Roll counter
      if((iSensorPinStates & (1 << ADJ_MIDDLESWITCH_ROLL))==0){
        counterRoll.readAndReset(); 
        byteFlag=byteFlag | B00000001;
      }
    
      // If Center Pitch Sensor detects reset Roll counter
      if((iSensorPinStates & (1 << ADJ_MIDDLESWITCH_PITCH))==0){
        counterPitch.readAndReset(); 
        byteFlag=byteFlag | B00000010;
      }
    }
#else
    // Print message to display for center position
    LcdPrintCalibrationMiddle();

    // wait for button press to measure center
    while ((iSensorPinStates & (1 << ADJ_BUTTON_CALIBRATION)) == LOW) {
      delay(1);
      ReadMux();
    }
    // reset counters to zero
    counterRoll.readAndReset();
    counterPitch.readAndReset();
#endif    


    // small wait; not really needed but nicer in behavior
    delay(500);

    // print message to display for moving to all axes end
    LcdPrintCalibrationAxesStart();

    // do until all axes are received
    while (poti_roll_min == -32768 || poti_roll_max == -32768 || poti_pitch_min == -32768 || poti_pitch_max == -32768)
    {
      // read end switches to detect changes
      ReadMux();

      // if pitch down end switch is reached save value
      if (((iSensorPinStates & (1 << ADJ_ENDSWITCH_PITCH_DOWN)) == LOW) && poti_pitch_min==-32768) {
        poti_pitch_min = counterPitch.read();
        LcdPrintCalibrationAxesUpdate(poti_roll_min, poti_roll_max, poti_pitch_max, poti_pitch_min);
      }

      // if pitch up end switch is reached save value
      if (((iSensorPinStates & (1 << ADJ_ENDSWITCH_PITCH_UP)) == LOW)&& poti_pitch_max==-32768) {
        poti_pitch_max = counterPitch.read();
        LcdPrintCalibrationAxesUpdate(poti_roll_min, poti_roll_max, poti_pitch_max, poti_pitch_min);
      }

      // if roll left end switch is reached save value
      if (((iSensorPinStates & (1 << ADJ_ENDSWITCH_ROLL_LEFT)) == LOW)&& poti_roll_min==-32768) {  //ir sensor
        //Serial.println("left");
        poti_roll_min = counterRoll.read();
        LcdPrintCalibrationAxesUpdate(poti_roll_min, poti_roll_max, poti_pitch_max, poti_pitch_min);
      }

      // if roll right end switch is reached save value
      if (((iSensorPinStates & (1 << ADJ_ENDSWITCH_ROLL_RIGHT)) == LOW)&& poti_roll_max==-32768) {   // ir sensor
        //Serial.println("right");
        poti_roll_max = counterRoll.read();
        LcdPrintCalibrationAxesUpdate(poti_roll_min, poti_roll_max, poti_pitch_max, poti_pitch_min);
      }
    } // while

    // calculate max values for Joystick axes
    // this uses the absolute values
    // e.g. (-201) and (+200) means 201 for max valuev
    // e.g. (-198) and (+200) means 200 for max value
    int absoluteMaxPitch = max(abs(poti_pitch_min), abs(poti_pitch_max));
    JOYSTICK_minY = -absoluteMaxPitch;
    JOYSTICK_maxY = absoluteMaxPitch;

    int absoluteMaxRoll = max(abs(poti_roll_min), abs(poti_roll_max));
    JOYSTICK_minX = -absoluteMaxRoll;
    JOYSTICK_maxX = absoluteMaxRoll;

    // wait to give the user time so read the last value from display
    delay(3000);

    // set the new Joystick range
    setRangeJoystick();

    // enable motors
    EnableMotors();

    // show adjustment values again
    LcdPrintAdjustmendValues();

  } // button not pressed
} // CheckCalibrationMode

/******************************************
   Reads the button states over multiplexer
*******************************************/
void ReadMux() {
  iYokeButtonPinStates = 0;
  iSensorPinStates = 0;

  // for every 16 imput lines of a multiplexer
  for (byte x = 0; x < 16; x++) {
    // direct port manipulation is faster than digitalwrite!
    // s0 = Pin A0 - PF7
    if (x & B00000001) // if bit 0 of counter is high
    {
      PORTF = PORTF | B10000000;  // set bit 7 of port (A0)
    } else {
      PORTF = PORTF & B01111111;  // clear bit 7 of port (A0)
    }

    // s1 =  Pin A1 - PF6
    if (x & B00000010)  // if bit 1 of counter is high
    {
      PORTF = PORTF | B01000000;  // set bit 6 of port (A1)
    } else {
      PORTF = PORTF & B10111111;  // clear bit 6 of port (A1)
    }

    // s2 = Pin A2 PF5
    if (x & B00000100) // if bit 2 of counter is high
    {
      PORTF = PORTF | B00100000;  // set bit 5 of port (A2)
    } else {
      PORTF = PORTF & B11011111; // clear bit 5 of port (A2)
    }

    // s3 = Pin A3 - PF4
    if (x & B00001000) // if bit 3 of counter is high
    {
      PORTF = PORTF | B00010000; // set bit 4 of port (A3)
    } else {
      PORTF = PORTF & B11101111; // clear bit 4 of port (A3)
    }

    // enable mux 1
    PORTC = PORTC & B10111111; // Digital Pin 5 - PortC6

    // wait for capacitors of mux to react
    delayMicroseconds(1);

    // read value
    if (x == 0)
    {
      iYokeButtonPinStates = digitalRead(MUX_SIGNAL_YOKE) << x;
    } else {
      iYokeButtonPinStates |= digitalRead(MUX_SIGNAL_YOKE) << x;
    }

    // disable mux1
    PORTC = PORTC | B01000000; // Digital Pin 5 - PortC6

    // enable mux 2
    PORTD = PORTD & B11101111; // Digital Pin 4 - PortD4

    // wait for capacitors of mux to react
    delayMicroseconds(1);

    // read value
    iSensorPinStates |= digitalRead(MUX_SIGNAL_INPUT) << x;

    if (x == 0)
    {
      iSensorPinStates = digitalRead(MUX_SIGNAL_INPUT) << x;
    } else {
      iSensorPinStates |= digitalRead(MUX_SIGNAL_INPUT) << x;
    }

    // disblae mux 2
    PORTD = PORTD | B00010000; // Digital Pin 4 - PortD4

  }//for

// only for censter sensors
#if defined(LANGUAGE_EN)
  // If Center Roll Sensor detects reset Roll counter
  if((iSensorPinStates & (1 << ADJ_MIDDLESWITCH_ROLL))==0){
    counterRoll.readAndReset(); 
  }

  // If Center Pitch Sensor detects reset Roll counter
  if((iSensorPinStates & (1 << ADJ_MIDDLESWITCH_PITCH))==0){
    counterPitch.readAndReset(); 
  }
#endif  
}// ReadMux

/******************************************
   set button states to Joystick library
*******************************************/
void UpdateJoystickButtons() {
  // detect hat switch position
  if ( iYokeButtonPinStates << 12 == 0B0000000000000000)
    Joystick.setHatSwitch(0, -1);   // no direction

  if ( iYokeButtonPinStates << 12 == 0B0100000000000000)
    Joystick.setHatSwitch(0, 0);    // up

  if ( iYokeButtonPinStates << 12 == 0B0101000000000000)
    Joystick.setHatSwitch(0, 45);   // up right

  if ( iYokeButtonPinStates << 12 == 0B0001000000000000)
    Joystick.setHatSwitch(0, 90);   // right

  if ( iYokeButtonPinStates << 12 == 0B0011000000000000)
    Joystick.setHatSwitch(0, 135);  // down right

  if ( iYokeButtonPinStates << 12 == 0B0010000000000000)
    Joystick.setHatSwitch(0, 180);  // down

  if ( iYokeButtonPinStates << 12 == 0B1010000000000000)
    Joystick.setHatSwitch(0, 225);   // down left

  if ( iYokeButtonPinStates << 12 == 0B1000000000000000)
    Joystick.setHatSwitch(0, 270);  // left

  if ( iYokeButtonPinStates << 12 == 0B1100000000000000)
    Joystick.setHatSwitch(0, 315);  // up left

  // read button states from multiplexer
  for (byte channel = 4; channel < 16; channel++)
  {
    Joystick.setButton(channel - 4, (iYokeButtonPinStates & (1 << channel)) >> channel);
  }
} //updateJoystickButtons
