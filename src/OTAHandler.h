#pragma once

#include <WiFiHandler.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ElegantOTA.h>

/// @brief Layer around ElegantOTA
class OTAHandler
{
    static const uint16_t OTA_PORT = 80;

    /// @brief How long to keep server open for uploading. In case we forget to switch it off.
    static const unsigned long OTA_SERVER_TIMEOUT_MIN = 30; // 0

public:
    OTAHandler(WifiHandler &wifiHandler);

    /// @brief Begin OTA mode for device. Connects to Wifi if required.
    void start();
    void loop();
    void end();

    bool isActive() const { return _active; }

    void serverEndSetListener(void (*cb)());

private:
    AsyncWebServer _server;
    WifiHandler &_wifiHandler;

    bool _active;
    // bool _serverStarted = false;

    /// @brief Millisecond.
    unsigned long _serverStartTime = 0;
    bool hasServerStarted() const { return _serverStartTime != 0; }

    void (*serverEndCb)() = nullptr; //single listener for now

    void startServer();
};