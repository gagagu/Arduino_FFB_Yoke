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
