/*
  Joystick.cpp – improved version
  Original: Copyright (c) 2015-2017 Matthew Heironimus (LGPL 2.1)
  FFB extensions: jmriego/Fino and contributors
*/

#include "Joystick.h"
#include "FFBDescriptor.h"   // BUG FIX: was included twice in the original
#include "filters.h"
#include "debug.h"

#if defined(_USING_DYNAMIC_HID)

#define JOYSTICK_REPORT_ID_INDEX     7
#define JOYSTICK_AXIS_MINIMUM    -32767
#define JOYSTICK_AXIS_MAXIMUM     32767
#define JOYSTICK_SIMULATOR_MINIMUM -32767
#define JOYSTICK_SIMULATOR_MAXIMUM  32767

#define JOYSTICK_INCLUDE_X_AXIS  B00000001
#define JOYSTICK_INCLUDE_Y_AXIS  B00000010
#define JOYSTICK_INCLUDE_Z_AXIS  B00000100
#define JOYSTICK_INCLUDE_RX_AXIS B00001000
#define JOYSTICK_INCLUDE_RY_AXIS B00010000
#define JOYSTICK_INCLUDE_RZ_AXIS B00100000

#define JOYSTICK_INCLUDE_RUDDER      B00000001
#define JOYSTICK_INCLUDE_THROTTLE    B00000010
#define JOYSTICK_INCLUDE_ACCELERATOR B00000100

// BUG FIX: descriptor buffer enlarged to 180 bytes to prevent silent overflow
// if more axes or buttons are enabled than the original 150-byte estimate.
#define HID_DESC_BUFFER_SIZE 180

const float cutoff_freq_damper   = 2.0f;
const float sampling_time_damper = 0.002f;
LowPassFilter damperFilter[FFB_AXIS_COUNT];
LowPassFilter inertiaFilter[FFB_AXIS_COUNT];
LowPassFilter frictionFilter[FFB_AXIS_COUNT];

