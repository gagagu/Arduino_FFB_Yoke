#include "PIDReportHandler.h"

// Constructor: Initializes the PIDReportHandler with default values
PIDReportHandler::PIDReportHandler() 
{
    nextEID = 1;  // Next available Effect ID starts at 1
    deviceState = MDEVICESTATE_SPRING;  // Set device state to SPRING mode
}

// Destructor: Frees all effects when the handler is destroyed
PIDReportHandler::~PIDReportHandler() 
{
    FreeAllEffects();  // Free all allocated effects
}

// Gets the next available effect ID and marks it as allocated
uint8_t PIDReportHandler::GetNextFreeEffect(void)
{
    if (nextEID == MAX_EFFECTS)  // If all effects are allocated, return 0
        return 0;

    uint8_t id = nextEID++;  // Allocate the next effect ID

    // Search for the next free spot in the effect states
    while (g_EffectStates[nextEID].state != 0)
    {
        if (nextEID >= MAX_EFFECTS)  // If no more free spots, break out
            break;
        nextEID++;
    }

    // Mark the effect as allocated and update the PID state
    g_EffectStates[id].state = MEFFECTSTATE_ALLOCATED;
    pidState.effectBlockIndex = id;

    return id;  // Return the allocated effect ID
}

// Stops all currently active effects
void PIDReportHandler::StopAllEffects(void)
{
    for (uint8_t id = 0; id <= MAX_EFFECTS; id++)  // Loop through all effects
        StopEffect(id);  // Stop each effect
}

// Starts a specific effect by its ID
void PIDReportHandler::StartEffect(uint8_t id)
{
    if (id > MAX_EFFECTS)  // Check if the effect ID is valid
        return;
    
    // Mark the effect as playing, reset elapsed time, and record the start time
    g_EffectStates[id].state = MEFFECTSTATE_PLAYING;
    g_EffectStates[id].elapsedTime = 0;
    g_EffectStates[id].startTime = (uint64_t)millis();
}

// Stops a specific effect by its ID
void PIDReportHandler::StopEffect(uint8_t id)
{
    if (id > MAX_EFFECTS)  // Check if the effect ID is valid
        return;
    
    // Remove the "playing" state flag and free up memory
    g_EffectStates[id].state &= ~MEFFECTSTATE_PLAYING;
    pidBlockLoad.ramPoolAvailable += SIZE_EFFECT;
}

// Frees a specific effect by its ID
void PIDReportHandler::FreeEffect(uint8_t id)
{
    if (id > MAX_EFFECTS)  // Check if the effect ID is valid
        return;
    
    g_EffectStates[id].state = 0;  // Mark the effect as free
    if (id < nextEID)  // Update nextEID if needed
        nextEID = id;
}

// Frees all effects and resets the handler
void PIDReportHandler::FreeAllEffects(void)
{
    nextEID = 1;  // Reset next available effect ID
    memset((void*)&g_EffectStates, 0, sizeof(g_EffectStates));  // Clear all effect states
    pidBlockLoad.ramPoolAvailable = MEMORY_SIZE;  // Reset available memory
}

// Handles effect operation based on the received data (start, stop, loop)
void PIDReportHandler::EffectOperation(USB_FFBReport_EffectOperation_Output_Data_t* data)
{
    g_EffectStates[data->effectBlockIndex].loopCount = data->loopCount;
    if (data->operation == 1)  // Start effect
    {
        if (data->loopCount == 0xFF)
        {
            g_EffectStates[data->effectBlockIndex].totalDuration = USB_DURATION_INFINITE;
        }
        else if (data->loopCount > 0)
        {
            // Calculate total duration based on loop count
            g_EffectStates[data->effectBlockIndex].totalDuration = (
                g_EffectStates[data->effectBlockIndex].startDelay
                + g_EffectStates[data->effectBlockIndex].duration
            ) * data->loopCount;
        }
        StartEffect(data->effectBlockIndex);  // Start the effect
    }
    else if (data->operation == 2)  // StartSolo: stop all and start a specific effect
    {
        StopAllEffects();
        StartEffect(data->effectBlockIndex);
    }
    else if (data->operation == 3)  // Stop effect
    {
        StopEffect(data->effectBlockIndex);
    }
}

// Handles the BlockFree operation to free one or all effects
void PIDReportHandler::BlockFree(USB_FFBReport_BlockFree_Output_Data_t* data)
{
    uint8_t eid = data->effectBlockIndex;
    if (eid == 0xFF)  // Free all effects
    {
        FreeAllEffects();
    }
    else  // Free a specific effect
    {
        FreeEffect(eid);
    }
}

