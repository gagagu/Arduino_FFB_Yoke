# Arduino_FFB_Yoke
Flightsim FFB Yoke with Arduino Leonardo

### use at your own risk. 
I am not responsible for any damage to man and machine. Be careful when handling electronics and mechanics.
### It is not allowed to use my work in a commercial way!
If you want to use it in an commercial way please contact me.

## New Hardware and new PCB Verison 2.0
The new Hardware is using an Arduino Pro Micro instead of original Arduino Micro because it's cheaper and easier to order.
The new Hardware created by JR4 is working with the old PCB V1.3.5 and the new PCB V2.0

The new Hardware is available [here](https://www.thingiverse.com/thing:6464701)

I have done some remixed with different planetary gears and other stuff [here](https://www.thingiverse.com/thing:6786283)

For the new Code with automatic calibration switch the branch to "2.0" but be careful, it's in alpha state and I am not responsible for any damage to man and machine. Be careful when handling electronics and mechanics.

To open the Code in Arduino Studi you have to place it into a directory called "Arduino_FFB_Yoke"!

You can use the code with board v2.0 and board v1.3.5 together. You only have to switch in defines.h file.


### This project is still in development and will be extend constantly.

It shows how to create a force feedback yoke (and later pedals) for using with Flight Simulator.
It consist of 3d components, electronic schematics and arduino source code.

See [Wiki](https://github.com/gagagu/Arduino_FFB_Yoke/wiki) for more info.
### Please read the [Issues](https://github.com/gagagu/Arduino_FFB_Yoke/wiki/Issues-and-ToDo) before begin!

## Discord
You are welcome to join my Discord to get newest Info and support, [here](https://discord.gg/zrYUcJWu7F)

## Important Info
If you want to use more than one Arduino Joysticks you have to change the Joystick.h file JOYSTICK_DEFAULT_REPORT_ID number to different number
Example:

#### #define JOYSTICK_DEFAULT_REPORT_ID         0x04

Also you can define the USB Name of the Joystick by changing the boards.txt file normally found in
#### C:\Program Files (x86)\Arduino\hardware\arduino\avr\boards.txt
#### or
#### C:\Users\your username\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.5\boards.txt

#### find:
micro.build.vid=0x2341

micro.build.pid=0x8037

micro.build.usb_product="Arduino Micro"

#### and change it to (use at your own risk), you can comment out the old value by using a Rhombus symbol
micro.build.vid=0x9998

micro.build.pid=0x9998

micro.build.usb_product="Arduino FFB Yoke"

## Calibration
The newest Version of the code uses a calibration routine for measure the Potentiometer Values on all directions. I will be started by pressing the calibration button. Everything you need to know will be displayed on the LCD Screen.

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
