#include "rca.h"
#include "ESP8266.h"
#include "Time.h"
#include <Wire.h>
#include <SeeedOLED.h>

#include "appconfig.h"
#include "ui.h"

void setup()
{
 	Wire.begin();
	SeeedOled.init();
	SeeedOled.deactivateScroll();
	SeeedOled.setNormalDisplay();
	SeeedOled.clearDisplay();
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