// Handles device control commands (enable/disable actuators, reset, pause, etc.)
void PIDReportHandler::DeviceControl(USB_FFBReport_DeviceControl_Output_Data_t* data)
{
    uint8_t control = data->control;

    switch (control)
    {
        case 0x01:  // Enable Actuators
            pidState.status |= 2;
            break;
        case 0x02:  // Disable Actuators
            pidState.status &= ~(0x02);
            break;
        case 0x03:  // Stop All Effects
            StopAllEffects();
            deviceState &= ~(MDEVICESTATE_SPRING);
            break;
        case 0x04:  // Reset Device
            FreeAllEffects();
            deviceState |= MDEVICESTATE_SPRING;
            break;
        case 0x05:  // Pause Device
            deviceState |= MDEVICESTATE_PAUSED;
            break;
        case 0x06:  // Continue Device
            deviceState &= ~(MDEVICESTATE_PAUSED);
            break;
        default:  // Handle unknown commands
            break;
    }
}

// Sets the device gain based on received data
void PIDReportHandler::DeviceGain(USB_FFBReport_DeviceGain_Output_Data_t* data)
{
    deviceGain.gain = data->gain;  // Update the device gain
}

// Placeholder for setting custom force (not yet implemented)
void PIDReportHandler::SetCustomForce(USB_FFBReport_SetCustomForce_Output_Data_t* data)
{
}

// Placeholder for setting custom force data (not yet implemented)
void PIDReportHandler::SetCustomForceData(USB_FFBReport_SetCustomForceData_Output_Data_t* data)
{
}

// Placeholder for setting download force sample (not yet implemented)
void PIDReportHandler::SetDownloadForceSample(USB_FFBReport_SetDownloadForceSample_Output_Data_t* data)
{
}

// Sets an effect with the provided data (effect type, duration, direction, etc.)
void PIDReportHandler::SetEffect(USB_FFBReport_SetEffect_Output_Data_t* data)
{
    volatile TEffectState* effect = &g_EffectStates[data->effectBlockIndex];

    effect->duration = data->duration;  // Set effect duration
    for (int i=0; i<FFB_AXIS_COUNT; ++i)
    {
        effect->direction[i] = data->direction[i];  // Set effect direction
    }
    effect->effectType = data->effectType;
    effect->gain = data->gain;
    effect->enableAxis = data->enableAxis;
    effect->startDelay = data->startDelay;

    // Recalculate effect duration if looping
    if (effect->loopCount != 0xFF)
    {
        uint8_t loopCount = effect->loopCount > 0 ? effect->loopCount : 1;
        effect->totalDuration = (data->duration + data->startDelay) * loopCount;
    }
}

// Sets the envelope for an effect (attack and fade levels and times)
void PIDReportHandler::SetEnvelope(USB_FFBReport_SetEnvelope_Output_Data_t* data, volatile TEffectState* effect)
{
    effect->attackLevel = data->attackLevel;
    effect->fadeLevel = data->fadeLevel;
    effect->attackTime = data->attackTime;
    effect->fadeTime = data->fadeTime;
}

// Sets the condition (axis-specific parameters) for an effect
void PIDReportHandler::SetCondition(USB_FFBReport_SetCondition_Output_Data_t* data, volatile TEffectState* effect)
{
    uint8_t axis = data->parameterBlockOffset;  // Get the axis
    if (axis >= effect->conditionReportsCount)
    {
        effect->conditionReportsCount = axis + 1;  // Update condition report count
    }

    // Set condition parameters for the axis
    effect->conditions[axis].cpOffset = data->cpOffset;
    effect->conditions[axis].positiveCoefficient = data->positiveCoefficient;
    effect->conditions[axis].negativeCoefficient = data->negativeCoefficient;
    effect->conditions[axis].positiveSaturation = data->positiveSaturation;
    effect->conditions[axis].negativeSaturation = data->negativeSaturation;
    effect->conditions[axis].deadBand = data->deadBand;
}

// Sets periodic parameters for an effect (magnitude, offset, phase, period)
void PIDReportHandler::SetPeriodic(USB_FFBReport_SetPeriodic_Output_Data_t* data, volatile TEffectState* effect)
{
    effect->magnitude = data->magnitude;
    effect->offset = data->offset;
    effect->phase = data->phase;
    effect->period = data->period;
}

void PIDReportHandler::SetConstantForce(USB_FFBReport_SetConstantForce_Output_Data_t* data, volatile TEffectState* effect)
{
	//  ReportPrint(*effect);
	effect->magnitude = data->magnitude;
}

