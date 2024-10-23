#ifndef _PIDREPORTHANDLER_H
#define _PIDREPORTHANDLER_H

// Bit-masks for different device states
#define MDEVICESTATE_FREE   0x00  // Device is free and not in use
#define MDEVICESTATE_PAUSED 0x01  // Device is paused
#define MDEVICESTATE_SPRING 0x02  // Device is in a spring state

#include <Arduino.h>              // Include Arduino core library
#include "PIDReportType.h"       // Include custom report type definitions

class PIDReportHandler {
public:
	PIDReportHandler();             // Constructor to initialize the handler
	~PIDReportHandler();            // Destructor to clean up resources

	// Effect management variables
	volatile uint8_t nextEID;      // Next effect index, starts from 1
	volatile TEffectState g_EffectStates[MAX_EFFECTS + 1]; // Array to store effect states
	volatile uint8_t deviceState;   // Variable to store the current device state

	// Variables for storing previous values used in calculations
	volatile int16_t inertiaT = 0;  // Variable to store inertia time
	volatile int16_t oldSpeed = 0;   // Variable to store the previous speed
	volatile int16_t oldAxisPosition = 0;  // Variable to store the previous axis position

	// Report structures to manage device state and reports
	volatile USB_FFBReport_PIDStatus_Input_Data_t pidState = { 2, 30, 0 }; // Status input report
	volatile USB_FFBReport_PIDBlockLoad_Feature_Data_t pidBlockLoad; // Block load feature report
	volatile USB_FFBReport_PIDPool_Feature_Data_t pidPoolReport; // Pool feature report
	volatile USB_FFBReport_DeviceGain_Output_Data_t deviceGain; // Device gain output report

	// Methods for managing effects
	uint8_t GetNextFreeEffect(void); // Get the next available effect ID
	void StartEffect(uint8_t id);     // Start a specific effect by ID
	void StopEffect(uint8_t id);      // Stop a specific effect by ID
	void StopAllEffects(void);         // Stop all currently running effects
	void FreeEffect(uint8_t id);      // Free a specific effect by ID
	void FreeAllEffects(void);         // Free all effects and reset states

	// Handle output PID report operations
	void EffectOperation(USB_FFBReport_EffectOperation_Output_Data_t* data); // Manage effect operations
	void BlockFree(USB_FFBReport_BlockFree_Output_Data_t* data); // Free specific effect block
	void DeviceControl(USB_FFBReport_DeviceControl_Output_Data_t* data); // Control device settings
	void DeviceGain(USB_FFBReport_DeviceGain_Output_Data_t* data); // Set device gain value
	void SetCustomForceData(USB_FFBReport_SetCustomForceData_Output_Data_t* data); // Set custom force data
	void SetDownloadForceSample(USB_FFBReport_SetDownloadForceSample_Output_Data_t* data); // Set download force sample
	void SetCustomForce(USB_FFBReport_SetCustomForce_Output_Data_t* data); // Set custom force
	void SetEffect(USB_FFBReport_SetEffect_Output_Data_t* data); // Set parameters for an effect
	void SetEnvelope(USB_FFBReport_SetEnvelope_Output_Data_t* data, volatile TEffectState* effect); // Set envelope parameters
	void SetCondition(USB_FFBReport_SetCondition_Output_Data_t* data, volatile TEffectState* effect); // Set condition parameters
	void SetPeriodic(USB_FFBReport_SetPeriodic_Output_Data_t* data, volatile TEffectState* effect); // Set periodic effect parameters
	void SetConstantForce(USB_FFBReport_SetConstantForce_Output_Data_t* data, volatile TEffectState* effect); // Set constant force parameters
	void SetRampForce(USB_FFBReport_SetRampForce_Output_Data_t* data, volatile TEffectState* effect); // Set ramp force parameters

	// Handle incoming data from USB reports
	void CreateNewEffect(USB_FFBReport_CreateNewEffect_Feature_Data_t* inData); // Create a new effect from incoming data
	void UppackUsbData(uint8_t* data, uint16_t len);
	uint8_t* getPIDPool(); // Get PID pool report data
	uint8_t* getPIDBlockLoad(); // Get PID block load report data
	uint8_t* getPIDStatus(); // Get PID status report data
};
#endif
