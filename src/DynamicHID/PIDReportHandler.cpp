#include "PIDReportHandler.h"

// USB HID PID operation codes (USB Device Class Specification for PID, section 7)
#define PID_OP_START      1
#define PID_OP_START_SOLO 2
#define PID_OP_STOP       3

PIDReportHandler::PIDReportHandler()
{
    nextEID     = 1;
    deviceState = MDEVICESTATE_SPRING;
}

PIDReportHandler::~PIDReportHandler()
{
    FreeAllEffects();
}

// Returns the next free effect ID (1-based) or 0 when all slots are taken.
//
// BUG FIX: original check was `nextEID == MAX_EFFECTS` which returned 0 when
// nextEID reached 14, even though slot 14 is valid (array is MAX_EFFECTS+1).
// Fixed to `> MAX_EFFECTS` so the last slot is actually usable.
uint8_t PIDReportHandler::GetNextFreeEffect(void)
{
    if (nextEID > MAX_EFFECTS)
        return 0;

    uint8_t id = nextEID++;

    // Advance nextEID past any already-allocated slots
    while (nextEID <= MAX_EFFECTS && g_EffectStates[nextEID].state != 0)
        nextEID++;

    g_EffectStates[id].state = MEFFECTSTATE_ALLOCATED;
    pidState.effectBlockIndex = id;
    return id;
}

void PIDReportHandler::StopAllEffects(void)
{
    for (uint8_t id = 0; id < MAX_EFFECTS; id++)
        StopEffect(id);
}

void PIDReportHandler::StartEffect(uint8_t id)
{
    if (id > MAX_EFFECTS) return;
    g_EffectStates[id].state       = MEFFECTSTATE_PLAYING;
    g_EffectStates[id].elapsedTime = 0;
    g_EffectStates[id].startTime   = (uint32_t)millis();
}

// BUG FIX: ramPoolAvailable was incremented unconditionally, even for effects
// that were never allocated. This caused the reported pool size to exceed
// MEMORY_SIZE. Now only allocated effects return their memory on stop.
void PIDReportHandler::StopEffect(uint8_t id)
{
    if (id > MAX_EFFECTS) return;
    if (g_EffectStates[id].state & MEFFECTSTATE_ALLOCATED) {
        pidBlockLoad.ramPoolAvailable += SIZE_EFFECT;
    }
    g_EffectStates[id].state &= ~MEFFECTSTATE_PLAYING;
}

void PIDReportHandler::FreeEffect(uint8_t id)
{
    if (id > MAX_EFFECTS) return;
    g_EffectStates[id].state = 0;
    if (id < nextEID)
        nextEID = id;
}

void PIDReportHandler::FreeAllEffects(void)
{
    nextEID = 1;
    memset((void*)&g_EffectStates, 0, sizeof(g_EffectStates));
    pidBlockLoad.ramPoolAvailable = MEMORY_SIZE;
}

// Handles start / start-solo / stop operations.
// BUG FIX (readability): replaced magic numbers 1/2/3 with named constants.
void PIDReportHandler::EffectOperation(USB_FFBReport_EffectOperation_Output_Data_t* data)
{
    volatile TEffectState& effect = g_EffectStates[data->effectBlockIndex];
    effect.loopCount = data->loopCount;

    switch (data->operation)
    {
        case PID_OP_START:
            if (data->loopCount == 0xFF) {
                effect.totalDuration = USB_DURATION_INFINITE;
            } else if (data->loopCount > 0) {
                effect.totalDuration = (effect.startDelay + effect.duration) * data->loopCount;
            }
            StartEffect(data->effectBlockIndex);
            break;

        case PID_OP_START_SOLO:
            StopAllEffects();
            StartEffect(data->effectBlockIndex);
            break;

        case PID_OP_STOP:
            StopEffect(data->effectBlockIndex);
            break;
    }
}

void PIDReportHandler::BlockFree(USB_FFBReport_BlockFree_Output_Data_t* data)
{
    uint8_t eid = data->effectBlockIndex;
    if (eid == 0xFF)
        FreeAllEffects();
    else
        FreeEffect(eid);
}

void PIDReportHandler::DeviceControl(USB_FFBReport_DeviceControl_Output_Data_t* data)
{
    switch (data->control)
    {
        case 0x01: pidState.status |= 2;                          break; // Enable actuators
        case 0x02: pidState.status &= ~0x02;                      break; // Disable actuators
        case 0x03: StopAllEffects(); deviceState &= ~MDEVICESTATE_SPRING; break; // Stop all
        case 0x04: FreeAllEffects(); deviceState |= MDEVICESTATE_SPRING;  break; // Reset
        case 0x05: deviceState |= MDEVICESTATE_PAUSED;            break; // Pause
        case 0x06: deviceState &= ~MDEVICESTATE_PAUSED;           break; // Continue
        default:                                                   break;
    }
}

void PIDReportHandler::DeviceGain(USB_FFBReport_DeviceGain_Output_Data_t* data)
{
    deviceGain.gain = data->gain;
}

void PIDReportHandler::SetCustomForce(USB_FFBReport_SetCustomForce_Output_Data_t* data) {}
void PIDReportHandler::SetCustomForceData(USB_FFBReport_SetCustomForceData_Output_Data_t* data) {}
void PIDReportHandler::SetDownloadForceSample(USB_FFBReport_SetDownloadForceSample_Output_Data_t* data) {}

