#pragma once

// Uncomment to enable debug
//#define DEBUG_ENABLED

#ifdef DEBUG_ENABLED

  #define DEBUG_BEGIN(baud) Serial.begin(baud)
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)

#else

  #define DEBUG_BEGIN(baud)
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)

#endif
