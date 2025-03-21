/* 
 Created by A.Eckers aka Gagagu
 http://www.gagagu.de
 https://github.com/gagagu/Arduino_FFB_Yoke
 https://www.youtube.com/@gagagu01
*/

/*
  This repository contains code for Arduino projects. 
  The code is provided "as is," without warranty of any kind, either express or implied, 
  including but not limited to the warranties of merchantability, 
  fitness for a particular purpose, or non-infringement. 
  The author(s) make no representations or warranties about the accuracy or completeness of 
  the code or its suitability for your specific use case.

  By using this code, you acknowledge and agree that you are solely responsible for any 
  consequences that may arise from its use. 

  For DIY projects involving electronic and electromechanical moving parts, caution is essential. 
  Ensure that you take the appropriate safety precautions, particularly when working with electricity. 
  Only work with devices if you understand their functionality and potential risks, and always wear 
  appropriate protective equipment. 
  Make sure you are working in a safe, well-lit environment, and that all components are properly installed and secured to avoid injury or damage.

  Special caution is required when building a force feedback device. Unexpected or sudden movements may occur, 
  which could lead to damage to people or other objects. 
  Ensure that all mechanical parts are securely mounted and that the work area is free of obstacles.
  
  By using this project, you acknowledge and agree that you are solely responsible for any consequences that may arise from its use. 
  The author(s) will not be held liable for any damages, injuries, or issues arising from the use of the project, 
  including but not limited to malfunctioning hardware, electrical damage, personal injury, or damage caused by 
  unintended movements of the force feedback device. The responsibility for proper handling, installation, 
  and use of the devices and components lies with the user.
  
  Use at your own risk.
*/

/******************************************
  clear all data in eeprom
*******************************************/
void CMD_WRITE_EEPROM_CLEAR() {
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
} //CMD_WRITE_EEPROM_CLEAR

/******************************************
  write all settings to eeprom
*******************************************/
void CMD_WRITE_DATA_EEPROM() {
  // start address
  int eeAddress = EEPROM_DATA_INDEX;

  // force and pwm
  WriteEepromInt16Array(eeAddress, adjForceMax, MEM_AXES);
  WriteEepromByteArray(eeAddress, adjPwmMin, MEM_AXES);
  WriteEepromByteArray(eeAddress, adjPwmMax, MEM_AXES);

  //write effects
  for (int i = 0; i < MEM_AXES; i++) {
    EEPROM.put(eeAddress, effects[i]);
    eeAddress += sizeof(EffectParams);
  }
  // write gains
  for (int i = 0; i < MEM_AXES; i++) {
    EEPROM.put(eeAddress, gains[i]);
    eeAddress += sizeof(Gains);
  }

  // set flag to indicate that data is available
  EEPROM.update(EEPROM_DATA_AVAILABLE_INDEX, 1);
} //CMD_WRITE_DATA_EEPROM

/******************************************
  returns value from available address of eeprom
  used to detect if data is available
*******************************************/
byte IsEepromDataAvailable() {
  return EEPROM.read(EEPROM_DATA_AVAILABLE_INDEX);
} //IsEepromDataAvailable

/******************************************
  read all settings from eeprom
*******************************************/
void ReadDataFromEeprom()
{
  // start address
  int eeAddress = EEPROM_DATA_INDEX;

  // read data  
  ReadEepromInt16Array(eeAddress, adjForceMax, MEM_AXES);
  ReadEepromByteArray(eeAddress, adjPwmMin, MEM_AXES);
  ReadEepromByteArray(eeAddress, adjPwmMax, MEM_AXES);

  //read effects
  for (int i = 0; i < MEM_AXES; i++) {
    EEPROM.get(eeAddress, effects[i]);
    eeAddress += sizeof(EffectParams);
  }
  // read gains
  for (int i = 0; i < MEM_AXES; i++) {
    EEPROM.get(eeAddress, gains[i]);
    eeAddress += sizeof(Gains);
  }

} //ReadDataFromEeprom

/******************************************
  writes an byte array to the eeprom
*******************************************/
void WriteEepromByteArray(int &myAddress, byte myValues[], byte arraySize) {
  for (int i = 0; i < arraySize; i++) {
    EEPROM.put(myAddress, myValues[i]);
    myAddress += sizeof(byte);
  }
} //WriteEepromByteArray

/******************************************
  reads an byte array  from eeprom
*******************************************/
void ReadEepromByteArray(int &myAddress, byte myArray[], byte arraySize) {
  for (int i = 0; i < arraySize; i++) {
    EEPROM.get(myAddress, myArray[i]);
    myAddress += sizeof(byte);
  }
} //ReadEepromByteArray

/******************************************
  writes an int16_t array to the eeprom
*******************************************/
void WriteEepromInt16Array(int &myAddress, int16_t myValues[], byte arraySize) {
  for (int i = 0; i < arraySize; i++) {
    EEPROM.put(myAddress, myValues[i]);
    myAddress += sizeof(int16_t);
  }
} //WriteEepromInt16Array

/******************************************
  reads an int16_t array  from eeprom
*******************************************/
void ReadEepromInt16Array(int &myAddress, int16_t myArray[], byte arraySize) {
  for (int i = 0; i < arraySize; i++) {
    EEPROM.get(myAddress, myArray[i]);
    myAddress += sizeof(int16_t);
  }
} //ReadEepromInt16Array

/******************************************
  writes an int16_t value to the eeprom
*******************************************/
void WriteEepromInt16(int &myAddress, int16_t myValue) {
    EEPROM.put(myAddress, myValue);
    myAddress += sizeof(int16_t);
} //WriteEepromInt16

/******************************************
  reads an int16_t value  from eeprom
*******************************************/
void ReadEepromInt16(int &myAddress, int16_t myValue) {
    EEPROM.get(myAddress, myValue);
    myAddress += sizeof(int16_t);
} //WriteEepromInt16