void PIDReportHandler::SetEffect(USB_FFBReport_SetEffect_Output_Data_t* data)
{
    volatile TEffectState* effect = &g_EffectStates[data->effectBlockIndex];
    effect->duration  = data->duration;
    for (int i = 0; i < FFB_AXIS_COUNT; ++i)
        effect->direction[i] = data->direction[i];
    effect->effectType  = data->effectType;
    effect->gain        = data->gain;
    effect->enableAxis  = data->enableAxis;
    effect->startDelay  = data->startDelay;

    if (effect->loopCount != 0xFF) {
        uint8_t lc = effect->loopCount > 0 ? effect->loopCount : 1;
        effect->totalDuration = (data->duration + data->startDelay) * lc;
    }
}

void PIDReportHandler::SetEnvelope(USB_FFBReport_SetEnvelope_Output_Data_t* data, volatile TEffectState* effect)
{
    effect->attackLevel = data->attackLevel;
    effect->fadeLevel   = data->fadeLevel;
    effect->attackTime  = data->attackTime;
    effect->fadeTime    = data->fadeTime;
}

void PIDReportHandler::SetCondition(USB_FFBReport_SetCondition_Output_Data_t* data, volatile TEffectState* effect)
{
    uint8_t axis = data->parameterBlockOffset;
    if (axis >= effect->conditionReportsCount)
        effect->conditionReportsCount = axis + 1;

    effect->conditions[axis].cpOffset            = data->cpOffset;
    effect->conditions[axis].positiveCoefficient = data->positiveCoefficient;
    effect->conditions[axis].negativeCoefficient = data->negativeCoefficient;
    effect->conditions[axis].positiveSaturation  = data->positiveSaturation;
    effect->conditions[axis].negativeSaturation  = data->negativeSaturation;
    effect->conditions[axis].deadBand            = data->deadBand;
}

void PIDReportHandler::SetPeriodic(USB_FFBReport_SetPeriodic_Output_Data_t* data, volatile TEffectState* effect)
{
    effect->magnitude = data->magnitude;
    effect->offset    = data->offset;
    effect->phase     = data->phase;
    effect->period    = data->period;
}

void PIDReportHandler::SetConstantForce(USB_FFBReport_SetConstantForce_Output_Data_t* data, volatile TEffectState* effect)
{
    effect->magnitude = data->magnitude;
}

void PIDReportHandler::SetRampForce(USB_FFBReport_SetRampForce_Output_Data_t* data, volatile TEffectState* effect)
{
    effect->startMagnitude = data->startMagnitude;
    effect->endMagnitude   = data->endMagnitude;
}

void PIDReportHandler::CreateNewEffect(USB_FFBReport_CreateNewEffect_Feature_Data_t* inData)
{
    pidBlockLoad.reportId         = 6;
    pidBlockLoad.effectBlockIndex = GetNextFreeEffect();

    if (pidBlockLoad.effectBlockIndex == 0) {
        pidBlockLoad.loadStatus = 2;  // Full
    } else {
        pidBlockLoad.loadStatus = 1;  // Success
        volatile TEffectState* effect = &g_EffectStates[pidBlockLoad.effectBlockIndex];
        memset((void*)effect, 0, sizeof(TEffectState));
        effect->state = MEFFECTSTATE_ALLOCATED;
        pidBlockLoad.ramPoolAvailable -= SIZE_EFFECT;
    }
}

void PIDReportHandler::UppackUsbData(uint8_t* data, uint16_t len)
{
    uint8_t effectId = data[1];  // effectBlockIndex is always the second byte
    switch (data[0])             // reportID
    {
        case  1: SetEffect       ((USB_FFBReport_SetEffect_Output_Data_t*)       data);                    break;
        case  2: SetEnvelope     ((USB_FFBReport_SetEnvelope_Output_Data_t*)     data, &g_EffectStates[effectId]); break;
        case  3: SetCondition    ((USB_FFBReport_SetCondition_Output_Data_t*)    data, &g_EffectStates[effectId]); break;
        case  4: SetPeriodic     ((USB_FFBReport_SetPeriodic_Output_Data_t*)     data, &g_EffectStates[effectId]); break;
        case  5: SetConstantForce((USB_FFBReport_SetConstantForce_Output_Data_t*)data, &g_EffectStates[effectId]); break;
        case  6: SetRampForce    ((USB_FFBReport_SetRampForce_Output_Data_t*)    data, &g_EffectStates[effectId]); break;
        case  7: SetCustomForceData     ((USB_FFBReport_SetCustomForceData_Output_Data_t*)      data); break;
        case  8: SetDownloadForceSample ((USB_FFBReport_SetDownloadForceSample_Output_Data_t*)  data); break;
        case  9: break;  // reserved
        case 10: EffectOperation ((USB_FFBReport_EffectOperation_Output_Data_t*) data); break;
        case 11: BlockFree       ((USB_FFBReport_BlockFree_Output_Data_t*)       data); break;
        case 12: DeviceControl   ((USB_FFBReport_DeviceControl_Output_Data_t*)  data); break;
        case 13: DeviceGain      ((USB_FFBReport_DeviceGain_Output_Data_t*)     data); break;
        case 14: SetCustomForce  ((USB_FFBReport_SetCustomForce_Output_Data_t*) data); break;
        default: break;
    }
}

uint8_t* PIDReportHandler::getPIDPool()
{
    FreeAllEffects();
    pidPoolReport.reportId              = 7;
    pidPoolReport.ramPoolSize           = MEMORY_SIZE;
    pidPoolReport.maxSimultaneousEffects = MAX_EFFECTS;
    pidPoolReport.memoryManagement      = 3;
    return (uint8_t*)&pidPoolReport;
}

uint8_t* PIDReportHandler::getPIDBlockLoad() { return (uint8_t*)&pidBlockLoad; }
uint8_t* PIDReportHandler::getPIDStatus()    { return (uint8_t*)&pidState;     }
