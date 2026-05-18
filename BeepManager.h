/*
 Created by A.Eckers aka Gagagu – improved version
 http://www.gagagu.de / https://github.com/gagagu/Arduino_FFB_Yoke

 Beep-Code Übersicht
 ====================
 Achsenpräfix  (vor jedem Fehlercode, 3× wiederholt):
   Roll  = 2× 1400 Hz kurz  (hoch-hoch)
   Pitch = 2× 400  Hz kurz  (tief-tief)
 + 600 ms Stille, dann Fehlercode:
   Motor  invertiert   : aufsteigend  400 → 800 → 1400 Hz
   Encoder invertiert  : absteigend  1400 → 800 → 400  Hz
   Achse blockiert     : 5× schnell  1000 Hz
   Kalibrierung Timeout: 1× lang      600 Hz
   Kein Motorstrom     : 4× langsam   350 Hz  (kein Achsenpräfix)
*/

#ifndef BEEPMANAGER_H
#define BEEPMANAGER_H

#include <Arduino.h>

class BeepManager {
public:
  BeepManager(int pin);

  void SystemStart();
  void CalibrationStart();
  void CalibrationError();

  void CalibrationTimeoutMotor(bool isRoll);
  void CalibrationTimeoutGeneral(bool isRoll);
  void CalibrationMotorInverted(bool isRoll);
  void CalibrationEncoderInverted(bool isRoll);
  void NoMotorPower();

private:
  int buzzerPin;

  void ManualTone(uint16_t frequency, uint16_t durationMs);
  void Beep(uint16_t frequency, uint16_t durationMs);
  void AxisPrefix(bool isRoll);
};

#endif
