/* 
 Created by A.Eckers aka Gagagu
 http://www.gagagu.de
 https://github.com/gagagu/Arduino_FFB_Yoke
 https://www.youtube.com/@gagagu01
*/

/***************
  Pin setup
****************/
void ArduinoSetup() {

  // Pitch motor driver pins
  pinMode(PITCH_EN,    OUTPUT);
  pinMode(PITCH_U_PWM, OUTPUT);
  pinMode(PITCH_D_PWM, OUTPUT);

  // Roll motor driver pins
  pinMode(ROLL_EN,    OUTPUT);
  pinMode(ROLL_R_PWM, OUTPUT);
  pinMode(ROLL_L_PWM, OUTPUT);

  // Buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);

#ifdef ARDUINO_PRO_MICRO
  // Multiplexer – Yoke buttons
  pinMode(MUX_YOKE_OUT, INPUT);
  pinMode(MUX_YOKE_PL,  OUTPUT);
  pinMode(MUX_YOKE_CLK, OUTPUT);

  // Multiplexer – calibration button, power measure, IR sensors
  pinMode(MUX_INT_OUT, INPUT);
  pinMode(MUX_INT_PL,  OUTPUT);
  pinMode(MUX_INT_CLK, OUTPUT);
#else
  // Multiplexer address pins
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);

  pinMode(MUX_EN_YOKE,     OUTPUT);
  pinMode(MUX_SIGNAL_YOKE, INPUT);

  pinMode(MUX_EN_INPUT,     OUTPUT);
  pinMode(MUX_SIGNAL_INPUT, INPUT);
#endif

  // Set all outputs to their safe / inactive default state
  digitalWrite(BUZZER_PIN, LOW);

  // Pitch motor
  digitalWrite(PITCH_EN,    LOW);
  digitalWrite(PITCH_U_PWM, LOW);
  digitalWrite(PITCH_D_PWM, LOW);

  // Roll motor
  digitalWrite(ROLL_EN,    LOW);
  digitalWrite(ROLL_R_PWM, LOW);
  digitalWrite(ROLL_L_PWM, LOW);

  // Multiplexer defaults
#ifdef ARDUINO_PRO_MICRO
  digitalWrite(MUX_YOKE_PL,  HIGH);
  digitalWrite(MUX_YOKE_CLK, LOW);
  digitalWrite(MUX_INT_PL,   HIGH);
  digitalWrite(MUX_INT_CLK,  LOW);
#else
  digitalWrite(MUX_S0,      LOW);
  digitalWrite(MUX_S1,      LOW);
  digitalWrite(MUX_S2,      LOW);
  digitalWrite(MUX_S3,      LOW);
  digitalWrite(MUX_EN_YOKE, HIGH);
  digitalWrite(MUX_EN_INPUT, HIGH);
#endif

  // Set PWM frequency to 31.25 kHz to eliminate audible motor whine.
  // Timer1: pins 9 & 10
  TCCR1B = _BV(CS10);

  // Timer4: pins 13 & 6
  TCCR4B = _BV(CS40);

  // Timer3: pin 5 (Pro Micro only)
#ifdef ARDUINO_PRO_MICRO
  TCCR3B = _BV(CS30);
#endif

}  // ArduinoSetup

/***************************
  Enable both motor drivers
****************************/
void EnableMotors() {
  digitalWrite(PITCH_EN, HIGH);
  digitalWrite(ROLL_EN,  HIGH);
}

/***************************
  Disable both motor drivers and coast to a stop
****************************/
void DisableMotors() {
  digitalWrite(PITCH_EN, LOW);
  digitalWrite(ROLL_EN,  LOW);

  analogWrite(ROLL_L_PWM, 0);
  analogWrite(ROLL_R_PWM, 0);
  roll_speed = 0;

  analogWrite(PITCH_U_PWM, 0);
  analogWrite(PITCH_D_PWM, 0);
  pitch_speed = 0;
}

/******************************************************
  Dispatch motor commands for both axes
******************************************************/
void PrepareMotors() {
  MoveMotorByForce(pitch_speed,
                   (mux.EndSwitchPitchDown() || mux.EndSwitchPitchUp()),
                   PITCH_U_PWM,
                   PITCH_D_PWM,
                   forces[MEM_PITCH],
                   adjForceMax[MEM_PITCH],
                   adjPwmMin[MEM_PITCH],
                   adjPwmMax[MEM_PITCH]);

  MoveMotorByForce(roll_speed,
                   (mux.EndSwitchRollLeft() || mux.EndSwitchRollRight()),
                   ROLL_L_PWM,
                   ROLL_R_PWM,
                   forces[MEM_ROLL],
                   adjForceMax[MEM_ROLL],
                   adjPwmMin[MEM_ROLL],
                   adjPwmMax[MEM_ROLL]);
}

/******************************************************
  Calculate PWM from FFB force and drive the motor.

  BUG FIX: removed the redundant `abs(pForce)` check –
           pForce is already non-negative (constrain of abs),
           so comparing abs(pForce) > 10 and pForce > 10 were
           equivalent but the former implied pForce could be negative.
           Using pForce directly is cleaner and correct.
******************************************************/
void MoveMotorByForce(byte    &rSpeed,
                      bool     blEndSwitch,
                      byte     pinLPWM,
                      byte     pinRPWM,
                      int16_t  gForce,
                      int      forceMax,
                      byte     pwmMin,
                      byte     pwmMax) {

  if (blEndSwitch) {
    // End-stop reached – kill power immediately
    analogWrite(pinLPWM, 0);
    analogWrite(pinRPWM, 0);
    rSpeed = 0;
    return;
  }

  // Clamp force magnitude to configured maximum
  int pForce = constrain(abs(gForce), 0, forceMax);

  // Apply a dead-band: ignore very small forces to prevent jitter
  if (pForce > 10) {
    rSpeed = map(pForce, 0, forceMax, pwmMin, pwmMax);
  } else {
    rSpeed = 0;
  }

  // Drive in the appropriate direction
  if (gForce > 0) {
    analogWrite(pinRPWM, 0);
    analogWrite(pinLPWM, rSpeed);
  } else {
    analogWrite(pinLPWM, 0);
    analogWrite(pinRPWM, rSpeed);
  }
}
