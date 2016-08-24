#include "rca.h"
#include "ESP8266.h"

#include "reporter.h"
#include "appconfig.h"

#define EVENT_NAME	"something_happened"
#define HOST_NAME	"maker.ifttt.com"
#define HOST_PORT	(80)

static char network_buffer[1024];

extern "C" void reporter_task(intptr_t)
{
	sprintf(network_buffer,
		"GET "
		"/trigger/" EVENT_NAME "/with/key/" IFTTT_KEY "/?value1=%d&value2=%d&value3=%d"
		" HTTP/1.1\r\nHost: " HOST_NAME "\r\nConnection: close\r\n\r\n",
		0, 0, 0);

	if (!WiFi.createTCP(HOST_NAME, HOST_PORT)) {
		Serial.println("create tcp err");
		return;
	}

	WiFi.send((const uint8_t*)network_buffer, strlen(network_buffer));

	uint32_t len = WiFi.recv((uint8_t*)network_buffer, sizeof(network_buffer), 10000);

	if (!WiFi.releaseTCP()) {
		Serial.println("release tcp err");
	}
}