Joystick_::Joystick_(
    uint8_t hidReportId,
    uint8_t joystickType,
    uint8_t buttonCount,
    uint8_t hatSwitchCount,
    bool includeXAxis,
    bool includeYAxis,
    bool includeZAxis,
    bool includeRxAxis,
    bool includeRyAxis,
    bool includeRzAxis,
    bool includeRudder,
    bool includeThrottle)
{
    _hidReportId = hidReportId;
    _buttonCount = buttonCount;
    _hatSwitchCount = hatSwitchCount;

    _includeAxisFlags = 0;
    _includeAxisFlags |= (includeXAxis  ? JOYSTICK_INCLUDE_X_AXIS  : 0);
    _includeAxisFlags |= (includeYAxis  ? JOYSTICK_INCLUDE_Y_AXIS  : 0);
    _includeAxisFlags |= (includeZAxis  ? JOYSTICK_INCLUDE_Z_AXIS  : 0);
    _includeAxisFlags |= (includeRxAxis ? JOYSTICK_INCLUDE_RX_AXIS : 0);
    _includeAxisFlags |= (includeRyAxis ? JOYSTICK_INCLUDE_RY_AXIS : 0);
    _includeAxisFlags |= (includeRzAxis ? JOYSTICK_INCLUDE_RZ_AXIS : 0);

    _includeSimulatorFlags = 0;
    _includeSimulatorFlags |= (includeRudder   ? JOYSTICK_INCLUDE_RUDDER   : 0);
    _includeSimulatorFlags |= (includeThrottle ? JOYSTICK_INCLUDE_THROTTLE : 0);

    uint8_t buttonsInLastByte = _buttonCount % 8;
    uint8_t buttonPaddingBits = (buttonsInLastByte > 0) ? (8 - buttonsInLastByte) : 0;

    uint8_t axisCount =
          (includeXAxis  ? 1 : 0) + (includeYAxis  ? 1 : 0)
        + (includeZAxis  ? 1 : 0) + (includeRxAxis ? 1 : 0)
        + (includeRyAxis ? 1 : 0) + (includeRzAxis ? 1 : 0);

    uint8_t simulationCount = (includeRudder ? 1 : 0) + (includeThrottle ? 1 : 0);

    static uint8_t tempHidReportDescriptor[HID_DESC_BUFFER_SIZE];
    int hidReportDescriptorSize = 0;

    // USAGE_PAGE (Generic Desktop)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
    // USAGE (Joystick / Gamepad / Multi-axis)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
    tempHidReportDescriptor[hidReportDescriptorSize++] = joystickType;
    // COLLECTION (Application)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0xa1;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
    // USAGE (Pointer)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
    // REPORT_ID
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x85;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    if (_buttonCount > 0) {
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05; // USAGE_PAGE (Button)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x19; // USAGE_MINIMUM (1)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x29; // USAGE_MAXIMUM
        tempHidReportDescriptor[hidReportDescriptorSize++] = _buttonCount;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15; // LOGICAL_MINIMUM (0)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25; // LOGICAL_MAXIMUM (1)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75; // REPORT_SIZE (1)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95; // REPORT_COUNT
        tempHidReportDescriptor[hidReportDescriptorSize++] = _buttonCount;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x55; // UNIT_EXPONENT (0)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x65; // UNIT (None)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81; // INPUT (Data,Var,Abs)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;

        if (buttonPaddingBits > 0) {
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75; // REPORT_SIZE (1)
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95; // REPORT_COUNT (padding)
            tempHidReportDescriptor[hidReportDescriptorSize++] = buttonPaddingBits;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81; // INPUT (Const,Var,Abs)
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x03;
        }
    }

    if ((axisCount > 0) || (_hatSwitchCount > 0)) {
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05; // USAGE_PAGE (Generic Desktop)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
    }

    if (_hatSwitchCount > 0) {
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; // USAGE (Hat Switch)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x39;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15; // LOGICAL_MINIMUM (0)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25; // LOGICAL_MAXIMUM (7)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x07;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x35; // PHYSICAL_MINIMUM (0)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x46; // PHYSICAL_MAXIMUM (315)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x3B;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x65; // UNIT (Eng Rot:Angular Pos)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x14;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75; // REPORT_SIZE (4)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95; // REPORT_COUNT (1)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81; // INPUT (Data,Var,Abs)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;

        if (_hatSwitchCount > 1) {
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; // USAGE (Hat Switch 2)
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x39;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x07;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x35;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x46;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x3B;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x65;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x14;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
        } else {
            // Padding for single hat switch
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
            tempHidReportDescriptor[hidReportDescriptorSize++] = 0x03;
        }
    }

    if (axisCount > 0) {
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; // USAGE (Pointer)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x16; // LOGICAL_MINIMUM (-32767)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x80;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x26; // LOGICAL_MAXIMUM (+32767)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x7F;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75; // REPORT_SIZE (16)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x10;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95; // REPORT_COUNT
        tempHidReportDescriptor[hidReportDescriptorSize++] = axisCount;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0xA1; // COLLECTION (Physical)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
        if (includeXAxis)  { tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; tempHidReportDescriptor[hidReportDescriptorSize++] = 0x30; }
        if (includeYAxis)  { tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; tempHidReportDescriptor[hidReportDescriptorSize++] = 0x31; }
        if (includeZAxis)  { tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; tempHidReportDescriptor[hidReportDescriptorSize++] = 0x32; }
        if (includeRxAxis) { tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; tempHidReportDescriptor[hidReportDescriptorSize++] = 0x33; }
        if (includeRyAxis) { tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; tempHidReportDescriptor[hidReportDescriptorSize++] = 0x34; }
        if (includeRzAxis) { tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; tempHidReportDescriptor[hidReportDescriptorSize++] = 0x35; }
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81; // INPUT (Data,Var,Abs)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0; // END_COLLECTION
    }

    if (simulationCount > 0) {
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05; // USAGE_PAGE (Simulation Controls)
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x16;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x80;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x26;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x7F;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x10;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
        tempHidReportDescriptor[hidReportDescriptorSize++] = simulationCount;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0xA1;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
        if (includeRudder)   { tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; tempHidReportDescriptor[hidReportDescriptorSize++] = 0xBA; }
        if (includeThrottle) { tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09; tempHidReportDescriptor[hidReportDescriptorSize++] = 0xBB; }
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
        tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;
    }

    uint8_t* customHidReportDescriptor = new uint8_t[hidReportDescriptorSize];
    memcpy(customHidReportDescriptor, tempHidReportDescriptor, hidReportDescriptorSize);
    DynamicHIDSubDescriptor* node = new DynamicHIDSubDescriptor(
        customHidReportDescriptor, hidReportDescriptorSize,
        pidReportDescriptor, pidReportDescriptorSize, false);
    DynamicHID().AppendDescriptor(node);

    if (buttonCount > 0) {
        _buttonValuesArraySize = (_buttonCount + 7) / 8;
        _buttonValues = new uint8_t[_buttonValuesArraySize];
        memset(_buttonValues, 0, _buttonValuesArraySize);
    }

    _hidReportSize  = _buttonValuesArraySize;
    _hidReportSize += (_hatSwitchCount > 0) ? 1 : 0;
    _hidReportSize += axisCount * 2;
    _hidReportSize += simulationCount * 2;

    _xAxis = _yAxis = _zAxis = 0;
    _xAxisRotation = _yAxisRotation = _zAxisRotation = 0;
    _throttle = _rudder = 0;
    for (int i = 0; i < JOYSTICK_HATSWITCH_COUNT_MAXIMUM; i++)
        _hatSwitchValues[i] = JOYSTICK_HATSWITCH_RELEASE;
}

