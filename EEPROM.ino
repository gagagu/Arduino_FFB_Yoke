
#define EEPROM_DATA_AVAILABLE_INDEX 0     // eeprom address to indicate data available
                                          // space between reserved
#define EEPROM_DATA_INDEX 10              // eeprom start address for data

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
void readEepromInt16(int &myAddress, int16_t myValue) {
    EEPROM.get(myAddress, myValue);
    myAddress += sizeof(int16_t);
} //WriteEepromInt16
