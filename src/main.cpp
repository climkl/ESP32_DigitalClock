#include "FourDigit7Segment.h"
#include "TimeManager.h"
#include "Macros.h"
#include "OTAHandler.h"
#include "DebouncedButton.h"

// Pin defines
//  const uint8_t data = 23; //serial out from esp32 to 74HC595 SER
//  const uint8_t latch = 22; //74HC595 RCLK
//  const uint8_t shiftClock = 19; //47HC595 SRCLK

// careful cos some pins on esp32 are special or e.g. output only
const uint8_t digitPins[4] = {32, 33, 25, 26};               // 12, 9, 8, 6 of display
const uint8_t segmentPins[8] = {19, 18, 5, 17, 16, 4, 0, 2}; // 11, 7, 4, 2, 1, 10, 5, 3 of display, ordered A-G+DP
const uint8_t otaBtnPin = 13;

FourDigit7Segment _segDisplay;
WifiHandler _wifiHandler;
TimeManager _timeManager(_wifiHandler);
OTAHandler _otaHandler(_wifiHandler);
DebouncedButton _otaButton;

uint16_t _lastMinute = 99;
// test code
//  int lastTime = 0;
//  int testInt = 0;
bool _initialized = false;
static bool _processOTAend = false;

void onOTAServerEnd()
{
  DEBUG_PRINTLN("OTA Server Ended");
  _processOTAend = true;
}

void setup()
{
#if DEBUG
  Serial.begin(9600);
#endif

  _otaButton.setup(otaBtnPin);
  _segDisplay.setup(digitPins, segmentPins, 4, true);
  // _segDisplay.displayNumber(5555, 0);

  _wifiHandler.setup();

#if DEBUG
  _timeManager.setup(1);
#else
  _timeManager.setup(60 * 24); // once a day
#endif

  _wifiHandler.Connect();
  _otaHandler.serverEndSetListener(onOTAServerEnd);
}

void loop()
{
  if (_initialized)
  {
    if (tm *timeInfo = _timeManager.getTime())
    {
      if (_lastMinute != timeInfo->tm_min)
      {
        _lastMinute = timeInfo->tm_min;
        uint16_t timeNumeral = timeInfo->tm_hour * 100 + timeInfo->tm_min; // e.g. 1530

        // Serial.print("Updating time display: ");
        // Serial.println(timeNumeral);
        _segDisplay.displayNumber(timeNumeral, 2);
      }
    }
  }
  else
  {
    if (_wifiHandler.isConnected())
    {
      _timeManager.start();
      _initialized = true;
    }
  }

  BtnEvent btnEvent = _otaButton.update();
  if (btnEvent == BtnEvent::Down)
  {
    DEBUG_PRINTLN("OTA Button Pressed!");
    // toggle ota state
    if (_otaHandler.isActive())
    {
      _otaHandler.end();
      //_timeManager.setAllowDisconnect(true); // allow wifi off if not needed
    }
    else
    {
      _timeManager.setAllowDisconnect(false); // keep wifi on during ota
      _otaHandler.start();
    }
  }

  if (_processOTAend)
  {
    _timeManager.setAllowDisconnect(true); //allow wifi to turn off to save power
    _processOTAend = false;
  }

  _timeManager.loop();
  _segDisplay.refreshDisplay();
  _otaHandler.loop();

  // Test Code
  //  if (millis() - lastTime > 1000) {
  //    lastTime = millis();
  //    _segDisplay.displayNumber(testInt);
  //    testInt++;
  //    Serial.print("number: ");
  //    Serial.println(testInt);
  //  }

  // _segDisplay.refreshDisplay();
  // _segDisplay.segmentPinTest();
}