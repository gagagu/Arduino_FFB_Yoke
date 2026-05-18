/*
 Created by A.Eckers aka Gagagu – improved version
 http://www.gagagu.de / https://github.com/gagagu/Arduino_FFB_Yoke
*/

#include "defines.h"
#include "Multiplexer.h"

Multiplexer::Multiplexer(Joystick_* joystickPtr) {
  this->joystick = joystickPtr;

#ifdef ARDUINO_PRO_MICRO
  mux_yoke.begin(MUX_YOKE_OUT, MUX_YOKE_PL, MUX_YOKE_CLK);
  mux_int.begin(MUX_INT_OUT,   MUX_INT_PL,  MUX_INT_CLK);
#endif
}

// Read all multiplexer inputs and update cached state.
void Multiplexer::ReadMux() {

#ifndef ARDUINO_PRO_MICRO
  iYokeButtonPinStates = 0;
  iSensorPinStates     = 0;

  for (byte x = 0; x < 16; x++) {
    // Set the 4-bit address on PORTF (pins A0-A3)
    for (int i = 0; i < 4; i++) {
      PORTF = (x & (1 << i)) ? (PORTF | (1 << (7 - i))) : (PORTF & ~(1 << (7 - i)));
    }

    // Yoke-button mux
    PORTC &= ~B01000000;  // enable  (pin 5 / PortC6 LOW)
    delayMicroseconds(1);
    iYokeButtonPinStates |= (uint16_t)digitalRead(MUX_SIGNAL_YOKE) << x;
    PORTC |=  B00100000;  // disable (pin 5 / PortC6 HIGH)

    // Sensor mux
    PORTD &= ~B00010000;  // enable  (pin 4 / PortD4 LOW)
    delayMicroseconds(1);
    iSensorPinStates |= (uint16_t)digitalRead(MUX_SIGNAL_INPUT) << x;
    PORTD |=  B00010000;  // disable (pin 4 / PortD4 HIGH)
  }

  // Decode end switches and controls (active-low logic → invert)
  blEndSwitchRollLeft        = (iSensorPinStates & (1 << ADJ_ENDSWITCH_ROLL_LEFT))  == 0;
  blEndSwitchRollRight       = (iSensorPinStates & (1 << ADJ_ENDSWITCH_ROLL_RIGHT)) == 0;
  blEndSwitchPitchUp         = (iSensorPinStates & (1 << ADJ_ENDSWITCH_PITCH_UP))   == 0;
  blEndSwitchPitchDown       = (iSensorPinStates & (1 << ADJ_ENDSWITCH_PITCH_DOWN)) == 0;
  blCalibrationButtonPushed  = (iSensorPinStates & (1 << ADJ_CALIBRATION_BUTTON))   != 0;
  blMotorPower               = (iSensorPinStates & (1 << ADJ_MOTOR_POWER))          != 0;

#else  // ARDUINO_PRO_MICRO

  mux_int.update();
  blEndSwitchPitchDown      = !mux_int.read(0);
  blEndSwitchPitchUp        = !mux_int.read(1);
  blEndSwitchRollLeft       = !mux_int.read(2);
  blEndSwitchRollRight      = !mux_int.read(3);
  blCalibrationButtonPushed =  mux_int.read(4);
  blMotorPower              =  mux_int.read(5);

#endif

  // Level-3 debug: raw sensor/switch state on every mux read
  DBG3(F("[mux] calib="));  DBG3(blCalibrationButtonPushed);
  DBG3(F(" pwr="));         DBG3(blMotorPower);
  DBG3(F(" dn="));          DBG3(blEndSwitchPitchDown);
  DBG3(F(" up="));          DBG3(blEndSwitchPitchUp);
  DBG3(F(" lf="));          DBG3(blEndSwitchRollLeft);
  DBG3(F(" rg="));          DBG3LN(blEndSwitchRollRight);
}

// ── Getters ────────────────────────────────────────────────────────────────

