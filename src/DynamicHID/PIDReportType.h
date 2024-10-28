// /*
  // Force Feedback Joystick
  // USB HID descriptors for a force feedback joystick.
  // Copyright 2012  Tero Loimuneva (tloimu [at] gmail [dot] com)
  // Copyright 2016  Jaka Simonic (telesimke [at] gmail [dot] com)
  // Copyright 2019  Hoan Tran (tranvanhoan206 [at] gmail [dot] com)
  // Copyright 2020  Yo Law (lymmm3 [at] gmail [dot] com)
  // MIT License.
  // Permission to use, copy, modify, distribute, and sell this
  // software and its documentation for any purpose is hereby granted
  // without fee, provided that the above copyright notice appear in
  // all copies and that both that the copyright notice and this
  // permission notice and warranty disclaimer appear in supporting
  // documentation, and that the name of the author not be used in
  // advertising or publicity pertaining to distribution of the
  // software without specific, written prior permission.
  // The author disclaim all warranties with regard to this
  // software, including all implied warranties of merchantability
  // and fitness.  In no event shall the author be liable for any
  // special, indirect or consequential damages or any damages
  // whatsoever resulting from loss of use, data or profits, whether
  // in an action of contract, negligence or other tortious action,
  // arising out of or in connection with the use or performance of
  // this software.
// */

/*
  Force Feedback Joystick
  USB HID descriptors for a force feedback joystick.
  Copyright 2012-2020 Various Authors
  MIT License.
*/

#ifndef _PIDREPORTTYPE_H  // Include guard to prevent multiple inclusions of this header file
#define _PIDREPORTTYPE_H

// Define constants related to the force feedback joystick
#define MAX_EFFECTS 14  // Maximum number of effects supported
#define FFB_AXIS_COUNT 2 // Number of axes for force feedback
#define SIZE_EFFECT sizeof(TEffectState) // Size of the effect state structure
#define MEMORY_SIZE (uint16_t)(MAX_EFFECTS * SIZE_EFFECT) // Total memory required for effects
// Macro to convert a 16-bit value to little-endian format
#define TO_LT_END_16(x) (((x) << 8) | ((x) >> 8))
//#define TO_LT_END_16(x) ((x<<8)&0xFF00)|((x>>8)&0x00FF)

// Effect constants for force feedback
#define USB_DURATION_INFINITE 0x7FFF // Represents infinite duration

// Effect types for the force feedback joystick
#define USB_EFFECT_CONSTANT         0x01
#define USB_EFFECT_RAMP             0x02
#define USB_EFFECT_SQUARE           0x03
#define USB_EFFECT_SINE             0x04
#define USB_EFFECT_TRIANGLE         0x05
#define USB_EFFECT_SAWTOOTHDOWN     0x06
#define USB_EFFECT_SAWTOOTHUP       0x07
#define USB_EFFECT_SPRING           0x08
#define USB_EFFECT_DAMPER           0x09
#define USB_EFFECT_INERTIA          0x0A
#define USB_EFFECT_FRICTION         0x0B
#define USB_EFFECT_CUSTOM           0x0C

// Bit-masks for effect states
#define MEFFECTSTATE_FREE           0x00 // Effect is free and not in use
#define MEFFECTSTATE_ALLOCATED      0x01 // Effect has been allocated
#define MEFFECTSTATE_PLAYING        0x02 // Effect is currently playing

// Bit-mask definitions for enabling axes
#define X_AXIS_ENABLE               0x01 // Enable X-axis
#define Y_AXIS_ENABLE               0x02 // Enable Y-axis
#define Z_AXIS_ENABLE               0x04 // Enable Z-axis
#define DIRECTION_ENABLE            0x01 << FFB_AXIS_COUNT // Enable direction based on axis count

// Testing constants for inertia and friction
#define INERTIA_FORCE               0xFF
#define FRICTION_FORCE              0xFF
#define INERTIA_DEADBAND           0x30 // Deadband for inertia force
#define FRICTION_DEADBAND           0x30 // Deadband for friction force

///Device-->Host

