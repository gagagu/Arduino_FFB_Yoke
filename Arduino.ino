/* 
 Created by A.Eckers aka Gagagu
 http://www.gagagu.de
 https://github.com/gagagu/Arduino_FFB_Yoke
 https://www.youtube.com/@gagagu01
*/

/*
  This repository contains code for Arduino projects. 
  The code is provided "as is," without warranty of any kind, either express or implied, 
  including but not limited to the warranties of merchantability, 
  fitness for a particular purpose, or non-infringement. 
  The author(s) make no representations or warranties about the accuracy or completeness of 
  the code or its suitability for your specific use case.

  By using this code, you acknowledge and agree that you are solely responsible for any 
  consequences that may arise from its use. 

  For DIY projects involving electronic and electromechanical moving parts, caution is essential. 
  Ensure that you take the appropriate safety precautions, particularly when working with electricity. 
  Only work with devices if you understand their functionality and potential risks, and always wear 
  appropriate protective equipment. 
  Make sure you are working in a safe, well-lit environment, and that all components are properly installed and secured to avoid injury or damage.

  Special caution is required when building a force feedback device. Unexpected or sudden movements may occur, 
  which could lead to damage to people or other objects. 
  Ensure that all mechanical parts are securely mounted and that the work area is free of obstacles.
  
  By using this project, you acknowledge and agree that you are solely responsible for any consequences that may arise from its use. 
  The author(s) will not be held liable for any damages, injuries, or issues arising from the use of the project, 
  including but not limited to malfunctioning hardware, electrical damage, personal injury, or damage caused by 
  unintended movements of the force feedback device. The responsibility for proper handling, installation, 
  and use of the devices and components lies with the user.
  
  Use at your own risk.
*/

/***************
  Pin setup
****************/
void ArduinoSetup() {

  // Pitch motor driver Pins
  pinMode(PITCH_EN, OUTPUT);
  pinMode(PITCH_U_PWM, OUTPUT);
  pinMode(PITCH_D_PWM, OUTPUT);

  // Roll motor driver  Pins
  pinMode(ROLL_EN, OUTPUT);
  pinMode(ROLL_R_PWM, OUTPUT);
  pinMode(ROLL_L_PWM, OUTPUT);

  // Buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);

#ifdef ARDUINO_PRO_MICRO  
  // Multiplexer Yoke Buttons
  pinMode(MUX_YOKE_OUT, INPUT);
  pinMode(MUX_YOKE_PL, OUTPUT);
  pinMode(MUX_YOKE_CLK, OUTPUT);

  // Multiplexer Calibration Button, Power Measure, IR Sensors
  pinMode(MUX_INT_OUT, INPUT);
  pinMode(MUX_INT_PL, OUTPUT);
  pinMode(MUX_INT_CLK, OUTPUT);

#else
  // Buttons Pins (Multiplexer)
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);

  pinMode(MUX_EN_YOKE, OUTPUT);
  pinMode(MUX_SIGNAL_YOKE, INPUT);

  pinMode(MUX_EN_INPUT, OUTPUT);
  pinMode(MUX_SIGNAL_INPUT, INPUT);
#endif

// define pin default states
digitalWrite(BUZZER_PIN,LOW);

// Pitch
digitalWrite(PITCH_EN, LOW);
digitalWrite(PITCH_U_PWM, LOW);
digitalWrite(PITCH_D_PWM, LOW);
//Roll
digitalWrite(ROLL_EN, LOW);
digitalWrite(ROLL_R_PWM, LOW);
digitalWrite(ROLL_L_PWM, LOW);

  // Multiplexer
#ifdef ARDUINO_PRO_MICRO  
  digitalWrite(MUX_YOKE_PL, HIGH);
  digitalWrite(MUX_YOKE_CLK, LOW);
  digitalWrite(MUX_INT_PL, HIGH);
  digitalWrite(MUX_INT_CLK, LOW);
