#ifndef BEEPMANAGER_H
#define BEEPMANAGER_H

#include <Arduino.h>

class BeepManager {
  public:
    // Constructor
    BeepManager(int pin);

    // System Start
    void SystemStart();

    // Start Calibration
    void CalibrationStart();

    // Calibration Successful
    void CalibrationSuccess();

    // Calibration Error
    void CalibrationError();

    // Calibration Movement Timeout on Axis
    void CalibrationTimeoutMotor(bool isRoll);

    // General calibration Timeout
    void CalibrationTimeoutGeneral(bool isRoll);

    // Wrong motor direction
    void CalibrationMotorInverted(bool isRoll);

    // wrong encoder direction
    void CalibrationEncoderInverted(bool isRoll);

    // no motor power
    void NoMotorPower();
  private:
    int buzzerPin;

    // create tone
    void ManualTone(int frequency, int duration);

    // beep with drequency and duration
    void Beep(int duration, int frequency);

    // beep code for roll axis
    void CalibrationBeepRoll();

    // beep code for pitch axis
    void CalibrationBeepPitch();

    // switch between axis
    void CalibrationBeepAxis(bool isRoll);
};

#endif
