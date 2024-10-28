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
  Beep(500, 600);  
  delay(500);
  Beep(500, 600);  
  delay(500);
  Beep(500, 600);  

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
  Beep(100, 600);  
  delay(100);
  Beep(100, 600);  
  delay(100);
  Beep(100, 600);  
  delay(200);

  Beep(300, 600);  
  delay(200);
  Beep(300, 600);  
  delay(200);
  Beep(300, 600);  
  delay(200);

  Beep(100, 600);  
  delay(100);
  Beep(100, 600);  
  delay(100);
  Beep(100, 600);  

  delay(1000);
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
