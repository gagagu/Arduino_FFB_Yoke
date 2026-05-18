/*
 Created by A.Eckers aka Gagagu – improved version
 http://www.gagagu.de / https://github.com/gagagu/Arduino_FFB_Yoke
*/

#include "BeepManager.h"
#include "defines.h"

BeepManager::BeepManager(int pin) {
  buzzerPin = pin;
  pinMode(buzzerPin, OUTPUT);
}

// Square-wave tone. BUG FIX: frequency=0 guard (was division-by-zero).
void BeepManager::ManualTone(uint16_t frequency, uint16_t durationMs) {
  if (frequency == 0) return;
  uint32_t period = 1000000UL / frequency;
  uint32_t cycles = (uint32_t)durationMs * 1000UL / period;
  for (uint32_t i = 0; i < cycles; i++) {
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(period / 2);
    digitalWrite(buzzerPin, LOW);
    delayMicroseconds(period / 2);
  }
}

// BUG FIX: parameter order (frequency, duration) – was swapped in original.
void BeepManager::Beep(uint16_t frequency, uint16_t durationMs) {
  ManualTone(frequency, durationMs);
  delay(120);
}

// Axis prefix – 2 short beeps at an axis-specific frequency + 600 ms silence.
//   Roll  = 1400 Hz  (high)
//   Pitch =  400 Hz  (low)
void BeepManager::AxisPrefix(bool isRoll) {
  uint16_t f = isRoll ? 1400 : 400;
  Beep(f, 120);
  Beep(f, 120);
  delay(600);
}

// ── System sounds ─────────────────────────────────────────────────────────────

void BeepManager::SystemStart() {
  Beep(1000, 200);
  delay(800);
}

void BeepManager::CalibrationStart() {
  Beep(800, 180);
  Beep(800, 180);
  Beep(800, 180);
  delay(800);
}

// 3 short beeps – simpler than SOS, same flash footprint as original.
void BeepManager::CalibrationError() {
  Beep(600, 80);
  Beep(600, 80);
  Beep(600, 80);
  delay(400);
}

// ── Error codes ───────────────────────────────────────────────────────────────
// Each code = axis prefix + 2-tone pattern, repeated BEEP_CODE_COUNT times.
// 2-tone patterns keep frequency differentiation without 3-ManualTone sequences:
//
//   Motor inverted  : LOW then HIGH  (400 → 1400 Hz)  "swap motor wires"
//   Encoder inverted: HIGH then LOW  (1400 → 400 Hz)  "swap encoder wires"
//   Axis stuck      : 5× rapid mid   (1000 Hz)        "motor is trying"
//   Timeout general : 1× long low    (600 Hz)         "time ran out"
//   No power        : 4× slow bass   (350 Hz)         "power dead"

void BeepManager::CalibrationMotorInverted(bool isRoll) {
  for (byte i = 0; i < BEEP_CODE_COUNT; i++) {
    AxisPrefix(isRoll);
    ManualTone(400,  300); delay(80);
    ManualTone(1400, 400);
    delay(BEEP_CODE_DELAY);
  }
}

void BeepManager::CalibrationEncoderInverted(bool isRoll) {
  for (byte i = 0; i < BEEP_CODE_COUNT; i++) {
    AxisPrefix(isRoll);
    ManualTone(1400, 300); delay(80);
    ManualTone(400,  400);
    delay(BEEP_CODE_DELAY);
  }
}

void BeepManager::CalibrationTimeoutMotor(bool isRoll) {
  for (byte i = 0; i < BEEP_CODE_COUNT; i++) {
    AxisPrefix(isRoll);
    for (byte j = 0; j < 5; j++) { ManualTone(1000, 80); delay(55); }
    delay(BEEP_CODE_DELAY);
  }
}

void BeepManager::CalibrationTimeoutGeneral(bool isRoll) {
  for (byte i = 0; i < BEEP_CODE_COUNT; i++) {
    AxisPrefix(isRoll);
    ManualTone(600, 1200);
    delay(BEEP_CODE_DELAY);
  }
}

void BeepManager::NoMotorPower() {
  for (byte rep = 0; rep < BEEP_CODE_COUNT; rep++) {
    for (byte i = 0; i < 4; i++) { ManualTone(350, 350); delay(200); }
    delay(BEEP_CODE_DELAY);
  }
}
