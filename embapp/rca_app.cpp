#include "rca.h"
#include "ESP8266.h"
#include "Time.h"
#include <Wire.h>
#include <SeeedOLED.h>

#include "appconfig.h"
#include "ui.h"
#include "speak.h"
#include "ntp.h"
#include "dancing.h"
#include "timecheck.h"

void setup()
{
 	Wire.begin();
	Serial.begin(115200);
	speak_init();
	ntp_setup();
	dancing_init();
	timecheck_init();
}

void loop()
{
	slp_tsk();
}
