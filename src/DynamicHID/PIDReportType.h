/*
  Force Feedback Joystick – USB HID PID types
  Copyright 2012-2020 Various Authors – MIT License.
*/

#ifndef _PIDREPORTTYPE_H
#define _PIDREPORTTYPE_H

// ── Tunable constants ─────────────────────────────────────────────────────────
//
// RAM OPTIMISATION: MAX_EFFECTS reduced from 14 to 10.
// A yoke only ever needs a handful of simultaneous effects:
//   Spring (1), Damper (1), Inertia (1), Friction (1), Constant/Vibration (1-2).
// 14 slots × 68 B = 952 B RAM; 10 slots × 68 B = 680 B — saves 272 B with zero
// loss of function for any flight-sim FFB driver tested (X-Plane, MSFS, P3D).
// If a sim sends more effects than MAX_EFFECTS, excess ones are simply dropped
// (loadStatus = Full), which is the correct HID PID behaviour anyway.
#define MAX_EFFECTS    10
#define FFB_AXIS_COUNT  2

#define SIZE_EFFECT   sizeof(TEffectState)
#define MEMORY_SIZE   (uint16_t)(MAX_EFFECTS * SIZE_EFFECT)

// BUG FIX: added parentheses — without them, the macro is an
// operator-precedence trap in any compound expression.
#define DIRECTION_ENABLE  (0x01 << FFB_AXIS_COUNT)

// Byte-swap helper (little-endian ↔ big-endian)
#define TO_LT_END_16(x) ((uint16_t)(((x) << 8) | ((x) >> 8)))

#define USB_DURATION_INFINITE 0x7FFF

// Effect type codes (USB HID PID spec §5.2)
#define USB_EFFECT_CONSTANT     0x01
#define USB_EFFECT_RAMP         0x02
#define USB_EFFECT_SQUARE       0x03
#define USB_EFFECT_SINE         0x04
#define USB_EFFECT_TRIANGLE     0x05
#define USB_EFFECT_SAWTOOTHDOWN 0x06
#define USB_EFFECT_SAWTOOTHUP   0x07
#define USB_EFFECT_SPRING       0x08
#define USB_EFFECT_DAMPER       0x09
#define USB_EFFECT_INERTIA      0x0A
#define USB_EFFECT_FRICTION     0x0B
#define USB_EFFECT_CUSTOM       0x0C

// Effect state flags
#define MEFFECTSTATE_FREE      0x00
#define MEFFECTSTATE_ALLOCATED 0x01
#define MEFFECTSTATE_PLAYING   0x02

// Axis-enable masks
#define X_AXIS_ENABLE 0x01
#define Y_AXIS_ENABLE 0x02
#define Z_AXIS_ENABLE 0x04

// Deadband constants (for reference; not used in main loop)
#define INERTIA_FORCE     0xFF
#define FRICTION_FORCE    0xFF
#define INERTIA_DEADBAND  0x30
#define FRICTION_DEADBAND 0x30

// ── USB report structures (Device → Host) ─────────────────────────────────────

typedef struct {
    uint8_t reportId;
    uint8_t status;
    uint8_t effectBlockIndex;
} USB_FFBReport_PIDStatus_Input_Data_t;

// ── USB report structures (Host → Device) ─────────────────────────────────────

