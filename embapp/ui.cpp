#include "rca.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#include "pindef.h"

static Adafruit_ILI9341 tft {
	pins::TftScreenCS, pins::TftScreenDC
};

extern "C" void ui_setup(void)
{
	Serial.println("ui_setup: initializing TFT");
	tft.begin();

	Serial.println("ui_setup: printing diag info");
	// diagnostics info (we don't need this but it makes the startup
	// routine about 20% cooler)
	uint8_t x = tft.readcommand8(ILI9341_RDMODE);
	Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDMADCTL);
	Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDPIXFMT);
	Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDIMGFMT);
	Serial.print("Image Format: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDSELFDIAG);
	Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 

	act_tsk(UI_TASK);
}

extern "C" void ui_task(intptr_t exinf)
{
	tft.fillScreen(ILI9341_BLACK);
	tft.setCursor(0, 0);
	tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
	tft.println("Hello World!");
}