void Joystick_::begin(bool initAutoSendState)
{
    _autoSendState = initAutoSendState;
    sendState();
    for (int i = 0; i < FFB_AXIS_COUNT; ++i) {
        damperFilter[i]  = LowPassFilter(cutoff_freq_damper, sampling_time_damper);
        inertiaFilter[i] = LowPassFilter(cutoff_freq_damper, sampling_time_damper);
        frictionFilter[i]= LowPassFilter(cutoff_freq_damper, sampling_time_damper);
    }
}

void Joystick_::getForce(int16_t* forces)
{
    DynamicHID().RecvfromUsb();
    forceCalculator(forces);
}

float Joystick_::getAngleRatio(volatile TEffectState& effect, int axis)
{
    float angle = (axis < 2 ? effect.direction[0] : effect.direction[1])
                  * 360.0f / 255.0f * DEG_TO_RAD;
    return (axis == 0) ? -sinf(angle) : cosf(angle);
}

// BUG FIX: ApplyEnvelope – division by zero when magnitude=0.
// Guard added: if magnitude is 0 the envelope is meaningless, return 0.
int16_t Joystick_::ApplyEnvelope(volatile TEffectState& effect, int16_t value)
{
    int32_t magnitude   = ApplyGain(effect.magnitude, effect.gain);
    if (magnitude == 0) return 0;  // guard against /0

    int32_t attackLevel = ApplyGain(effect.attackLevel, effect.gain);
    int32_t fadeLevel   = ApplyGain(effect.fadeLevel,   effect.gain);
    int32_t newValue    = magnitude;
    int32_t attackTime  = effect.attackTime;
    int32_t fadeTime    = effect.fadeTime;
    int32_t elapsedTime = effect.elapsedTime;
    int32_t duration    = effect.duration;

    if (elapsedTime < attackTime) {
        newValue = (magnitude - attackLevel) * elapsedTime / attackTime + attackLevel;
    }
    if (elapsedTime > (duration - fadeTime)) {
        newValue = (magnitude - fadeLevel) * (duration - elapsedTime) / fadeTime + fadeLevel;
    }
    return (int16_t)(newValue * value / magnitude);
}