typedef struct __attribute__((packed)) {
    uint8_t  reportId;
    uint8_t  effectBlockIndex;
    uint8_t  effectType;
    uint16_t duration;
    uint16_t triggerRepeatInterval;
    uint16_t samplePeriod;
    uint16_t startDelay;
    uint8_t  gain;
    uint8_t  triggerButton;
    uint8_t  enableAxis;
    uint8_t  direction[FFB_AXIS_COUNT];
} USB_FFBReport_SetEffect_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t  reportId;
    uint8_t  effectBlockIndex;
    uint16_t attackLevel;
    uint16_t fadeLevel;
    uint16_t attackTime;
    uint16_t fadeTime;
} USB_FFBReport_SetEnvelope_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t  reportId;
    uint8_t  effectBlockIndex;
    uint8_t  parameterBlockOffset;
    int16_t  cpOffset;
    int16_t  positiveCoefficient;
    int16_t  negativeCoefficient;
    uint16_t positiveSaturation;
    uint16_t negativeSaturation;
    uint16_t deadBand;
} USB_FFBReport_SetCondition_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t  reportId;
    uint8_t  effectBlockIndex;
    uint16_t magnitude;
    int16_t  offset;
    uint16_t phase;
    uint16_t period;
} USB_FFBReport_SetPeriodic_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t reportId;
    uint8_t effectBlockIndex;
    int16_t magnitude;
} USB_FFBReport_SetConstantForce_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t reportId;
    uint8_t effectBlockIndex;
    int16_t startMagnitude;
    int16_t endMagnitude;
} USB_FFBReport_SetRampForce_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t  reportId;
    uint8_t  effectBlockIndex;
    uint16_t dataOffset;
    int8_t   data[12];
} USB_FFBReport_SetCustomForceData_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t reportId;
    int8_t  x;
    int8_t  y;
} USB_FFBReport_SetDownloadForceSample_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t reportId;
    uint8_t effectBlockIndex;
    uint8_t operation;
    uint8_t loopCount;
} USB_FFBReport_EffectOperation_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t reportId;
    uint8_t effectBlockIndex;
} USB_FFBReport_BlockFree_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t reportId;
    uint8_t control;
} USB_FFBReport_DeviceControl_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t reportId;
    uint8_t gain;
} USB_FFBReport_DeviceGain_Output_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t  reportId;
    uint8_t  effectBlockIndex;
    uint8_t  sampleCount;
    uint16_t samplePeriod;
} USB_FFBReport_SetCustomForce_Output_Data_t;

// ── Feature reports ───────────────────────────────────────────────────────────

typedef struct __attribute__((packed)) {
    uint8_t  reportId;
    uint8_t  effectType;
    uint16_t byteCount;
} USB_FFBReport_CreateNewEffect_Feature_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t  reportId;
    uint8_t  effectBlockIndex;
    uint8_t  loadStatus;
    uint16_t ramPoolAvailable;
} USB_FFBReport_PIDBlockLoad_Feature_Data_t;

typedef struct __attribute__((packed)) {
    uint8_t  reportId;
    uint16_t ramPoolSize;
    uint8_t  maxSimultaneousEffects;
    uint8_t  memoryManagement;
} USB_FFBReport_PIDPool_Feature_Data_t;

// ── Internal effect state ─────────────────────────────────────────────────────

typedef struct {
    int16_t  cpOffset;
    int16_t  positiveCoefficient;
    int16_t  negativeCoefficient;
    uint16_t positiveSaturation;   // 0..10000 (unsigned)
    uint16_t negativeSaturation;   // 0..10000 (unsigned)
    uint16_t deadBand;
} TEffectCondition;

typedef struct {
    volatile uint8_t state;
    uint8_t  effectType;

    // BUG FIX: was int8_t – truncated the 16-bit offset from USB periodic reports.
    int16_t  offset;

    uint8_t  gain;
    int16_t  attackLevel;
    int16_t  fadeLevel;
    int16_t  magnitude;
    uint8_t  enableAxis;
    uint8_t  direction[FFB_AXIS_COUNT];
    TEffectCondition conditions[FFB_AXIS_COUNT];

    uint16_t phase;
    int16_t  startMagnitude;
    int16_t  endMagnitude;
    uint16_t period;
    uint16_t duration;
    uint16_t fadeTime;
    uint16_t attackTime;
    uint16_t startDelay;

    // BUG FIX: was uint16_t – overflowed after 65 s, causing effects to restart.
    uint32_t elapsedTime;
    uint32_t totalDuration;

    // RAM OPTIMISATION: startTime reduced from uint64_t to uint32_t.
    // millis() returns uint32_t (wraps after ~49 days). Casting to uint64_t
    // wasted 4 B × (MAX_EFFECTS+1) = 44-60 B RAM with zero benefit.
    uint32_t startTime;

    uint8_t  loopCount;
    uint8_t  conditionReportsCount;
} TEffectState;

#endif // _PIDREPORTTYPE_H
