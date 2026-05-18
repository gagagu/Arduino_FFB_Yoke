/*
  debug.h – DBG macro bridge for src/ files
  
  Joystick.cpp and other files in src/ cannot directly include
  ../defines.h because the Arduino build system does not guarantee
  the parent sketch directory is on the include path when compiling
  library sources.

  This header mirrors the DBG_LEVEL macro logic from defines.h so
  that src/ files get the same debug output behaviour without a
  fragile relative-path include.

  Usage: #include "debug.h" at the top of any src/ .cpp file that
  uses DBG1 / DBG2 / DBG3 / DBG_TS.
*/

#ifndef SRC_DEBUG_H
#define SRC_DEBUG_H

// Allow the sketch's defines.h to override the level by defining
// DBG_LEVEL before this header is included (e.g. via compiler -D flag
// or because defines.h was included transitively).
// If nothing is set, default to silent (level 0).

#if defined(DBG_LEVEL) && DBG_LEVEL >= 1
  #ifndef DBG1
    #define DBG1(x)   Serial.print(x)
  #endif
  #ifndef DBG1LN
    #define DBG1LN(x) Serial.println(x)
  #endif
  #ifndef DBG_TS
    #define DBG_TS()  do { Serial.print(F("[")); Serial.print(millis()); Serial.print(F("] ")); } while(0)
  #endif
#else
  #ifndef DBG1
    #define DBG1(x)
  #endif
  #ifndef DBG1LN
    #define DBG1LN(x)
  #endif
  #ifndef DBG_TS
    #define DBG_TS()
  #endif
#endif

#if defined(DBG_LEVEL) && DBG_LEVEL >= 2
  #ifndef DBG2
    #define DBG2(x)   Serial.print(x)
  #endif
  #ifndef DBG2LN
    #define DBG2LN(x) Serial.println(x)
  #endif
#else
  #ifndef DBG2
    #define DBG2(x)
  #endif
  #ifndef DBG2LN
    #define DBG2LN(x)
  #endif
#endif

#if defined(DBG_LEVEL) && DBG_LEVEL >= 3
  #ifndef DBG3
    #define DBG3(x)   Serial.print(x)
  #endif
  #ifndef DBG3LN
    #define DBG3LN(x) Serial.println(x)
  #endif
#else
  #ifndef DBG3
    #define DBG3(x)
  #endif
  #ifndef DBG3LN
    #define DBG3LN(x)
  #endif
#endif

#endif // SRC_DEBUG_H
