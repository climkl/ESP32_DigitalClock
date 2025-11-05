#pragma once
#if DEBUG
  #define DEBUG_PRINT(x)  Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)  do {} while(0) //prevents arguments from being evaluated
  #define DEBUG_PRINTLN(x)  do {} while(0)
#endif