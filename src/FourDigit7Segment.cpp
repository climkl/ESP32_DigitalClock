#include "FourDigit7Segment.h"
#include "Arduino.h"

FourDigit7Segment::FourDigit7Segment()
{
}

// void FourDigit7Segment::setup(uint8_t dataPin, uint8_t latchPin, uint8_t shiftClockPin) {
//     _dataPin = dataPin;
//     _latchPin = latchPin;
//     _shiftClockPin = shiftClockPin;

//     pinMode(latchPin,OUTPUT);
//     pinMode(shiftClockPin,OUTPUT);
//     pinMode(dataPin,OUTPUT);
// }

void FourDigit7Segment::setup(const uint8_t digitPins[], const uint8_t segmentPins[SEGMENT_COUNT], uint8_t digitCount, bool allowLeadingZeroes)
{
  _digitCount = digitCount;
  _allowLeadingZeroes = allowLeadingZeroes;

  for (int i = 0; i < digitCount; ++i)
  {
    _digitPins[i] = digitPins[i];
    pinMode(_digitPins[i], OUTPUT);
    digitalWrite(_digitPins[i], DIGIT_OFF);
  }

  for (int i = 0; i < SEGMENT_COUNT; ++i)
  {
    _segmentPins[i] = segmentPins[i];
    pinMode(_segmentPins[i], OUTPUT);
    digitalWrite(_segmentPins[i], SEGMENT_OFF);
  }

  _currentIdx = 0;
}

void FourDigit7Segment::displayNumber(uint16_t number, uint8_t decimalPos)
{
  if (number > pow(10, _digitCount) - 1)
  {
    Serial.print("Error: Number out of range");
    return;
  }

  if (decimalPos > _digitCount - 1)
  {
    Serial.print("Error: Decimal out of range");
    decimalPos = 0;
    // continue but just ignore decimal point
  }

  // Split number into individual digits
  uint8_t digits[4] = {0, 0, 0, 0};
  splitNumber(number, digits, decimalPos);

  // Serial.print("Split: ");
  // Serial.print(digits[0]);
  // Serial.print(digits[1]);
  // Serial.print(digits[2]);
  // Serial.println(digits[3]);

  setDigitCodes(digits, decimalPos);

  // Serial.print("Digits: ");
  // for (int i=0; i < DIGIT_COUNT; i++)
  //   Serial.println(_currentDigitCodes[i], HEX);
}

void FourDigit7Segment::splitNumber(uint16_t number, uint8_t digits[], uint8_t decimalPos)
{
  // e.g. 1234
  digits[0] = (number / 1000) % 10; // 1234 => 1 => 1
  digits[1] = (number / 100) % 10;  // 1234 => 12 => 2
  digits[2] = (number / 10) % 10;   // 1234 => 123 => 3
  digits[3] = number % 10;          // 4

  if (!_allowLeadingZeroes)
  {
    // handling leading zeroes e.g. 0054
    // decimal pos matters because e.g. 0.54 is ok, 00.54 not really.
    // unless we are in accepting leading zero mode - e.g. clock time 00:54 would be midnight
    for (int i = 0; i < _digitCount; ++i)
    {
      if (digits[i] == 0 
        && i != (_digitCount - decimalPos - 1)) 
        digits[i] = BLANK_IDX; // index - later we map the entire thing to digitMap
      else
        break; // not leading alr just exit e.g. 0104 can have 0 in between
    }
  }
}

void FourDigit7Segment::setDigitCodes(const uint8_t digits[], int8_t decimalPos)
{
  for (int i = 0; i < _digitCount; ++i)
  {
    _currentDigitCodes[i] = digitMap[digits[i]];
  }

  if (decimalPos > 0)
  {
    _currentDigitCodes[_digitCount - decimalPos - 1] |= digitMap[DECIMAL_IDX];
  }
}

void FourDigit7Segment::refreshDisplay()
{
  // Multiplexing - we write digits 0-1-2-3 in a sequence across time, u can see it on video camera esp with long refresh intervals, but our eyes are better than that

  // resistors are on digits for my setup - 4 resistors
  unsigned long microsNow = micros();

  if (_isWaitOff)
  {
    if (microsNow - _lastUpdateTime < OFF_WAIT_INTERVAL)
      return;
  }
  else
  {
    if (microsNow - _lastUpdateTime < REFRESH_INTERVAL)
      return;
  }

  _lastUpdateTime = microsNow;

  // segment off and wait - when wait is done, will go straight to segmentOn
  if (_isWaitOff)
  {
    // wait is done
    _isWaitOff = false;
  }
  else
  {
    segmentOff(_currentIdx);

    if (OFF_WAIT_INTERVAL > 0)
    {
      _isWaitOff = true;
      return;
    }
  }

  _currentIdx++;
  if (_currentIdx > SEGMENT_COUNT - 1)
    _currentIdx = 0; // loopy

  segmentOn(_currentIdx);
}

void FourDigit7Segment::segmentOn(uint8_t segIdx)
{
  // turn on the segment and all digit pins that has that segment to display

  digitalWrite(_segmentPins[segIdx], SEGMENT_ON);

  for (int i = 0; i < _digitCount; ++i)
  {
    if (_currentDigitCodes[i] & (1 << segIdx))
    { // e.g. segNum 3 - bitshift 1 by 3 to get 00000100. then AND
      digitalWrite(_digitPins[i], DIGIT_ON);
    }
  }
}

void FourDigit7Segment::segmentOff(uint8_t segIdx)
{
  // opposite of segmentOn, just that we don't need to check, just turn off everything

  digitalWrite(_segmentPins[segIdx], SEGMENT_OFF);

  for (int i = 0; i < _digitCount; ++i)
  {
    digitalWrite(_digitPins[i], DIGIT_OFF);
  }
}

void FourDigit7Segment::segmentPinTest()
{
  displayNumber(8888);
  for (uint8_t seg = 0; seg < SEGMENT_COUNT; ++seg)
  {
    segmentOn(seg);
    delay(1000);
    segmentOff(seg);
    delay(1000);
  }
}
