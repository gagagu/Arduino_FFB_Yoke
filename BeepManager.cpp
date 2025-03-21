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

#include "BeepManager.h"
#include "defines.h"

// constructor
BeepManager::BeepManager(int pin) {
  buzzerPin = pin;
  pinMode(buzzerPin, OUTPUT);
}

// create tone
void BeepManager::ManualTone(int frequency, int duration) {
  long period = 1000000L / frequency;  // Berechne die Periode in Mikrosekunden
  long cycles = (long)duration * 1000L / period;  // Anzahl der Zyklen

  for (long i = 0; i < cycles; i++) {
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(period / 2);  // Halbe Periode HIGH
    digitalWrite(buzzerPin, LOW);
    delayMicroseconds(period / 2);  // Halbe Periode LOW
  }
}

// create a beep
void BeepManager::Beep(int duration, int frequency) {
  ManualTone(frequency, duration); // create beep
  delay(200);                      // delay
}

// beep code for pitch axis
void BeepManager::CalibrationBeepPitch() {
  Beep(BEEP_LONG_TONE, 1000);  
  Beep(BEEP_SHORT_TONE, 500);  
  delay(300);
}

// beep code for roll axis
void BeepManager::CalibrationBeepRoll() {
  Beep(BEEP_LONG_TONE, 500);  
  Beep(BEEP_SHORT_TONE, 1000);  
  delay(300);
}

// Roll or Pitch code?
void BeepManager::CalibrationBeepAxis(bool isRoll) {
  if(isRoll)
  {
    CalibrationBeepRoll();
  }else{
    CalibrationBeepPitch();
  }
}


// system start
void BeepManager::SystemStart() {
  Beep(200, 1000); 
  delay(1000);
}

// start calibration
void BeepManager::CalibrationStart() {
  Beep(250, 600);  
  delay(250);
  Beep(250, 600);  
  delay(250);
  Beep(250, 600);  

  delay(1000);
}

// calibration successful finished
void BeepManager::CalibrationSuccess() {
  Beep(100, 500);  
  delay(100);
  Beep(100, 600); 
  delay(100);
  Beep(300, 700); 

  delay(1000);
}

// Error on calibration
void BeepManager::CalibrationError() {
  Beep(50, 600);  
  delay(50);
  Beep(50, 600);  
  delay(50);
  Beep(50, 600);  
  delay(100);

  Beep(150, 600);  
  delay(100);
  Beep(150, 600);  
  delay(100);
  Beep(150, 600);  
  delay(100);

  Beep(50, 600);  
  delay(50);
  Beep(50, 600);  
  delay(50);
  Beep(50, 600);  

  delay(500);
}

// axis not moving timeout (short, short)
void BeepManager::CalibrationTimeoutMotor(bool isRoll) {
  for(byte x=0; x<=BEEP_CODE_COUNT;x++)
  {
    CalibrationBeepAxis(isRoll);
    Beep(BEEP_SHORT_TONE, BEEP_CODE_FREQUENCY);  
    Beep(BEEP_SHORT_TONE, BEEP_CODE_FREQUENCY);  
    delay(BEEP_CODE_DELAY);
  }
}

// timeout for calibration (short, long)
void BeepManager::CalibrationTimeoutGeneral(bool isRoll) {
  for(byte x=0; x<=BEEP_CODE_COUNT;x++)
  {
    CalibrationBeepAxis(isRoll);
    Beep(BEEP_SHORT_TONE, BEEP_CODE_FREQUENCY);  
    Beep(BEEP_LONG_TONE, BEEP_CODE_FREQUENCY);  
    delay(BEEP_CODE_DELAY);
  }
}

// motor goes in wrong direction (long, short)
void BeepManager::CalibrationMotorInverted(bool isRoll) {
  for(byte x=0; x<=BEEP_CODE_COUNT;x++)
  {
    CalibrationBeepAxis(isRoll);
    Beep(BEEP_LONG_TONE, BEEP_CODE_FREQUENCY);  
    Beep(BEEP_SHORT_TONE, BEEP_CODE_FREQUENCY);  
    delay(BEEP_CODE_DELAY);
  }
}

// encoder counts in wrong direction (long,long)
void BeepManager::CalibrationEncoderInverted(bool isRoll) {
  for(byte x=0; x<=BEEP_CODE_COUNT;x++)
  {
    CalibrationBeepAxis(isRoll);
    Beep(BEEP_LONG_TONE, BEEP_CODE_FREQUENCY);  
    Beep(BEEP_LONG_TONE, BEEP_CODE_FREQUENCY); 
    delay(BEEP_CODE_DELAY);
  }
}

void BeepManager::NoMotorPower() {
  for(byte x=0; x<=BEEP_CODE_COUNT;x++)
  {
    Beep(BEEP_LONG_TONE, BEEP_CODE_FREQUENCY);  
    Beep(BEEP_LONG_TONE, BEEP_CODE_FREQUENCY);  
    Beep(BEEP_LONG_TONE, BEEP_CODE_FREQUENCY);  
    Beep(BEEP_LONG_TONE, BEEP_CODE_FREQUENCY);  
    delay(BEEP_CODE_DELAY);
  }
}