bool     Multiplexer::EndSwitchRollLeft()       { return blEndSwitchRollLeft; }
bool     Multiplexer::EndSwitchRollRight()      { return blEndSwitchRollRight; }
bool     Multiplexer::EndSwitchPitchUp()        { return blEndSwitchPitchUp; }
bool     Multiplexer::EndSwitchPitchDown()      { return blEndSwitchPitchDown; }
bool     Multiplexer::CalibrationButtonPushed() { return blCalibrationButtonPushed; }
bool     Multiplexer::MotorPower()              { return blMotorPower; }
uint16_t Multiplexer::GetYokeButtonPinStates()  { return iYokeButtonPinStates; }
uint16_t Multiplexer::GetSensorPinStates()      { return iSensorPinStates; }

// Update joystick button and hat-switch state from the latest mux read.
//
// BUG FIX: on Pro Micro, the original code wrapped ALL button-setting
// logic inside #else of #ifdef SERIAL_DEBUG.  In debug mode, only
// Serial.print ran – buttons were never set.  Now debug output and
// button-setting are always both executed; the DBG3 macro compiles
// to nothing in non-debug builds, so there is no overhead.
void Multiplexer::UpdateJoystickButtons() {

#ifndef ARDUINO_PRO_MICRO

  // Debug: log raw channel states (level 3 only)
  for (byte ch = 4; ch < 16; ch++) {
    DBG3(F(", Ch.")); DBG3(ch);
    DBG3(F(":")); DBG3((iYokeButtonPinStates >> ch) & 1);
  }

  // Hat switch: bits 0-3 of the yoke button word
  uint16_t hatSwitchState = iYokeButtonPinStates << 12;
  switch (hatSwitchState) {
    case 0b0000000000000000: joystick->setHatSwitch(0,  -1); break;
    case 0b0100000000000000: joystick->setHatSwitch(0,   0); break;
    case 0b0101000000000000: joystick->setHatSwitch(0,  45); break;
    case 0b0001000000000000: joystick->setHatSwitch(0,  90); break;
    case 0b0011000000000000: joystick->setHatSwitch(0, 135); break;
    case 0b0010000000000000: joystick->setHatSwitch(0, 180); break;
    case 0b1010000000000000: joystick->setHatSwitch(0, 225); break;
    case 0b1000000000000000: joystick->setHatSwitch(0, 270); break;
    case 0b1100000000000000: joystick->setHatSwitch(0, 315); break;
    default: break;
  }

  for (byte ch = 4; ch < 16; ch++) {
    joystick->setButton(ch - 4, (iYokeButtonPinStates >> ch) & 1);
  }

#else  // ARDUINO_PRO_MICRO

  mux_yoke.update();

  // Debug: log hat-switch and button channels (level 3 only)
  DBG3(F(", H.Up="));  DBG3(!mux_yoke.read(0));
  DBG3(F(" H.Dn="));   DBG3(!mux_yoke.read(2));
  DBG3(F(" H.Lf="));   DBG3(!mux_yoke.read(3));
  DBG3(F(" H.Rg="));   DBG3LN(!mux_yoke.read(1));

  for (uint8_t i = 4, n = mux_yoke.getLength(); i < n; i++) {
    DBG3(F(", Pin")); DBG3(i); DBG3(F("=")); DBG3LN(!mux_yoke.read(i));
  }

  // Hat switch – always set (was missing in debug mode before)
  byte hatState = 0;
  hatState |= (!mux_yoke.read(0) << 0);  // up
  hatState |= (!mux_yoke.read(1) << 1);  // right
  hatState |= (!mux_yoke.read(2) << 2);  // down
  hatState |= (!mux_yoke.read(3) << 3);  // left

  switch (hatState) {
    case 0b00000000: joystick->setHatSwitch(0,  -1); break;
    case 0b00000001: joystick->setHatSwitch(0,   0); break;
    case 0b00000011: joystick->setHatSwitch(0,  45); break;
    case 0b00000010: joystick->setHatSwitch(0,  90); break;
    case 0b00000110: joystick->setHatSwitch(0, 135); break;
    case 0b00000100: joystick->setHatSwitch(0, 180); break;
    case 0b00001100: joystick->setHatSwitch(0, 225); break;
    case 0b00001000: joystick->setHatSwitch(0, 270); break;
    case 0b00001001: joystick->setHatSwitch(0, 315); break;
    default: break;
  }

  // Buttons – always set
  for (byte ch = 4; ch < 16; ch++) {
    joystick->setButton(ch - 4, !mux_yoke.read(ch));
  }

#endif
}