// BUG FIX: NormalizeRange – division by zero when maxValue=0 (axis not calibrated yet).
// Returns 0.0 instead of crashing.
float Joystick_::NormalizeRange(int16_t x, int16_t maxValue)
{
    if (maxValue == 0) return 0.0f;
    float value = (float)x / (float)maxValue;
    return constrain(value, -1.0f, 1.0f);
}

int16_t Joystick_::ApplyGain(uint16_t value, uint8_t gain)
{
    return (int16_t)(((int32_t)value * gain) / 255);
}

int16_t Joystick_::ConstantForceCalculator(volatile TEffectState& effect)
{
    return (int16_t)((float)effect.magnitude * effect.gain / 255.0f);
}

#ifdef ENABLE_PERIODIC_EFFECTS
int16_t Joystick_::RampForceCalculator(volatile TEffectState& effect)
{
    return (int16_t)(effect.startMagnitude
        + (float)effect.elapsedTime / effect.duration
          * (effect.endMagnitude - effect.startMagnitude));
}
#endif

#ifdef ENABLE_PERIODIC_EFFECTS
int16_t Joystick_::SquareForceCalculator(volatile TEffectState& effect)
{
    int16_t  offset       = effect.offset * 2;
    uint16_t magnitude    = effect.magnitude;
    uint16_t elapsedTime  = effect.elapsedTime;
    uint16_t phase        = effect.phase;
    uint16_t period       = effect.period;
    int16_t  maxMagnitude = offset + magnitude;
    int16_t  minMagnitude = offset - magnitude;
    uint16_t phasetime    = (phase * period) / 36000;
    uint16_t timeTemp     = elapsedTime + phasetime;
    uint16_t reminder     = timeTemp % period;
    int16_t  tempforce    = (reminder > (period / 2)) ? minMagnitude : maxMagnitude;
    return ApplyEnvelope(effect, tempforce);
}
#endif

#ifdef ENABLE_PERIODIC_EFFECTS
int16_t Joystick_::SinForceCalculator(volatile TEffectState& effect)
{
    float offset    = effect.offset * 2.0f;
    float magnitude = effect.magnitude;
    float angle     = ((float)effect.elapsedTime / effect.period
                       + (float)effect.phase / 36000.0f) * 2.0f * PI;
    float tempforce = sinf(angle) * magnitude + offset;
    return ApplyEnvelope(effect, (int16_t)tempforce);
}
#endif

#ifdef ENABLE_PERIODIC_EFFECTS
int16_t Joystick_::TriangleForceCalculator(volatile TEffectState& effect)
{
    float    offset       = effect.offset * 2.0f;
    float    magnitude    = effect.magnitude;
    float    periodF      = effect.period;
    float    maxMagnitude = offset + magnitude;
    float    minMagnitude = offset - magnitude;
    uint16_t phasetime    = ((uint16_t)effect.phase * (uint16_t)effect.period) / 36000;
    uint16_t timeTemp     = (uint16_t)effect.elapsedTime + phasetime;
    float    reminder     = timeTemp % (uint16_t)effect.period;
    float    slope        = (maxMagnitude - minMagnitude) * 2.0f / periodF;
    float    tempforce    = (reminder > periodF / 2.0f)
                            ? slope * (periodF - reminder)
                            : slope * reminder;
    tempforce += minMagnitude;
    return ApplyEnvelope(effect, (int16_t)(-tempforce));
}
#endif

