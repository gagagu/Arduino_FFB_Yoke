# Arduino_FFB_Yoke
Flightsim FFB Yoke with Arduino Leonardo

### use at your own risk. 
I am not responsible for any damage to man and machine. Be careful when handling electronics and mechanics.
### It is not allowed to use my work in a commercial way!
If you want to use it in an commercial way please contact me.
### This project is still in development and will be extend constantly.

It shows how to create a force feedback yoke (and later pedals) for using with Flight Simulator.
It consist of 3d components, electronic schematics and arduino source code.

See [Wiki](https://github.com/gagagu/Arduino_FFB_Yoke/wiki) for more info.
### Please read the [Issues](https://github.com/gagagu/Arduino_FFB_Yoke/wiki/Issues-and-ToDo) before begin!

## Discord
You are welcome to join my Discord to get newest Info and support, [here](https://discord.gg/btbg4JRz)

## Important Info
If you want to use more than one Arduino Joysticks you have to change the Joystick.h file JOYSTICK_DEFAULT_REPORT_ID number to different number
Example:

#### #define JOYSTICK_DEFAULT_REPORT_ID         0x04

Also you can define the USB Name of the Joystick by changing the boards.txt file normally found in
#### C:\Program Files (x86)\Arduino\hardware\arduino\avr\boards.txt

#### find:
leonardo.build.vid=0x2341
leonardo.build.pid=0x8036
leonardo.build.usb_product="Arduino Leonardo"

#### and change it to (use at your own risk)

leonardo.build.vid=0x9998
leonardo.build.pid=0x9998
leonardo.build.usb_product="Arduino FFB Yoke"

## Ref
This project is a combination of several existing github projects and 3d Models from Thingiverse.com

### Github Projects, Thanks a lot for your work!
* [FINO](https://github.com/jmriego/Fino)
* [Heironimus](https://github.com/MHeironimus/ArduinoJoystickLibrary)
* [hoantv](https://github.com/hoantv/VNWheel)
* [YukMingLaw](https://github.com/YukMingLaw/ArduinoJoystickWithFFBLibrary) 
* [arafin](https://github.com/araffin/arduino-robust-serial/)
* [jimmyberg](https://github.com/jimmyberg/LowPassFilter)

### Thingiverse
* [Saitek FFB Yoke](https://www.thingiverse.com/thing:5241628)
* [CadetYoke v2.0](https://www.thingiverse.com/thing:4884092)
