/*
 Created by A.Eckers aka Gagagu – improved version
 http://www.gagagu.de / https://github.com/gagagu/Arduino_FFB_Yoke
*/

#include "AxisCalibration.h"

Axis::Axis(int motorLeftPin, int motorRightPin, bool isRoll,
           Encoder* encoderPtr, Multiplexer* multiplexerPtr, BeepManager* beepManagerPtr)
    : motorPinLeft(motorLeftPin), motorPinRight(motorRightPin),
      blIsRoll(isRoll), encoder(encoderPtr),
      speed(1), lastMovementTime(millis()),
      multiplexer(multiplexerPtr), beepManager(beepManagerPtr)
{}

void Axis::MoveMotor(bool direction) {
  if (direction) {
    analogWrite(motorPinLeft,  0);
    analogWrite(motorPinRight, speed);
  } else {
    analogWrite(motorPinLeft,  speed);
    analogWrite(motorPinRight, 0);
  }
}

void Axis::StopMotor() {
  analogWrite(motorPinLeft,  0);
  analogWrite(motorPinRight, 0);
  delay(waitDelayMotorStops);
}

void Axis::ReadMultiplexer() {
  multiplexer->ReadMux();
  if (blIsRoll) {
    blEndSwitchLeft  = multiplexer->EndSwitchRollLeft();
    blEndSwitchRight = multiplexer->EndSwitchRollRight();
  } else {
    blEndSwitchLeft  = multiplexer->EndSwitchPitchUp();
    blEndSwitchRight = multiplexer->EndSwitchPitchDown();
  }
}

int Axis::ResetEncoder() {
  encoder->write(0);
  return 0;
}

// BUG FIX 4: renamed parameter to avoid shadowing this->lastMovementTime
bool Axis::CheckTimeouts(unsigned long lastMoveTime, unsigned long calibStartTime) {
  if (millis() - lastMoveTime >= timeout) {
    StopMotor();
    config.blError       = true;
    config.blAxisTimeout = true;
    DBG2(F("[calib] TIMEOUT – axis not moving\n"));
    return true;
  }
  if (millis() - calibStartTime >= calibrationTimeout) {
    StopMotor();
    config.blError   = true;
    config.blTimeout = true;
    DBG2(F("[calib] TIMEOUT – overall calibration limit\n"));
    return true;
  }
  return false;
}

void Axis::ManageMovement(bool direction, unsigned long &lastMoveTime,
                          int &lastEncoderValue, bool &speedIncreased) {
  ReadMultiplexer();
  int current = encoder->read();

  if (abs(current - lastEncoderValue) <= 20) {
    if (speed < maxSpeed) speed++;
  } else {
    lastMoveTime = millis();
    if (!speedIncreased) {
      speed += speedIncrement;
      if (speed > maxSpeed) speed = maxSpeed;
      speedIncreased = true;
    }
  }
  lastEncoderValue = current;
  MoveMotor(direction);
}

// ── Helper: print calibration step header via DBG2 ──────────────────────────
#if defined(DBG_LEVEL) && DBG_LEVEL >= 2
static void dbgStep(byte step, const __FlashStringHelper* label) {
  DBG2(F("[calib] step "));
  DBG2(step);
  DBG2(F(": "));
  DBG2LN(label);
}
#else
static void dbgStep(byte, const __FlashStringHelper*) {}
#endif

