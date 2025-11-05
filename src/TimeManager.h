#pragma once

#include "inttypes.h"
#include "time.h"
#include "WiFiHandler.h"

/// @brief In charge of handling fetching time from NTP and updating local clock
class TimeManager {
  
  enum NetworkStatus {
    None,
    ConnectingWifi,
    GettingTime,
  };

  const char* ntpServer = "pool.ntp.org";
  const long gmtOffset_sec = 28800;
  const int daylightOffset_sec = 0;

  /// @brief How long to keep attempt to GetLocalTime(). Milliseconds.
  const static unsigned long RETRY_GETTIME_INTERVAL_MS = 1000;

  /// @brief How long to wait before the first GetLocalTime() query
  constexpr static unsigned long GETTIME_INITIAL_INTERVAL_MS = 2000 - RETRY_GETTIME_INTERVAL_MS;


  public: 
    TimeManager(WifiHandler &wifiHandler);

    void setup(unsigned long NTPfetchFrequency_min);
    void start();
    void loop();
    
    /// @brief Poll time
    /// @return can be null if time not yet initialized 
    tm* getTime();

    void setAllowDisconnect(bool allow);

  private:
    NetworkStatus _status = NetworkStatus::None;
    /// @brief True after the first successful get of time.
    bool _RTCInitialized = false;
    tm _timeInfo;
    WifiHandler &_wifiHandler;
    bool _allowDisconnect = false;

    //fetching from NTP
    unsigned long _NTPfetchFrequency_min = 60*24; //default once a day
    unsigned long _lastNTPFetch = 0;
  
    /// @brief Last attempt for GetLocalTime() initialization
    unsigned long _lastGetTimeRetry = 0;

    /// @brief Trigger a wifi connection to retrieve time from ntp server.
    void fetchNTPTime();
    void onWifiConnected();
    void tryInitializeTime();
    bool getLocalTime_Single(struct tm *info);
};