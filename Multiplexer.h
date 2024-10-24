#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include <Arduino.h>
#include "src/Joystick.h"   // Include the Joystick class header file

class Multiplexer {
private:
    Joystick_* joystick;  // Pointer to a Joystick object
    // variables for button pin states
    uint16_t iYokeButtonPinStates = 0;
    uint16_t iSensorPinStates = 0;

    // Pointers to end switch states
    bool blEndSwitchRollLeft;    
    bool blEndSwitchRollRight;   
    bool blEndSwitchPitchUp;     
    bool blEndSwitchPitchDown;   
    bool blCalubrationButtonPushed;
    bool blMotorPower;
public:
    
    // Constructor: pass a pointer to a Joystick object
    //Multiplexer(Joystick_* joystickPtr, bool* rollLeft, bool* rollRight, bool* pitchUp, bool* pitchDown);
    Multiplexer(Joystick_* joystickPtr);

    // Method to read from the multiplexer and update end switches
    void ReadMux();

    // Method to update the joystick buttons based on multiplexer state
    void UpdateJoystickButtons();

    bool EndSwitchRollLeft();
    bool EndSwitchRollRight();   
    bool EndSwitchPitchUp();     
    bool EndSwitchPitchDown();   
    bool CalibrationButtonPushed(); 
    bool MotorPower();
    uint16_t GetYokeButtonPinStates();
    uint16_t GetSensorPinStates();
};

#endif
