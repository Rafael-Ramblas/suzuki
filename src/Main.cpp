#include <LiquidCrystal_I2C.h>

#include <Arduino.h>

#include <sys/time.h>

#include <string.h>

#include <time.h>

#include "WiFiManager.h"

#include "esp_sleep.h"

#include "sha1.h"

#include "TOTP++.h"

#include "lwip/err.h"

#include "apps/sntp/sntp.h"


TOTP totp = TOTP();
char code[6];
WiFiManager wifi;

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

/**************************   NTP HANDLING   **********************************/

static void initSNTP(void) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("-->Initializing SNTP");
    configTime(0, 0, "pool.ntp.org");
}

static void getTimeFromSNTP(void) {
    initSNTP();
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {
        0
    };
    int retry = 0;
    const int retry_count = 20;


    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("-->Waiting for system time to be set.");


    while (timeinfo.tm_year < (2019 - 1900) && ++retry < retry_count) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(".");
        delay(10);
        time( & now);
        localtime_r( & now, & timeinfo);
    }
    Serial.println("ready.");
}

void goToSleep(){
    lcd.noDisplay();
    lcd.noBacklight();
    wifi.disableWifi();
    delay(2000);
    esp_deep_sleep_start();
}


void calcOTPCodeAndPrintScreen() {
    time_t now;
    struct tm timeinfo;
    char timebuf[64];

    char * newCode = totp.getCode("mnmcdbop5qegxtk3fe4mdnxt2vkfvnly", 30, time( & now));
    localtime_r( & now, & timeinfo);
    strftime(timebuf, sizeof(timebuf), "%Y %a, %d %b %H:%M:%S", & timeinfo);
    if (strcmp(code, newCode) != 0) {
        strcpy(code, newCode);
        if (timeinfo.tm_year < (2019 - 1900)) {
           Serial.println("TESTE");
            lcd.setCursor(0, 0);
        } else {
            Serial.println(timebuf);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("OTP code: ");
            lcd.print(code);
            goToSleep();
        }
    }
}

/******************************************************************************
 *****************************   S E T U P   ***********************************
 ******************************************************************************/

void setup() {
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("== ESP32 Booting ==");

    wifi.disableWifi();

    setenv("TZ", "<-03>3", 1);
    tzset();

    wifi.init();
    getTimeFromSNTP();
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1);
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("== ESP32 Booting ==");
    calcOTPCodeAndPrintScreen();
    
    
    
}

/******************************************************************************
 *****************************    L O O P    ***********************************
 ******************************************************************************/

void loop() {
   
}