// Input Data Report structure for device status
typedef struct {
    uint8_t reportId; // Report ID (2)
    uint8_t status; // Bitfield for device status
    uint8_t effectBlockIndex; // Effect block index: Bit7 (playing), Bit0..7 (Effect ID 1..40)
} USB_FFBReport_PIDStatus_Input_Data_t;

// Output Data Reports structures for different effect settings
typedef struct __attribute__((packed)) { // Ensures tight packing of structure
    uint8_t reportId; // Report ID (1)
    uint8_t effectBlockIndex; // Effect block index (1..40)
    uint8_t effectType; // Type of effect (1..12)
    uint16_t duration; // Duration of effect (0..32767 ms)
    uint16_t triggerRepeatInterval; // Repeat interval for triggering the effect
    uint16_t samplePeriod; // Period for sampling the effect
    uint16_t startDelay; // Delay before starting the effect
    uint8_t gain; // Gain value (0..255)
    uint8_t triggerButton; // Button ID that triggers the effect (0..8)
    uint8_t enableAxis; // Bits for enabling axes (0=X, 1=Y, 2=Direction)
    uint8_t direction[FFB_AXIS_COUNT]; // Direction angles (0=0 .. 255=360 degrees)
} USB_FFBReport_SetEffect_Output_Data_t;

// Structure for setting the envelope of an effect
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (2)
    uint8_t effectBlockIndex; // Effect block index (1..40)
    uint16_t attackLevel; // Level during the attack phase
    uint16_t fadeLevel; // Level during the fade phase
    uint16_t attackTime; // Duration of the attack phase (ms)
    uint16_t fadeTime; // Duration of the fade phase (ms)
} USB_FFBReport_SetEnvelope_Output_Data_t;

// Structure for setting conditions of an effect
typedef struct __attribute__((packed)) // FFB: Set Condition Output Report
{
	uint8_t	reportId;	// =3
	uint8_t	effectBlockIndex;	// 1..40
	uint8_t	parameterBlockOffset;	// bits: 0..3=parameterBlockOffset, 4..5=instance1, 6..7=instance2
	int16_t cpOffset;	// 0..255
	int16_t	positiveCoefficient;	// -128..127
	int16_t	negativeCoefficient;	// -128..127
	uint16_t	positiveSaturation;	// -	128..127
	uint16_t	negativeSaturation;	// -128..127
	uint16_t	deadBand;	// 0..255
} USB_FFBReport_SetCondition_Output_Data_t;

// Structure for setting periodic effects
typedef struct __attribute__((packed)) //FFB: Set Periodic Output Report
{
	uint8_t	reportId;	// =4
	uint8_t	effectBlockIndex;	// 1..40
	uint16_t magnitude;
	int16_t	offset;
	uint16_t	phase;	// 0..255 (=0..359, exp-2)
	uint16_t	period;	// 0..32767 ms
} USB_FFBReport_SetPeriodic_Output_Data_t;

// Structure for setting a constant force effect
typedef struct __attribute__((packed)) //FFB: Set ConstantForce Output Report
{
	uint8_t	reportId;	// =5
	uint8_t	effectBlockIndex;	// 1..40
	int16_t magnitude;	// -255..255
} USB_FFBReport_SetConstantForce_Output_Data_t;

// Structure for setting a ramp force effect
typedef struct __attribute__((packed)) //FFB: Set RampForce Output Report
{
	uint8_t	reportId;	// =6
	uint8_t	effectBlockIndex;	// 1..40
	int16_t startMagnitude;
	int16_t	endMagnitude;
} USB_FFBReport_SetRampForce_Output_Data_t;

// Structure for setting custom force data
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (7)
    uint8_t effectBlockIndex; // Effect block index (1..40)
    uint16_t dataOffset; // Offset for custom data
    int8_t data[12]; // Fixed-size data array for custom forces
} USB_FFBReport_SetCustomForceData_Output_Data_t;

// Structure for setting a force sample to download
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (8)
    int8_t x; // X-axis force sample
    int8_t y; // Y-axis force sample
} USB_FFBReport_SetDownloadForceSample_Output_Data_t;

