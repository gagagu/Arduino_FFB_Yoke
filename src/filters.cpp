#include "filters.h"
#include <math.h>

// BUG FIX (warning): Arduino.h already defines PI, causing -Wmacro-redefinition.
// Use TWO_PI from Arduino.h instead, or guard the definition.
#ifndef PI
  #define PI 3.14159265358979323846f
#endif

LowPassFilter::LowPassFilter():
    output(0),
    ePow(0) {}

LowPassFilter::LowPassFilter(float iCutOffFrequency, float iDeltaTime):
    output(0),
    ePow(1.0f - expf(-iDeltaTime * 2.0f * PI * iCutOffFrequency))
{}

float LowPassFilter::update(float input) {
    return output += (input - output) * ePow;
}

float LowPassFilter::update(float input, float deltaTime, float cutoffFrequency) {
    reconfigureFilter(deltaTime, cutoffFrequency);
    return output += (input - output) * ePow;
}

void LowPassFilter::reconfigureFilter(float deltaTime, float cutoffFrequency) {
    ePow = 1.0f - expf(-deltaTime * 2.0f * PI * cutoffFrequency);
}