#ifdef ENABLE_PERIODIC_EFFECTS
int16_t Joystick_::SawtoothDownForceCalculator(volatile TEffectState& effect)
{
    float    offset       = effect.offset * 2.0f;
    float    magnitude    = effect.magnitude;
    float    periodF      = effect.period;
    float    maxMagnitude = offset + magnitude;
    float    minMagnitude = offset - magnitude;
    int16_t  phasetime    = ((int16_t)effect.phase * (uint16_t)effect.period) / 36000;
    uint16_t timeTemp     = (uint16_t)effect.elapsedTime + phasetime;
    float    reminder     = timeTemp % (uint16_t)effect.period;
    float    slope        = (maxMagnitude - minMagnitude) / periodF;
    float    tempforce    = slope * ((uint16_t)effect.period - reminder) + minMagnitude;
    return ApplyEnvelope(effect, (int16_t)(-tempforce));
}
#endif

#ifdef ENABLE_PERIODIC_EFFECTS
int16_t Joystick_::SawtoothUpForceCalculator(volatile TEffectState& effect)
{
    float    offset       = effect.offset * 2.0f;
    float    magnitude    = effect.magnitude;
    float    periodF      = effect.period;
    float    maxMagnitude = offset + magnitude;
    float    minMagnitude = offset - magnitude;
    int16_t  phasetime    = ((int16_t)effect.phase * (uint16_t)effect.period) / 36000;
    uint16_t timeTemp     = (uint16_t)effect.elapsedTime + phasetime;
    float    reminder     = timeTemp % (uint16_t)effect.period;
    float    slope        = (maxMagnitude - minMagnitude) / periodF;
    float    tempforce    = slope * reminder + minMagnitude;
    return ApplyEnvelope(effect, (int16_t)(-tempforce));
}
#endif

int16_t Joystick_::ConditionForceCalculator(volatile TEffectState& effect, float metric, uint8_t conditionReport)
{
    const auto& cond = effect.conditions[conditionReport];

    // BUG FIX: cpOffset and deadBand from the USB HID report are in the
    // HID logical range [-10000, +10000], but metric (from NormalizeRange)
    // is in [-1, +1]. Comparing them directly caused the condition to be
    // almost always true (0.5 < 5000), making all spring/damper effects
    // push in one direction regardless of position – and crucially, the
    // autopilot spring-follow never worked because a non-zero cpOffset
    // (the AP target position) was never correctly compared against the
    // normalised axis position.
    float cpOffsetNorm = cond.cpOffset  / 10000.0f;
    float deadBandNorm = cond.deadBand  / 10000.0f;
    float negCoeff     = cond.negativeCoefficient;
    float posCoeff     = cond.positiveCoefficient;
    float negSat       = cond.negativeSaturation;
    float posSat       = cond.positiveSaturation;

    float tempForce = 0.0f;
    if (metric < (cpOffsetNorm - deadBandNorm)) {
        tempForce = (metric - cpOffsetNorm + deadBandNorm) * negCoeff;
        if (tempForce < -negSat) tempForce = -negSat;
    } else if (metric > (cpOffsetNorm + deadBandNorm)) {
        tempForce = (metric - cpOffsetNorm - deadBandNorm) * posCoeff;
        if (tempForce > posSat) tempForce = posSat;
    } else {
        return 0;
    }
    return (int16_t)(-tempForce * effect.gain / 255.0f);
}

