#ifndef WifiManager_h
#define WifiManager_h

#include "Arduino.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define MAX_RECONNECT 5

class WiFiManager
{

  public:

    bool init();

    void disableWifi();

    void enableOTA();

    bool isWifiEnable = false;

  private:

    // WiFi setup, define secrets on global enveiroment
    const char *ssid         = "susu";

    const char *password     = "suzukinha";

    bool isEnableOTA = false;

    int reconnect = 0;

};

#endif
