#pragma once

#include "inttypes.h"
#include "time.h"
#include <WiFiManager.h>
#include <WiFi.h>

class WifiHandler
{
    /// @brief How long to attempt wifi connection before starting AP mode. Should be long enough to attempt, but short enough to quickly fire AP so user is not waiting.
    const static unsigned long PRE_AP_TIMEOUT_SEC = 15;

    /// @brief How long to keep AP portal open. Should be long enough for them to fumble through the menus.
    const static unsigned long AP_TIMEOUT_SEC = 60 * 5;

public:
    WifiHandler() {}
    void setup();

    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }

    void Disconnect();
    void Connect();

private:
    // credentials
    String _wifiSSID = "";
    String _wifiPass = "";

    bool _isConnected = false;
    bool _APdone = false;

    void ConnectAP();
    static void configModeCallback(WiFiManager *wifiManager);
};
