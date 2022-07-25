// gerneral debug info (forces)
#define NODEBUG
// Button Debug Info
#define NOBUTTONDEBUG
// Poti Debug Info for Serial Plotter
#define NOPOTIDEBUG

#ifdef _VARIANT_ARDUINO_DUE_X_
  #define Serial SerialUSB
#endif

// the digits mean Mmmmrrr (M=Major,m=minor,r=revision)
#define SKETCH_VERSION 3000001

#include "src/Joystick.h"
#include "config.h"


// -------------------------
// Various global variables
// -------------------------
unsigned long lastEffectsUpdate;
unsigned long nextJoystickMillis;
unsigned long nextEffectsMillis;

// --------------------------
// Joystick related variables
// --------------------------
#define JOYSTICK_minX -32768
#define JOYSTICK_maxX 32767
#define JOYSTICK_minY -32768
#define JOYSTICK_maxY 32767

bool is_connected = false;
bool forces_requested = false;
bool pos_updated = false;

int16_t pos[2] = {0, 0};
int lastX;
int lastY;
int lastVelX;
int lastVelY;
int lastAccelX;
int lastAccelY;

EffectParams effects[2];
int32_t forces[2] = {0, 0};

Joystick_ Joystick(
    JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
    12, 1, // Button Count, Hat Switch Count
    true, true, false, // X, Y, Z
    false, false, false, // Rx, Ry, Rz
    false, false); // rudder, throttle

void setup() {

    ArduinoSetup();
    setupJoystick();

    // setup communication
    #if defined(COMINO) 
      || defined(DEBUG)
      || defined(BUTTONDEBUG)
      || defined(POTIDEBUG)
      Serial.begin(SERIAL_BAUD);
    #endif

    // setup timing and run them as soon as possible
    lastEffectsUpdate = 0;
    nextJoystickMillis = 0;
    nextEffectsMillis = 0;
}


void loop(){
    unsigned long currentMillis;
    currentMillis = millis();
    // do not run more frequently than these many milliseconds
    if (currentMillis >= nextJoystickMillis) {
        updateJoystickPos();
        updateJoystickButtons();
        nextJoystickMillis = currentMillis + 2;

        // we calculate condition forces every 100ms or more frequently if we get position updates
        if (currentMillis >= nextEffectsMillis || pos_updated) {
            updateEffects(true);
            nextEffectsMillis = currentMillis + 100;
            pos_updated = false;
        } else {
            // calculate forces without recalculating condition forces
            // this helps having smoother spring/damper/friction
            // if our update rate matches our input device
            updateEffects(false);
        }

    }

    DriveMotors();
    
    Serial.println("");
}