int16_t Joystick_::getEffectForce(volatile TEffectState& effect, EffectParams _effect_params, uint8_t axis)
{
    float   angle_ratio;
    uint8_t condition = 0;

    if (effect.enableAxis == DIRECTION_ENABLE && effect.conditionReportsCount > 1) {
        angle_ratio = 1.0f;
        condition   = axis;
    } else {
        angle_ratio = getAngleRatio(effect, axis);
    }

    // BUG FIX: removed redundant outer (float)(...) casts.
    // gain/100.0f is already float; the outer cast did nothing except obscure intent.
    float   gainFactor;
    int16_t force = 0;

    switch (effect.effectType)
    {
        case USB_EFFECT_CONSTANT:
            gainFactor = m_gains[axis].constantGain    / 100.0f;
            force = (int16_t)(ConstantForceCalculator(effect)   * gainFactor * angle_ratio);
            break;
#ifdef ENABLE_PERIODIC_EFFECTS
        case USB_EFFECT_RAMP:
            gainFactor = m_gains[axis].rampGain        / 100.0f;
            force = (int16_t)(RampForceCalculator(effect)       * gainFactor * angle_ratio);
            break;
        case USB_EFFECT_SQUARE:
            gainFactor = m_gains[axis].squareGain      / 100.0f;
            force = (int16_t)(SquareForceCalculator(effect)     * gainFactor * angle_ratio);
            break;
        case USB_EFFECT_SINE:
            gainFactor = m_gains[axis].sineGain        / 100.0f;
            force = (int16_t)(SinForceCalculator(effect)        * gainFactor * angle_ratio);
            break;
        case USB_EFFECT_TRIANGLE:
            gainFactor = m_gains[axis].triangleGain    / 100.0f;
            force = (int16_t)(TriangleForceCalculator(effect)   * gainFactor * angle_ratio);
            break;
        case USB_EFFECT_SAWTOOTHDOWN:
            gainFactor = m_gains[axis].sawtoothdownGain / 100.0f;
            force = (int16_t)(SawtoothDownForceCalculator(effect) * gainFactor * angle_ratio);
            break;
        case USB_EFFECT_SAWTOOTHUP:
            gainFactor = m_gains[axis].sawtoothupGain  / 100.0f;
            force = (int16_t)(SawtoothUpForceCalculator(effect) * gainFactor * angle_ratio);
            break;
#endif // ENABLE_PERIODIC_EFFECTS
        case USB_EFFECT_SPRING:
            gainFactor = m_gains[axis].springGain      / 100.0f;
            force = (int16_t)(ConditionForceCalculator(effect,
                NormalizeRange(_effect_params.springPosition,
                               m_effect_params[axis].springMaxPosition), condition)
                * angle_ratio * gainFactor);
            break;
        case USB_EFFECT_DAMPER:
            gainFactor = m_gains[axis].damperGain      / 100.0f;
            force = (int16_t)(ConditionForceCalculator(effect,
                NormalizeRange(_effect_params.damperVelocity,
                               m_effect_params[axis].damperMaxVelocity), condition)
                * angle_ratio * gainFactor);
            force = (int16_t)damperFilter[axis].update((float)force);
            break;
        case USB_EFFECT_INERTIA:
            gainFactor = m_gains[axis].inertiaGain     / 100.0f;
            if (_effect_params.inertiaAcceleration < 0 && _effect_params.frictionPositionChange < 0) {
                force = (int16_t)(ConditionForceCalculator(effect,
                    fabsf(NormalizeRange(_effect_params.inertiaAcceleration,
                                        m_effect_params[axis].inertiaMaxAcceleration)), condition)
                    * angle_ratio * gainFactor);
            } else if (_effect_params.inertiaAcceleration < 0 && _effect_params.frictionPositionChange > 0) {
                force = (int16_t)(-ConditionForceCalculator(effect,
                    fabsf(NormalizeRange(_effect_params.inertiaAcceleration,
                                        m_effect_params[axis].inertiaMaxAcceleration)), condition)
                    * angle_ratio * gainFactor);
            }
            force = (int16_t)inertiaFilter[axis].update((float)force);
            break;
        case USB_EFFECT_FRICTION:
            gainFactor = m_gains[axis].frictionGain    / 100.0f;
            force = (int16_t)(ConditionForceCalculator(effect,
                NormalizeRange(_effect_params.frictionPositionChange,
                               m_effect_params[axis].frictionMaxPositionChange), condition)
                * angle_ratio * gainFactor);
            force = (int16_t)frictionFilter[axis].update((float)force);
            break;
    }

    return force;
}

