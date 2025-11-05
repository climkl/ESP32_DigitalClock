#pragma once

#include "inttypes.h"
#include "Arduino.h"

/// @brief generic handler layer
class FourDigit7Segment {
  /// @brief Mapping 0-9 to the LED bits
  /// @brief Where goes from g f e d c b a
  const unsigned char digitMap[18] = {
    0x3f, //0
    0x06, //1
    0x5b, //2
    0x4f, //3
    0x66, //4
    0x6d, //5
    0x7d, //6
    0x07, //7
    0x7f, //8
    0x6f, //9
    0x77, //A //10
    0x7c, //b //11
    0x39, //c //12
    0x5e, //d //13
    0x79, //e //14
    0x71, //f //15
    0x00,  //none - 16
    0x80, //0x10000000 MSB for decimal point - 17
  };

  /// @brief index of digit map corresponding to blank
  static const uint8_t BLANK_IDX = 16;
  static const uint8_t DECIMAL_IDX = 17;

  //for now just a constant, but should be eventually configurable depending on hardware - mine is common cathode, so LOW = ON for digits
  static const uint8_t DIGIT_OFF = HIGH;
  static const uint8_t DIGIT_ON = LOW;
  static const uint8_t SEGMENT_OFF = LOW;
  static const uint8_t SEGMENT_ON = HIGH;

  /// @brief microseconds. about 2000 is ok for human eyes, but very visible on video camera
  static const unsigned long REFRESH_INTERVAL = 2000;
  /// @brief microseconds. very short time we turn off, which helps with ghosting. 
  static const unsigned long OFF_WAIT_INTERVAL = 500;
  static const uint8_t SEGMENT_COUNT = 8;
  static const uint8_t MAX_DIGITS = 8; //for buffer

  public:

    FourDigit7Segment();
   
    //void setup(uint8_t dataPin, uint8_t latchPin, uint8_t shiftClockPin, const uint8_t digitPins[4]); //Using serial to parallel component TODO

    /// @brief segment pins should be in order A, B, C, D, E, F, G, DP
    void setup(const uint8_t digitPins[], const uint8_t segmentPins[SEGMENT_COUNT], uint8_t digitCount, bool _allowLeadingZeroes = false);

    /// @brief e.g. 1234 dec 0 = 1234, dec 1 = 123.4, dec 2 = 12.34, dec 3 = 1.234
    void displayNumber(uint16_t number, uint8_t decimalPos = 0);

    /// @brief refresh separately to ensure constant refresh rate. must be called in main loop()
    void refreshDisplay();

    //Tests ========================================================================= 
    /// @brief call this without displayNumber or refreshDisplay in the loop() just to visually test pin connections
    void segmentPinTest();

  private:
    //store pins
    // uint8_t _dataPin;
    // uint8_t _latchPin;
    // uint8_t _shiftClockPin;
    uint8_t _digitPins[MAX_DIGITS];
    uint8_t _segmentPins[SEGMENT_COUNT];
    uint8_t _digitCount;

    bool _allowLeadingZeroes = false;

    /// @brief last time the display was updated
    unsigned long _lastUpdateTime;
    /// @brief if in resistors on digits, this is the segment index since that's what we're iterating
    uint8_t _currentIdx;
    /// @brief we are currently waiting for the off interval before updating again
    bool _isWaitOff;
    
    /// @brief buffer
    uint8_t _currentDigitCodes[MAX_DIGITS];

    /// @brief split a whole number into 4 digits
    void splitNumber(uint16_t number, uint8_t digits[], uint8_t decimalPos);

    /// @brief set from e.g. 1 2 3 4 to the codes in our digitMap. 
    /// @brief writes to our display buffer
    void setDigitCodes(const uint8_t digits[], int8_t decimalPos);

    void segmentOn(uint8_t segIdx);
    void segmentOff(uint8_t segIdx);
};
