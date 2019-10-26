#include "Arduino.h"
#include "WiFiManager.h"
#include "esp_wifi.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


void WiFiManager::enableOTA(){
  ArduinoOTA.begin();
  ArduinoOTA.onStart([&]() {
      Serial.println("ArduinoOTA starting..");
  });

  ArduinoOTA.onError([&](ota_error_t err) {
      Serial.println("OTA error: ");
      Serial.println(err);
      ESP.restart();
  });

  ArduinoOTA.onEnd([&]() {
      Serial.println("OTA complete.");
  });

  isEnableOTA=true;

}

bool WiFiManager::init(){

  String msg = "WiFi Setup\nConnecting to ";
  Serial.println("-->WiFi connecting to ");
  Serial.println(ssid);

  WiFi.begin (ssid, password);
  WiFi.mode(WIFI_STA);
  WiFi.reconnect();
  delay(100);

  while (WiFi.waitForConnectResult() != WL_CONNECTED && reconnect<MAX_RECONNECT) {
    Serial.println("Connection Failed! try to reconnect:");
    Serial.println(reconnect);
    reconnect++;
    delay(100);
  }

  reconnect=0;

  if(WiFi.isConnected()){
    Serial.println("-->WiFi ready");
    WiFi.setHostname("esp32potp");

    if(!isEnableOTA)enableOTA();

    // Align text vertical/horizontal center
    Serial.println("-->OTA ready");

    isWifiEnable=true;
  }
  else {
    Serial.println("WiFi setup failed!");
    isWifiEnable = false;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(1000);
  }

  return isWifiEnable;

}

void WiFiManager::disableWifi(){
  isWifiEnable = false;
  Serial.println("-->Disabling WiFi..");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_MODE_NULL);
  // ESP.restart();
}
