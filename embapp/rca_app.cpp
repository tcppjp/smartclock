#include "rca.h"

#include "ESP8266.h"
#include "../../examples_gdef.h"

#define IFTTT_KEY	"dOu9kuvx4fX5P5ziwbSQF91VWfGbFJ52aZhEnZrMiR"
#define EVENT_NAME	"something_happened"

#define HOST_NAME	"maker.ifttt.com"
#define HOST_PORT	(80)

#define WiFi wifi
#define TOUCH_SW 2

ESP8266 WiFi;

void setup()
{
	int ret;

	Serial.begin(115200);
	Serial.print("IFTTT demo");

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
}

char buffer[1024];
const char request1[] = "GET /trigger/" EVENT_NAME "/with/key/" IFTTT_KEY "/";
const char request2[] = " HTTP/1.1\r\nHost: " HOST_NAME "\r\nConnection: close\r\n\r\n";

void loop()
{
	static int value1 = 0, value2 = 1, value3 = 2;
	int sw_push = 0;
	static int sw_st = 1;

	switch(sw_st) {
	  case 0:
		if(digitalRead(TOUCH_SW)) {
			sw_push = 1;
			sw_st = 1;
		}
		break;
	  default:
		if(!digitalRead(TOUCH_SW)) {
			sw_st = 0;
		}
		break;
	}

	if(sw_push) {
		value1++; value2++; value3++;

		Serial.print("Send Touch Event and Extra Data ");
		Serial.print(value1); Serial.print(", ");
		Serial.print(value2); Serial.print(", ");
		Serial.println(value3);

		if (!WiFi.createTCP(HOST_NAME, HOST_PORT)) {
			Serial.println("create tcp err");
			return;
		}

		sprintf(buffer, "%s?value1=%d&value2=%d&value3=%d%s", request1, value1, value2, value3, request2);

		WiFi.send((const uint8_t*)buffer, strlen(buffer));

		uint32_t len = WiFi.recv((uint8_t*)buffer, sizeof(buffer), 10000);

		if (!WiFi.releaseTCP()) {
			Serial.println("release tcp err");
		}
	}
}
