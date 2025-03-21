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

#include "defines.h"
#include "Multiplexer.h"


// Constructor to initialize the Joystick pointer and end switch state pointers
Multiplexer::Multiplexer(Joystick_* joystickPtr) {
    this->joystick = joystickPtr;

    #ifdef ARDUINO_PRO_MICRO
      mux_yoke.begin(MUX_YOKE_OUT, MUX_YOKE_PL, MUX_YOKE_CLK);
      mux_int.begin(MUX_INT_OUT, MUX_INT_PL, MUX_INT_CLK);
    #endif
}

// Method to read the multiplexer and update end switches
void Multiplexer::ReadMux() {
  #ifndef ARDUINO_PRO_MICRO
    iYokeButtonPinStates = 0;
    iSensorPinStates = 0;

    // for every 16 imput lines of a mutiplexer
    for (byte x = 0; x < 16; x++) {
      for (int i = 0; i < 4; i++) {
          PORTF = (x & (1 << i)) ? (PORTF | (1 << (7 - i))) : (PORTF & ~(1 << (7 - i)));
      }

      // enable mux 1
      //PORTC = PORTC & B10111111;  // Digital Pin 5 - PortC6
      PORTC &= ~B01000000; // Digital Pin 5 - PortC6

      // wait for capacitors of mux to react
      delayMicroseconds(1);

      // read value
      iYokeButtonPinStates |= digitalRead(MUX_SIGNAL_YOKE) << x;

      // disable mux1
      PORTC |= B00100000; // Digital Pin 5 - PortC6

      // enable mux 2
      PORTD &= ~B00010000; // Digital Pin 4 - PortD4

      // wait for capacitors of mux to react
      delayMicroseconds(1);

      // Read value from the second multiplexer
      iSensorPinStates |= digitalRead(MUX_SIGNAL_INPUT) << x;

      // disblae mux 2
      PORTD |= B00010000; // Digital Pin 4 - PortD4

    }  //for

    //Check end switches
    blEndSwitchRollLeft=(iSensorPinStates & (1 << ADJ_ENDSWITCH_ROLL_LEFT))==0;
    blEndSwitchRollRight=(iSensorPinStates & (1 << ADJ_ENDSWITCH_ROLL_RIGHT))==0;
    blEndSwitchPitchUp=(iSensorPinStates & (1 << ADJ_ENDSWITCH_PITCH_UP))==0;
    blEndSwitchPitchDown=(iSensorPinStates & (1 << ADJ_ENDSWITCH_PITCH_DOWN))==0;
    blCalibrationButtonPushed=(iSensorPinStates & (1 << ADJ_CALIBRATION_BUTTON))!=0;
    blMotorPower=(iSensorPinStates & (1 << ADJ_MOTOR_POWER))!=0;
  #else
    mux_int.update();
    blEndSwitchPitchDown=!mux_int.read(0);
    blEndSwitchPitchUp=!mux_int.read(1);
    blEndSwitchRollLeft=!mux_int.read(2);
    blEndSwitchRollRight=!mux_int.read(3);
    blCalibrationButtonPushed=mux_int.read(4);
    blMotorPower=mux_int.read(5);
  #endif  

  #ifdef SERIAL_DEBUG
    Serial.print("Calib.: ");
    Serial.print(blCalibrationButtonPushed);
    Serial.print(", Power: ");
    Serial.print(blMotorPower);
    Serial.print(", Down: ");
    Serial.print(blEndSwitchPitchDown); 
    Serial.print(", Up: ");
    Serial.print(blEndSwitchPitchUp); 
    Serial.print(", Left: ");
    Serial.print(blEndSwitchRollLeft); 
    Serial.print(", Right: ");
    Serial.print(blEndSwitchRollRight);     
  #endif  
}

bool Multiplexer::EndSwitchRollLeft(){
  return blEndSwitchRollLeft;
}

bool Multiplexer::EndSwitchRollRight(){
  return blEndSwitchRollRight;
}

bool Multiplexer::EndSwitchPitchUp(){
  return blEndSwitchPitchUp;
}

bool Multiplexer::EndSwitchPitchDown(){
  return blEndSwitchPitchDown;
}

bool Multiplexer::CalibrationButtonPushed(){
  return blCalibrationButtonPushed;
}

