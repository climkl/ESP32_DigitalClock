#include "TimeManager.h"
#include "time.h"
#include "Macros.h"

TimeManager::TimeManager(WifiHandler &wifiHandler) : _wifiHandler(wifiHandler)
{
}

void TimeManager::setup(unsigned long NTPfetchFrequency_min)
{
  _NTPfetchFrequency_min = NTPfetchFrequency_min;
}

void TimeManager::start()
{
  fetchNTPTime();
}

void TimeManager::fetchNTPTime()
{
  if (_wifiHandler.isConnected())
  {
    onWifiConnected();
  }
  else
  {
    DEBUG_PRINTLN("Connecting Wifi...");
    _status = NetworkStatus::ConnectingWifi;
    _wifiHandler.Connect();
  }
}

void TimeManager::onWifiConnected()
{
  // https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-time.c
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // this configures the local clock to read from ntp server
  _status = NetworkStatus::GettingTime;
  _lastGetTimeRetry = millis() + GETTIME_INITIAL_INTERVAL_MS;
}

void TimeManager::tryInitializeTime()
{
  // struct tm timeinfo;

  if (!getLocalTime_Single(&_timeInfo))
  {
    // initially, it can take really quite like 5+ seconds
    DEBUG_PRINTLN("GetTime: Failed to get local time, will retry.");
    return;
  }
  else
  {
#if DEBUG
    Serial.println(&_timeInfo, "%A, %B %d %Y %H:%M:%S"); // Tuesday, October 21 2025 15:35:47
#endif
  }

  // disconnect since we have retrieved what we wanted
  // WiFi.disconnect(true, false);
  // WiFi.mode(WIFI_OFF);

  if (_allowDisconnect)
  {
    _wifiHandler.Disconnect();
  }

  _status = NetworkStatus::None;
  _RTCInitialized = true;
  _lastNTPFetch = millis();
}

// modified version of
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-time.c
// to remove blocking delay >:/ which interferes with display loop
// we'll repoll in our loop
bool TimeManager::getLocalTime_Single(struct tm *info)
{
  // uint32_t start = millis();
  time_t now;
  // while ((millis() - start) <= ms) {
  time(&now);
  localtime_r(&now, info);
  // tm_year is years since 1900
  if (info->tm_year > (2024 - 1900))
  {
    return true;
  }
  // delay(10);
  //}
  return false;
}

// public version of tryGetTime that can be called once the time has been initially set in the RTC
tm *TimeManager::getTime()
{
  if (!_RTCInitialized)
    return nullptr;

  if (!getLocalTime_Single(&_timeInfo))
  {
    // first time after uploading new sketch, it can take really quite like 5+ seconds, since internal RTC is empty
    DEBUG_PRINTLN("GetTime: Failed to get local time.");
    return nullptr;
  }

  return &_timeInfo;
}

void TimeManager::loop()
{
  switch (_status)
  {
  case NetworkStatus::ConnectingWifi:
    if (_wifiHandler.isConnected()) // WiFi.status() == WL_CONNECTED)
    {
      DEBUG_PRINTLN("Wifi Connected");
      onWifiConnected();
    }
    break;
  case NetworkStatus::GettingTime:
  {
    long milli = millis();
    if (milli - _lastGetTimeRetry > RETRY_GETTIME_INTERVAL_MS)
    {
      _lastGetTimeRetry = milli;
      tryInitializeTime();
    }
  }
  break;
  case NetworkStatus::None:
  {
    long milli = millis();
    if (milli - _lastNTPFetch > _NTPfetchFrequency_min * 60UL * 1000UL)
    {
      DEBUG_PRINTLN("Period time fetch from NTP");
      fetchNTPTime();
    }
  }
  break;
  }
}

void TimeManager::setAllowDisconnect(bool allow)
{
  _allowDisconnect = allow;
  if (_allowDisconnect &&_status == NetworkStatus::None)
  {
    // time manager doesn't need it
    _wifiHandler.Disconnect();
  }
}