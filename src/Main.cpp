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


void setupLocalTime()
// *******************************************************************************************
{
	// see https://github.com/esp8266/Arduino/issues/4637
	time_t now; 
	now = time(nullptr); // if there's no time, this will have a value of 28800; Thu Jan  1 08:00:00 1970 
	Serial.print("Initial time:"); Serial.println(now);
	Serial.println(ctime(&now));

	int myTimezone = -7;
	int dst = 0;
	int SecondsPerHour = 3600;
	int MAX_TIME_RETRY = 60;
	int i = 0;

	// it is unlikely that the time is already set since we have no battery; 
	// if no time is avalable, then try to set time from the network
	if (now <= 1500000000) {
		// try to set network time via ntp packets
		configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // see https://github.com/esp8266/Arduino/issues/4749#issuecomment-390822737

                // Starting in 2007, most of the United States and Canada observe DST from
		// the second Sunday in March to the first Sunday in November.
		// example setting Pacific Time:
		setenv("TZ", "PST8PDT,M3.2.0/02:00:00,M11.1.0/02:00:00", 1); // see https://users.pja.edu.pl/~jms/qnx/help/watcom/clibref/global_data.html
		//                     | month 3, second sunday at 2:00AM
		//                                    | Month 11 - firsst Sunday, at 2:00am  
		// Mm.n.d
		//   The dth day(0 <= d <= 6) of week n of month m of the year(1 <= n <= 5, 1 <= m <= 12, where 
		//     week 5 means "the last d day in month m", which may occur in the fourth or fifth week).
		//     Week 1 is the first week in which the dth day occurs.Day zero is Sunday.

		tzset();
		Serial.print("Waiting for time(nullptr).");
		i = 0;
		while (!time(nullptr)) {
			Serial.print(".");
			delay(1000);
			i++;
			if (i > MAX_TIME_RETRY) {
				Serial.println("Gave up waiting for time(nullptr) to have a valid value.");
				break;
			}
		}
	}
	Serial.println("");

	// wait and determine if we have a valid time from the network. 
	now = time(nullptr);
	i = 0;
	Serial.print("Waiting for network time.");
	while (now <= 1500000000) {
		Serial.print(".");
		delay(1000); // allow a few seconds to connect to network time.
		i++;
		now = time(nullptr);
		if (i > MAX_TIME_RETRY) {
			Serial.println("Gave up waiting for network time(nullptr) to have a valid value.");
			break;
		}
	}
	Serial.println("");

	// get the time from the system
	char *tzvalue;
	tzvalue = getenv("TZ");
	Serial.print("Network time:");  Serial.println(now);
	Serial.println(ctime(&now));
	Serial.print("tzvalue for timezone = "); Serial.println(tzvalue);


	now = time(nullptr);
	Serial.println("Final time:");  Serial.println(now);
	Serial.println(ctime(&now));
}


static void initSNTP(void) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("-->Initializing SNTP");
    configTime(0, 0, "a.st1.ntp.br");
}

static void getTimeFromSNTP(void) {
    setupLocalTime();
    time_t now;
    // wait for time to be set
    struct tm timeinfo = {
        0
    };
    int retry = 0;
    const int retry_count = 20;


    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("-->Waiting for system time to be set.");

    Serial.println(timeinfo.tm_year);
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

    char * newCode = totp.getCode(<<OTP_CODE>>, 30, time( & now));
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
            //goToSleep();
        }
    }
}

/******************************************************************************
 *****************************   S E T U P   ***********************************
 ******************************************************************************/

void setup() {
    Serial.begin(115200);
    delay(5000);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("== ESP32 Booting ==");

    wifi.disableWifi();
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
   calcOTPCodeAndPrintScreen();
}
