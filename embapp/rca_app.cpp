#include "rca.h"
#include "ESP8266.h"
#include "Time.h"
#include <Wire.h>
#include <SeeedOLED.h>

#include "appconfig.h"
#include "ui.h"
#include "speak.h"
#include "ntp.h"

void setup()
{
 	Wire.begin();
	Serial.begin(115200);
	ui_showSplash();
	speak_init();
	ntp_setup();
	ui_setup();
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