void PIDReportHandler::SetRampForce(USB_FFBReport_SetRampForce_Output_Data_t* data, volatile TEffectState* effect)
{
	effect->startMagnitude = data->startMagnitude;
	effect->endMagnitude = data->endMagnitude;
}

void PIDReportHandler::CreateNewEffect(USB_FFBReport_CreateNewEffect_Feature_Data_t* inData)
{
	pidBlockLoad.reportId = 6;
	pidBlockLoad.effectBlockIndex = GetNextFreeEffect();

	if (pidBlockLoad.effectBlockIndex == 0)
	{
		pidBlockLoad.loadStatus = 2;    // 1=Success,2=Full,3=Error
	}
	else
	{
		pidBlockLoad.loadStatus = 1;    // 1=Success,2=Full,3=Error

		volatile TEffectState* effect = &g_EffectStates[pidBlockLoad.effectBlockIndex];

		memset((void*)effect, 0, sizeof(TEffectState));
		effect->state = MEFFECTSTATE_ALLOCATED;
		pidBlockLoad.ramPoolAvailable -= SIZE_EFFECT;
	}
}

// Unpack USB data based on the incoming report ID
void PIDReportHandler::UppackUsbData(uint8_t* data, uint16_t len)
{
    // Extract the effect ID from the incoming data
    uint8_t effectId = data[1];  // The effectBlockIndex is always the second byte.
    
    // Handle different report IDs
    switch (data[0])  // reportID
    {
    case 1:
        SetEffect((USB_FFBReport_SetEffect_Output_Data_t*)data);  // Set effect
        break;
    case 2:
        SetEnvelope((USB_FFBReport_SetEnvelope_Output_Data_t*)data, &g_EffectStates[effectId]);  // Set envelope
        break;
    case 3:
        SetCondition((USB_FFBReport_SetCondition_Output_Data_t*)data, &g_EffectStates[effectId]);  // Set condition
        break;
    case 4:
        SetPeriodic((USB_FFBReport_SetPeriodic_Output_Data_t*)data, &g_EffectStates[effectId]);  // Set periodic effect
        break;
    case 5:
        SetConstantForce((USB_FFBReport_SetConstantForce_Output_Data_t*)data, &g_EffectStates[effectId]);  // Set constant force
        break;
    case 6:
        SetRampForce((USB_FFBReport_SetRampForce_Output_Data_t*)data, &g_EffectStates[effectId]);  // Set ramp force
        break;
    case 7:
        SetCustomForceData((USB_FFBReport_SetCustomForceData_Output_Data_t*)data);  // Set custom force data
        break;
    case 8:
        SetDownloadForceSample((USB_FFBReport_SetDownloadForceSample_Output_Data_t*)data);  // Set download force sample
        break;
    case 9:
        // No operation for this report ID
        break;
    case 10:
        EffectOperation((USB_FFBReport_EffectOperation_Output_Data_t*)data);  // Perform effect operation
        break;
    case 11:
        BlockFree((USB_FFBReport_BlockFree_Output_Data_t*)data);  // Free effect block
        break;
    case 12:
        DeviceControl((USB_FFBReport_DeviceControl_Output_Data_t*)data);  // Control device
        break;
    case 13:
        DeviceGain((USB_FFBReport_DeviceGain_Output_Data_t*)data);  // Set device gain
        break;
    case 14:
        SetCustomForce((USB_FFBReport_SetCustomForce_Output_Data_t*)data);  // Set custom force
        break;
    default:
        break;  // No action for unknown report IDs
    }
}

// Get the PID pool report data
uint8_t* PIDReportHandler::getPIDPool()
{
    FreeAllEffects();  // Free any previously allocated effects

    pidPoolReport.reportId = 7;  // Set the report ID for the PID pool
    pidPoolReport.ramPoolSize = MEMORY_SIZE;  // Set the size of the RAM pool
    pidPoolReport.maxSimultaneousEffects = MAX_EFFECTS;  // Set the max simultaneous effects allowed
    pidPoolReport.memoryManagement = 3;  // Set memory management value
    return (uint8_t*)& pidPoolReport;  // Return the address of the PID pool report
}

// Get the PID block load report data
uint8_t* PIDReportHandler::getPIDBlockLoad()
{
    return (uint8_t*)& pidBlockLoad;  // Return the address of the PID block load report
}

// Get the PID status report data
uint8_t* PIDReportHandler::getPIDStatus()
{
    return (uint8_t*)& pidState;  // Return the address of the PID status report
}
