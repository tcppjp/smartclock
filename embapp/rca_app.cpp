#include "rca.h"
#include "ESP8266.h"
#include "Time.h"
#include <Wire.h>
#include <SeeedOLED.h>

#include "appconfig.h"

void setup()
{
	int ret;

	Serial.begin(115200);
	Serial.print("sN0WwH1T3 prototype app");

	// Connect to WiFi access point.
	Serial.println(); Serial.println();
	Serial.print("Connecting to ");
	Serial.println(STA_SSID);

	ret = WiFi.begin(Serial5, 115200);

	if(ret == 1) {
		Serial.print("Cannot communicate with ESP8266.");
		while(1);
	} else if(ret == 2) {
		Serial.println("FW Version mismatch.");
		Serial.print("FW Version:");
		Serial.println(WiFi.getVersion().c_str());
		Serial.print("Supported FW Version:");
		Serial.println(ESP8266_SUPPORT_VERSION);
		while(1);
	} else {
		Serial.print("begin ok\r\n");
	}

	Serial.print("FW Version:");
	Serial.println(WiFi.getVersion().c_str());

	if (WiFi.setOprToStation()) {
		Serial.print("to station ok\r\n");
	} else {
		Serial.print("to station err\r\n");
	}

	if (WiFi.joinAP(STA_SSID, STA_PASSWORD)) {
		Serial.print("Join AP success\r\n");
		Serial.print("IP: ");
		Serial.println(WiFi.getLocalIP().c_str());
	} else {
		Serial.print("Join AP failure\r\n");
	}

	if (WiFi.stopServer()) {
		Serial.print("Stop server ok\r\n");
	} else {
		Serial.print("Stop server err\r\n");
	}

	if (WiFi.disableMUX()) {
		Serial.print("single ok\r\n");
	} else {
		Serial.print("single err\r\n");
	}

	Serial.println("setup end\r\n");

 	Wire.begin();
	SeeedOled.init();
	SeeedOled.deactivateScroll();
	SeeedOled.setNormalDisplay();
	SeeedOled.clearDisplay();
}

void loop()
{
	SeeedOled.setTextXY(0, 0);
	SeeedOled.putString("sN0WwH1T3");
	SeeedOled.setTextXY(1, 0);
	char msgbuf[32];
	sprintf(msgbuf, "%02d:%02d:%02d", hour(), minute(), second());
	SeeedOled.putString(msgbuf);
	dly_tsk(1000);
}