bool Multiplexer::MotorPower(){
  return blMotorPower;
}

uint16_t Multiplexer::GetYokeButtonPinStates(){
  return iYokeButtonPinStates;
}

uint16_t Multiplexer::GetSensorPinStates(){
  return iSensorPinStates;
}

// Method to update the joystick buttons based on multiplexer input
void Multiplexer::UpdateJoystickButtons() {
  bool data =0;

  #ifndef ARDUINO_PRO_MICRO
    #ifdef SERIAL_DEBUG
   
      for (byte channel = 4; channel < 16; channel++) {
          Serial.print(", Ch.");
          Serial.print(channel);
          Serial.print(": ");
          Serial.print((iYokeButtonPinStates >> channel) & 1);
      }
    #endif

    // Bit-Shift um 12 für Hat-Switch-Position
    uint16_t hatSwitchState = iYokeButtonPinStates << 12;

    // Setze die Hat-Switch-Position
    switch (hatSwitchState) {
      case 0B0000000000000000:
        joystick->setHatSwitch(0, -1); // no direction
        break;
      case 0B0100000000000000:
        joystick->setHatSwitch(0, 0); // up
        break;
      case 0B0101000000000000:
        joystick->setHatSwitch(0, 45); // up right
        break;
      case 0B0001000000000000:
        joystick->setHatSwitch(0, 90); // right
        break;
      case 0B0011000000000000:
        joystick->setHatSwitch(0, 135); // down right
        break;
      case 0B0010000000000000:
        joystick->setHatSwitch(0, 180); // down
        break;
      case 0B1010000000000000:
        joystick->setHatSwitch(0, 225); // down left
        break;
      case 0B1000000000000000:
        joystick->setHatSwitch(0, 270); // left
        break;
      case 0B1100000000000000:
        joystick->setHatSwitch(0, 315); // up left
        break;
      default:
        break; // no change
    }

    // Lese Button-Zustände vom Multiplexer
    for (byte channel = 4; channel < 16; channel++) {
      joystick->setButton(channel - 4, (iYokeButtonPinStates >> channel) & 1);
    }
  #else
  
    mux_yoke.update();

    #ifdef SERIAL_DEBUG
      Serial.print(", H. Up: ");
      Serial.print(!mux_yoke.read(0));
      Serial.print(", H. Dn: ");
      Serial.print(!mux_yoke.read(2));
      Serial.print(", H. Lf: ");
      Serial.print(!mux_yoke.read(3));
      Serial.print(", H. Rg: ");
      Serial.print(!mux_yoke.read(1));


      for(uint8_t i=4, n=mux_yoke.getLength(); i < n ; i++){
          data = !mux_yoke.read(i);
          Serial.print(", Pin ");
          Serial.print(i);
          Serial.print(": ");
          Serial.print(data);
      }
    #else

    byte hatSwitchState=0;
    hatSwitchState |= (!mux_yoke.read(0) << 0);
    hatSwitchState |= (!mux_yoke.read(1) << 1);
    hatSwitchState |= (!mux_yoke.read(2) << 2);
    hatSwitchState |= (!mux_yoke.read(3) << 3);

    switch (hatSwitchState) {
        case 0B00000000:
          joystick->setHatSwitch(0, -1); // no direction
          break;
        case 0B00000001:
          joystick->setHatSwitch(0, 0); // up
          break;
        case 0B00000011:
          joystick->setHatSwitch(0, 45); // up right
          break;
        case 0B00000010:
          joystick->setHatSwitch(0, 90); // right
          break;
        case 0B00000110:
          joystick->setHatSwitch(0, 135); // down right
          break;
        case 0B00000100:
          joystick->setHatSwitch(0, 180); // down
          break;
        case 0B00001100:
          joystick->setHatSwitch(0, 225); // down left
          break;
        case 0B00001000:
          joystick->setHatSwitch(0, 270); // left
          break;
        case 0B00001001:
          joystick->setHatSwitch(0, 315); // up left
          break;
        default:
          break; // no change
      }

      for (byte channel = 4; channel < 16; channel++) {
        joystick->setButton(channel - 4, !mux_yoke.read(channel));
      }
    #endif
  #endif
}