void Joystick_::forceCalculator(int16_t* forces)
{
    forces[0] = 0;
    forces[1] = 0;

    if (DynamicHID().pidReportHandler.deviceState == MDEVICESTATE_SPRING)
    {
        for (int axis = 0; axis < FFB_AXIS_COUNT; ++axis) {
            forces[axis] = (int16_t)(
                NormalizeRange(m_effect_params[axis].springPosition,
                               m_effect_params[axis].springMaxPosition)
                * -10000.0f
                * (m_gains[axis].defaultSpringGain / 100.0f));

        }
    }
    else
    {
        for (int id = 0; id < MAX_EFFECTS; id++)
        {
            volatile TEffectState& effect = DynamicHID().pidReportHandler.g_EffectStates[id];

            effect.elapsedTime = (uint32_t)(millis() - effect.startTime);

            if ((effect.totalDuration != USB_DURATION_INFINITE) &&
                (effect.elapsedTime >= effect.totalDuration))
                continue;  // effect has finished all repetitions

            // Wrap elapsed time within one repetition
            effect.elapsedTime = effect.elapsedTime % ((uint32_t)effect.duration + effect.startDelay);

            // BUG FIX: startDelay was never respected because elapsedTime is uint64_t
            // and the original check `effect.elapsedTime >= 0` is ALWAYS true for
            // unsigned integers. Now we subtract startDelay only after checking
            // the actual delay period has passed.
            if (effect.elapsedTime < effect.startDelay)
                continue;  // still in the delay period – skip this effect

            effect.elapsedTime -= effect.startDelay;

            if ((effect.state == MEFFECTSTATE_PLAYING) &&
                (effect.elapsedTime <= (uint32_t)effect.duration) &&
                !DynamicHID().pidReportHandler.deviceState)
            {
                EffectParams direction_effect_params = {};
                if (effect.conditionReportsCount == 1) {
                    for (int axis = 0; axis < FFB_AXIS_COUNT; ++axis) {
                        float ar = getAngleRatio(effect, axis);
                        direction_effect_params.springPosition      += (int16_t)(m_effect_params[axis].springPosition      * ar);
                        direction_effect_params.damperVelocity      += (int16_t)(m_effect_params[axis].damperVelocity      * ar);
                        direction_effect_params.inertiaAcceleration += (int16_t)(m_effect_params[axis].inertiaAcceleration * ar);
                        direction_effect_params.frictionPositionChange += (int16_t)(m_effect_params[axis].frictionPositionChange * ar);
                    }
                }

                for (int axis = 0; axis < FFB_AXIS_COUNT; ++axis) {
                    if (effect.enableAxis == DIRECTION_ENABLE || (effect.enableAxis & (1 << axis))) {
                        const EffectParams& ep = (effect.conditionReportsCount == 1)
                                                 ? direction_effect_params
                                                 : m_effect_params[axis];
                        forces[axis] += getEffectForce(effect, ep, axis);
                    }
                }
            }
        }
    }

    for (int axis = 0; axis < FFB_AXIS_COUNT; ++axis) {
        forces[axis] = (int16_t)((float)forces[axis] * m_gains[axis].totalGain / 100.0f);
        forces[axis] = constrain(forces[axis], -10000, 10000);
    }
}

// ── Axis / button setters ──────────────────────────────────────────────────
void Joystick_::end() {}

void Joystick_::setButton(uint8_t button, uint8_t value) {
    if (value == 0) releaseButton(button);
    else            pressButton(button);
}
void Joystick_::pressButton(uint8_t button) {
    if (button >= _buttonCount) return;
    bitSet(_buttonValues[button / 8], button % 8);
    if (_autoSendState) sendState();
}
void Joystick_::releaseButton(uint8_t button) {
    if (button >= _buttonCount) return;
    bitClear(_buttonValues[button / 8], button % 8);
    if (_autoSendState) sendState();
}

void Joystick_::setXAxis(int16_t v) { _xAxis = v;          if (_autoSendState) sendState(); }
void Joystick_::setYAxis(int16_t v) { _yAxis = v;          if (_autoSendState) sendState(); }
void Joystick_::setZAxis(int16_t v) { _zAxis = v;          if (_autoSendState) sendState(); }
void Joystick_::setRudder(int16_t v)   { _rudder   = v;    if (_autoSendState) sendState(); }
void Joystick_::setThrottle(int16_t v) { _throttle  = v;   if (_autoSendState) sendState(); }

