#include "OTAHandler.h"
#include "Macros.h"

OTAHandler::OTAHandler(WifiHandler &wifiHandler) : _server(OTA_PORT), _wifiHandler(wifiHandler)
{
}
void OTAHandler::start() // TODO display does flicker a bit...
{
  DEBUG_PRINTLN("Starting OTA...");
  _active = true;
  if (!_wifiHandler.isConnected())
  {
    DEBUG_PRINTLN("OTA: Wifi not connected, will connect now");
    _wifiHandler.Connect();
  }
  else
  {
    startServer();
  }
}

void OTAHandler::startServer()
{
  // https://randomnerdtutorials.com/esp32-ota-elegantota-arduino/
  DEBUG_PRINTLN("Starting OTA server...");

  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
             { request->send(200, "text/plain", "ClimClock: Version 0.0.2"); }); //TEMP hardcode

  _server.begin();

  ElegantOTA.begin(&_server); // Start ElegantOTA

  _serverStartTime = millis();
}

void OTAHandler::loop()
{
  if (_active)
  {
    if (!hasServerStarted() && _wifiHandler.isConnected())
      startServer();

    if (hasServerStarted())
    {
      ElegantOTA.loop();
      if (millis() - _serverStartTime > OTA_SERVER_TIMEOUT_MIN * 60UL * 1000UL)
      {
        DEBUG_PRINTLN("OTA Server Timeout");
        end();
      }
    }
  }
}

void OTAHandler::end()
{
  _active = false;
  _serverStartTime = 0;

  _server.end();
  DEBUG_PRINTLN("OTA ended.");
  
  if (serverEndCb) serverEndCb();
}

void OTAHandler::serverEndSetListener(void (*cb)())
{
  serverEndCb = cb;
}