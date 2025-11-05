#include "WiFiHandler.h"
#include "Macros.h"

void WifiHandler::setup()
{
}

void WifiHandler::Connect()
{
    if (!_APdone)
    {
        ConnectAP();
        return;
    }
    else
    {
        // reconnection with stored credentials
        WiFi.mode(WIFI_MODE_STA);
        // dont rely on WiFi.SSID / PSK as it gets clear on disconnect so subsequent connections wont work
        WiFi.begin(_wifiSSID, _wifiPass); // this actually is blocking, but unless we want to write the whole wifi library, we accept the small flicker per day that should be fine
    }
}

void WifiHandler::Disconnect()
{
    WiFi.disconnect(true, false);
    DEBUG_PRINTLN("Wifi Disconnect");
}

void WifiHandler::ConnectAP()
{
    WiFi.mode(WIFI_MODE_STA);

    // Trigger AP if necessary
    WiFiManager wifiManager; // following example, keep local, discard after done
#if DEBUG
    // wifiManager.resetSettings(); //to test new AP config flow
#endif
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setConnectTimeout(PRE_AP_TIMEOUT_SEC);
    wifiManager.setTimeout(AP_TIMEOUT_SEC);
    if (!wifiManager.autoConnect("ClimClock"))
    {
        DEBUG_PRINTLN("Failed to connect and hit timeout");
        // reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(1000);
    }

    // store credentials in mem
    DEBUG_PRINT("ssid: ");
    DEBUG_PRINTLN(wifiManager.getWiFiSSID());
    DEBUG_PRINT("password: ");
    DEBUG_PRINTLN(wifiManager.getWiFiPass());
    _wifiSSID = wifiManager.getWiFiSSID();
    _wifiPass = wifiManager.getWiFiPass();

    _isConnected = true;
    _APdone = true;

    DEBUG_PRINTLN("AP Done");
    
    DEBUG_PRINT("ESP32 IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
}

void WifiHandler::configModeCallback(WiFiManager *wifiManager)
{
    DEBUG_PRINT("Entered config mode");
    DEBUG_PRINTLN(WiFi.softAPIP());
    // if you used auto generated SSID, print it
    DEBUG_PRINTLN(wifiManager->getConfigPortalSSID());
}