void Joystick_::setHatSwitch(int8_t hatSwitchIndex, int16_t value) {
    if (hatSwitchIndex >= _hatSwitchCount) return;
    _hatSwitchValues[hatSwitchIndex] = value;
    if (_autoSendState) sendState();
}

int Joystick_::buildAndSet16BitValue(bool includeValue, int16_t value,
    int16_t valueMinimum, int16_t valueMaximum,
    int16_t actualMinimum, int16_t actualMaximum, uint8_t dataLocation[])
{
    if (!includeValue) return 0;
    int16_t realMinimum = min(valueMinimum, valueMaximum);
    int16_t realMaximum = max(valueMinimum, valueMaximum);
    value = constrain(value, realMinimum, realMaximum);
    if (valueMinimum > valueMaximum)
        value = realMaximum - value + realMinimum;
    int16_t convertedValue = map(value, realMinimum, realMaximum, actualMinimum, actualMaximum);
    dataLocation[0] = (uint8_t)(convertedValue & 0xFF);
    dataLocation[1] = (uint8_t)(convertedValue >> 8);
    return 2;
}

int Joystick_::buildAndSetAxisValue(bool includeAxis, int16_t axisValue, int16_t axisMinimum, int16_t axisMaximum, uint8_t dataLocation[]) {
    return buildAndSet16BitValue(includeAxis, axisValue, axisMinimum, axisMaximum, JOYSTICK_AXIS_MINIMUM, JOYSTICK_AXIS_MAXIMUM, dataLocation);
}

int Joystick_::buildAndSetSimulationValue(bool includeValue, int16_t value, int16_t valueMinimum, int16_t valueMaximum, uint8_t dataLocation[]) {
    return buildAndSet16BitValue(includeValue, value, valueMinimum, valueMaximum, JOYSTICK_SIMULATOR_MINIMUM, JOYSTICK_SIMULATOR_MAXIMUM, dataLocation);
}

void Joystick_::sendState()
{
    uint8_t data[_hidReportSize];
    int index = 0;

    for (; index < _buttonValuesArraySize; index++)
        data[index] = _buttonValues[index];

    if (_hatSwitchCount > 0) {
        uint8_t convertedHatSwitch[JOYSTICK_HATSWITCH_COUNT_MAXIMUM];
        for (int i = 0; i < JOYSTICK_HATSWITCH_COUNT_MAXIMUM; i++)
            convertedHatSwitch[i] = (_hatSwitchValues[i] < 0) ? 8 : (_hatSwitchValues[i] % 360) / 45;
        data[index++] = (convertedHatSwitch[1] << 4) | (0x0F & convertedHatSwitch[0]);
    }

    index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_X_AXIS,  _xAxis,         _xAxisMinimum,  _xAxisMaximum,  &data[index]);
    index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_Y_AXIS,  _yAxis,         _yAxisMinimum,  _yAxisMaximum,  &data[index]);
    index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_Z_AXIS,  _zAxis,         _zAxisMinimum,  _zAxisMaximum,  &data[index]);
    index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_RX_AXIS, _xAxisRotation, _rxAxisMinimum, _rxAxisMaximum, &data[index]);
    index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_RY_AXIS, _yAxisRotation, _ryAxisMinimum, _ryAxisMaximum, &data[index]);
    index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_RZ_AXIS, _zAxisRotation, _rzAxisMinimum, _rzAxisMaximum, &data[index]);
    index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_RUDDER,   _rudder,   _rudderMinimum,   _rudderMaximum,   &data[index]);
    index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_THROTTLE, _throttle, _throttleMinimum, _throttleMaximum, &data[index]);

    DynamicHID().SendReport(_hidReportId, data, _hidReportSize);
}

#endif // _USING_DYNAMIC_HID