/***********************************************************************
  Full calibration sequence

  Step 1 – escape any end switch the axis starts on
  Step 2 – drive to the right end switch
  Step 3 – drive to the left end switch; measure full travel
  Step 4 – return to centre (encoder = 0)
***********************************************************************/
void Axis::Calibrate() {
  config               = {false, 0, 0, false, false, false, false};
  calibrationStartTime = millis();

  int  lastEncoderValue = ResetEncoder();
  bool speedIncreased   = false;

  speed = 1;
  ReadMultiplexer();
  StopMotor();

  DBG2(F("[calib] start – axis: "));
  DBG2LN(blIsRoll ? F("ROLL") : F("PITCH"));

  //--------------------------------------------------------------------
  // Step 1: escape from end switch
  // BUG FIX 1: choose escape direction based on which switch is active
  //--------------------------------------------------------------------
  dbgStep(1, F("escape end switch (if needed)"));

  if (blEndSwitchLeft || blEndSwitchRight) {
    if (blEndSwitchLeft && blEndSwitchRight) {
      StopMotor();
      config.blError = true;
      DBG2(F("[calib] ERROR: both end switches active simultaneously\n"));
      return;
    }

    bool escapeDir = blEndSwitchLeft;  // left active → move right (true), and vice versa
    DBG2(F("[calib] on end switch, escaping dir=")); DBG2LN(escapeDir);

    unsigned long lastMoveTime = millis();
    while (blEndSwitchLeft || blEndSwitchRight) {
      ManageMovement(escapeDir, lastMoveTime, lastEncoderValue, speedIncreased);
      DBG2(F("  enc=")); DBG2(encoder->read()); DBG2(F(" spd=")); DBG2LN(speed);
      // Overall calibration timeout still catches stuck hardware here
      if (millis() - calibrationStartTime >= calibrationTimeout) break;
      delay(whileDelay);
    }
    StopMotor();
    delay(waitDelayAfterMoveOutEndstop);
  }
  if (config.blError) return;

  //--------------------------------------------------------------------
  // Step 2: drive to right end switch
  //--------------------------------------------------------------------
  dbgStep(2, F("drive to right end switch"));

  bool direction = true;
  speed          = 1;
  speedIncreased = false;
  unsigned long lastMoveTime = millis();
  lastEncoderValue = ResetEncoder();

  while (!blEndSwitchLeft && !blEndSwitchRight) {
    ManageMovement(direction, lastMoveTime, lastEncoderValue, speedIncreased);
    DBG2(F("  enc=")); DBG2(encoder->read()); DBG2(F(" spd=")); DBG2LN(speed);
    if (CheckTimeouts(lastMoveTime, calibrationStartTime)) break;
    delay(whileDelay);
  }
  StopMotor();

  if (blEndSwitchLeft) {
    config.blMotorInverted = true;
    config.blError         = true;
    DBG2(F("[calib] ERROR: motor inverted (left switch hit in step 2)\n"));
  }
  if (config.blError) return;

  DBG2(F("[calib] right end switch reached, enc=")); DBG2LN(encoder->read());

  //--------------------------------------------------------------------
  // Step 3: drive to left end switch; measure full travel.
  //
  // NOTE: at loop entry blEndSwitchRight is still TRUE – the motor just
  // stopped on the right switch at the end of Step 2.  Including it in
  // the while-condition would exit the loop immediately and falsely flag
  // an encoder-inversion error.  We therefore loop only on !blEndSwitchLeft
  // (as the original code did) and check blEndSwitchRight AFTER the loop:
  // if we ended up back on it without ever reaching the left switch, the
  // motor drove the wrong way → encoder polarity is inverted.
  //--------------------------------------------------------------------
  dbgStep(3, F("drive to left end switch"));

  direction      = !direction;
  speed          = 1;
  speedIncreased = false;
  lastMoveTime   = millis();
  lastEncoderValue = ResetEncoder();

  delay(1000);

  while (!blEndSwitchLeft) {
    ManageMovement(direction, lastMoveTime, lastEncoderValue, speedIncreased);
    DBG2(F("  enc=")); DBG2(encoder->read()); DBG2(F(" spd=")); DBG2LN(speed);
    if (CheckTimeouts(lastMoveTime, calibrationStartTime)) break;
    delay(whileDelay);
  }
  StopMotor();

  // Ended on the right switch → motor went the wrong direction → encoder inverted
  if (!blEndSwitchLeft && blEndSwitchRight) {
    config.blEncoderInverted = true;
    config.blError           = true;
    DBG2(F("[calib] ERROR: encoder inverted (right switch hit in step 3)\n"));
  }
  if (!config.blError && encoder->read() < 0) {
    config.blEncoderInverted = true;
    config.blError           = true;
    DBG2(F("[calib] ERROR: encoder inverted (negative value at left switch)\n"));
  }
  if (config.blError) return;

  // Symmetric axis range – rounded to avoid 1-count asymmetry on odd totals
  int32_t totalTravel = encoder->read();
  config.iMax = (int16_t)((totalTravel + 1) / 2);
  config.iMin = -config.iMax;
  encoder->write(config.iMax);

  DBG2(F("[calib] travel=")); DBG2(totalTravel);
  DBG2(F(" iMin="));          DBG2(config.iMin);
  DBG2(F(" iMax="));          DBG2LN(config.iMax);

  //--------------------------------------------------------------------
  // Step 4: return to centre
  // BUG FIX 3: reset speedIncreased so the speed ramp works normally
  //--------------------------------------------------------------------
  dbgStep(4, F("return to centre"));

  direction      = !direction;
  speed          = 1;
  speedIncreased = false;  // ← the critical reset
  lastMoveTime   = millis();
  lastEncoderValue = config.iMax;

  delay(1000);

  bool startedPositive = (lastEncoderValue > 0);
  while (((startedPositive  && encoder->read() >= 0) ||
          (!startedPositive && encoder->read() <= 0)) &&
         !blEndSwitchRight) {
    ManageMovement(direction, lastMoveTime, lastEncoderValue, speedIncreased);
    DBG2(F("  enc=")); DBG2(encoder->read()); DBG2(F(" spd=")); DBG2LN(speed);
    if (CheckTimeouts(lastMoveTime, calibrationStartTime)) return;
    delay(whileDelay);
  }
  StopMotor();

  DBG2(F("[calib] done, centre enc=")); DBG2LN(encoder->read());
}

AxisConfiguration Axis::GetConfiguration() {
  return config;
}

bool Axis::CheckError(bool isRoll) {
  if (!config.blError) return false;

  beepManager->CalibrationError();
  delay(BEEP_CODE_DELAY);

  if (config.blEncoderInverted) beepManager->CalibrationEncoderInverted(isRoll);
  if (config.blMotorInverted)   beepManager->CalibrationMotorInverted(isRoll);
  if (config.blAxisTimeout)     beepManager->CalibrationTimeoutMotor(isRoll);
  if (config.blTimeout)         beepManager->CalibrationTimeoutGeneral(isRoll);

  return true;
}