// Structure for managing effect operations
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (10)
    uint8_t effectBlockIndex; // Effect block index (1..40)
    uint8_t operation; // Operation type (1=Start, 2=StartSolo, 3=Stop)
    uint8_t loopCount; // Number of times to loop the effect
} USB_FFBReport_EffectOperation_Output_Data_t;

// Structure for freeing effect blocks
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (11)
    uint8_t effectBlockIndex; // Effect block index (1..40)
} USB_FFBReport_BlockFree_Output_Data_t;

// Structure for device control commands
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (12)
    uint8_t control; // Control command for device
} USB_FFBReport_DeviceControl_Output_Data_t;

// Structure for setting device gain
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (13)
    uint8_t gain; // Gain value for device
} USB_FFBReport_DeviceGain_Output_Data_t;

// Structure for setting custom force samples
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (14)
    uint8_t effectBlockIndex; // Effect block index (1..40)
    uint8_t sampleCount; // Number of samples for the custom force
    uint16_t samplePeriod; // Period between samples (0..32767 ms)
} USB_FFBReport_SetCustomForce_Output_Data_t;

// Feature Reports for managing effects
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (5)
    uint8_t effectType; // Type of effect (1..12)
    uint16_t byteCount; // Number of bytes for the effect data (0..511)
} USB_FFBReport_CreateNewEffect_Feature_Data_t;

// Structure for reporting load status of effect blocks
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (6)
    uint8_t effectBlockIndex; // Effect block index (1..40)
    uint8_t loadStatus; // Load status (1=Success, 2=Full, 3=Error)
    uint16_t ramPoolAvailable; // Available RAM pool (0 or 0xFFFF)
} USB_FFBReport_PIDBlockLoad_Feature_Data_t;

// Structure for reporting the RAM pool size and effect management
typedef struct __attribute__((packed)) {
    uint8_t reportId; // Report ID (7)
    uint16_t ramPoolSize; // Size of the RAM pool
    uint8_t maxSimultaneousEffects; // Maximum number of simultaneous effects
    uint8_t memoryManagement; // Memory management settings (0=DeviceManagedPool, 1=SharedParameterBlocks)
} USB_FFBReport_PIDPool_Feature_Data_t;



// Structure for effect conditions
typedef struct {
	int16_t cpOffset; // -128..127
	int16_t  positiveCoefficient; // -128..127
	int16_t  negativeCoefficient; // -128..127
	uint16_t positiveSaturation;  // -128..127
	uint16_t negativeSaturation;  // -128..127
	uint16_t deadBand;  // 0..255
} TEffectCondition;

// Structure for maintaining the state of an effect
typedef struct {
    volatile uint8_t state; // Current state of the effect
    uint8_t effectType; // Type of effect
    int8_t offset; // Offset for the effect
    uint8_t gain; // Gain value for the effect
    int16_t attackLevel; // Attack level for the effect
    int16_t fadeLevel; // Fade level for the effect
    int16_t magnitude; // Magnitude of the effect
    uint8_t enableAxis; // Axis enabling bits (0=X, 1=Y, 2=Direction)
    uint8_t direction[FFB_AXIS_COUNT]; // Direction angles for force feedback
    TEffectCondition conditions[FFB_AXIS_COUNT]; // Conditions for the effect on each axis

    uint16_t phase; // Current phase of the effect (0..255)
    int16_t startMagnitude; // Starting magnitude for ramp or effects
    int16_t endMagnitude; // Ending magnitude for ramp or effects
    uint16_t period; // Period of the effect (0..32767 ms)
    uint16_t duration; // Total duration of the effect
    uint16_t fadeTime; // Fade time for the effect
    uint16_t attackTime; // Attack time for the effect
    uint16_t elapsedTime; // Elapsed time since the effect started
    uint16_t totalDuration; // Total duration of the effect including fade and attack times
    uint16_t startDelay; // Delay before starting the effect
    uint64_t startTime; // Timestamp when the effect started
    uint8_t loopCount; // Number of times to loop the effect
    uint8_t conditionReportsCount; // Count of condition reports for the effect
} TEffectState;
#endif