#else
  digitalWrite(MUX_S0, LOW);
  digitalWrite(MUX_S1, LOW);
  digitalWrite(MUX_S2, LOW);
  digitalWrite(MUX_S3, LOW);
  digitalWrite(MUX_EN_YOKE, HIGH);
  digitalWrite(MUX_EN_INPUT, HIGH);
#endif

  // not for all Arduinos!
  // This sets the PWM Speed to maximun for noise reduction

  // Timer1: pins 9 & 10
  TCCR1B = _BV(CS10);  // change the PWM frequencey to 31.25kHz - pins 9 & 10

  // Timer4: pin 13 & 6
  TCCR4B = _BV(CS40);  // change the PWM frequencey to 31.25kHz - pin 13 & 6

  //Timer3: pin 5
 #ifdef ARDUINO_PRO_MICRO  
   TCCR3B = _BV(CS30);  // Change the PWM frequency to 31.25kHz - pin 5
 #endif
 
}  //ArduinoSetup

/**************************
  Enables the motordrivers
****************************/
void EnableMotors() {
  digitalWrite(PITCH_EN, HIGH);
  digitalWrite(ROLL_EN, HIGH);
}  //EnableMotors

/***************************
  Disables the motordrivers
****************************/
void DisableMotors() {
  digitalWrite(PITCH_EN, LOW);
  digitalWrite(ROLL_EN, LOW);

  analogWrite(ROLL_L_PWM, 0);  // stop left
  analogWrite(ROLL_R_PWM, 0);  // stop right
  roll_speed = 0;              // speed to 0

  analogWrite(PITCH_U_PWM, 0);  // stop left
  analogWrite(PITCH_D_PWM, 0);  // stop right
  pitch_speed = 0;              // speed to 0
}  //DisableMotors

/******************************************************
  Prepare motors for movement
******************************************************/
void PrepareMotors() {

  
  // prepare motor for pitch direction
  MoveMotorByForce(pitch_speed,
                   (mux.EndSwitchPitchDown() || mux.EndSwitchPitchUp()),
                   PITCH_U_PWM,
                   PITCH_D_PWM,
                   forces[MEM_PITCH],
                   adjForceMax[MEM_PITCH],
                   adjPwmMin[MEM_PITCH],
                   adjPwmMax[MEM_PITCH]);

  // prepare motor for roll direction
  MoveMotorByForce(roll_speed,
                   (mux.EndSwitchRollLeft() || mux.EndSwitchRollRight()),
                   ROLL_L_PWM, 
                   ROLL_R_PWM, 
                   forces[MEM_ROLL],
                   adjForceMax[MEM_ROLL],
                   adjPwmMin[MEM_ROLL],
                   adjPwmMax[MEM_ROLL]);

}  //PrepareMotors

/******************************************************
  calculates the motor speeds and move
******************************************************/
void MoveMotorByForce(byte &rSpeed,
                      bool blEndSwitch,
                      byte pinLPWM,
                      byte pinRPWM,
                      int16_t gForce,
                      int forceMax,
                      byte pwmMin,
                      byte pwmMax) {

  // if position is on end switch then stop the motor
  if (blEndSwitch) {
    analogWrite(pinLPWM, 0);  // stop left
    analogWrite(pinRPWM, 0);  // stop right
    rSpeed = 0;               // speed to 0
  } else {
    // cut force to maximum value
    int pForce = constrain(abs(gForce), 0, forceMax);
    // calculate motor speed (pwm) by force between min pwm and max pwm speed
    if(abs(pForce) > 10)
      rSpeed = map(pForce, 0, forceMax, pwmMin, pwmMax);
    else
      rSpeed=0;

   // which direction?
    if (gForce > 0) {
      analogWrite(pinRPWM, 0);       // stop right
      analogWrite(pinLPWM, rSpeed);  // speed up left
    } else {
      analogWrite(pinLPWM, 0);       // stop left
      analogWrite(pinRPWM, rSpeed);  // speed up right
    }
  }
}  //MoveMotor
