/* 
 Created by A.Eckers aka Gagagu
 http://www.gagagu.de
 https://github.com/gagagu/Arduino_FFB_Yoke
 https://www.youtube.com/@gagagu01
*/

/******************************************
  Clear all data in EEPROM
*******************************************/
void CMD_WRITE_EEPROM_CLEAR() {
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
}

/******************************************
  Write all settings to EEPROM
*******************************************/
void CMD_WRITE_DATA_EEPROM() {
  int eeAddress = EEPROM_DATA_INDEX;

  WriteEepromInt16Array(eeAddress, adjForceMax, MEM_AXES);
  WriteEepromByteArray(eeAddress, adjPwmMin,   MEM_AXES);
  WriteEepromByteArray(eeAddress, adjPwmMax,   MEM_AXES);

  for (int i = 0; i < MEM_AXES; i++) {
    EEPROM.put(eeAddress, effects[i]);
    eeAddress += sizeof(EffectParams);
  }
  for (int i = 0; i < MEM_AXES; i++) {
    EEPROM.put(eeAddress, gains[i]);
    eeAddress += sizeof(Gains);
  }

  // Mark that valid data is stored
  EEPROM.update(EEPROM_DATA_AVAILABLE_INDEX, 1);
}

/******************************************
  Returns true if valid data is stored in EEPROM
*******************************************/
byte IsEepromDataAvailable() {
  return EEPROM.read(EEPROM_DATA_AVAILABLE_INDEX);
}

/******************************************
  Read all settings from EEPROM
*******************************************/
void ReadDataFromEeprom() {
  int eeAddress = EEPROM_DATA_INDEX;

  ReadEepromInt16Array(eeAddress, adjForceMax, MEM_AXES);
  ReadEepromByteArray(eeAddress, adjPwmMin,   MEM_AXES);
  ReadEepromByteArray(eeAddress, adjPwmMax,   MEM_AXES);

  for (int i = 0; i < MEM_AXES; i++) {
    EEPROM.get(eeAddress, effects[i]);
    eeAddress += sizeof(EffectParams);
  }
  for (int i = 0; i < MEM_AXES; i++) {
    EEPROM.get(eeAddress, gains[i]);
    eeAddress += sizeof(Gains);
  }
}

/******************************************
  Write a byte array to EEPROM
*******************************************/
void WriteEepromByteArray(int &myAddress, byte myValues[], byte arraySize) {
  for (int i = 0; i < arraySize; i++) {
    EEPROM.put(myAddress, myValues[i]);
    myAddress += sizeof(byte);
  }
}

/******************************************
  Read a byte array from EEPROM
*******************************************/
void ReadEepromByteArray(int &myAddress, byte myArray[], byte arraySize) {
  for (int i = 0; i < arraySize; i++) {
    EEPROM.get(myAddress, myArray[i]);
    myAddress += sizeof(byte);
  }
}

/******************************************
  Write an int16_t array to EEPROM
*******************************************/
void WriteEepromInt16Array(int &myAddress, int16_t myValues[], byte arraySize) {
  for (int i = 0; i < arraySize; i++) {
    EEPROM.put(myAddress, myValues[i]);
    myAddress += sizeof(int16_t);
  }
}

/******************************************
  Read an int16_t array from EEPROM
*******************************************/
void ReadEepromInt16Array(int &myAddress, int16_t myArray[], byte arraySize) {
  for (int i = 0; i < arraySize; i++) {
    EEPROM.get(myAddress, myArray[i]);
    myAddress += sizeof(int16_t);
  }
}

/******************************************
  Write a single int16_t value to EEPROM
*******************************************/
void WriteEepromInt16(int &myAddress, int16_t myValue) {
  EEPROM.put(myAddress, myValue);
  myAddress += sizeof(int16_t);
}

/******************************************
  Read a single int16_t value from EEPROM
  BUG FIX: was passing myValue by value – the caller never received the result.
           Now passes by reference so the read value is returned correctly.
*******************************************/
void ReadEepromInt16(int &myAddress, int16_t &myValue) {
  EEPROM.get(myAddress, myValue);
  myAddress += sizeof(int16_t);
}
