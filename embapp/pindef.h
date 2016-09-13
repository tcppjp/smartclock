#ifndef _PINDEF_H
#define _PINDEF_H

namespace pins
{
	// Adafruit 2.8'' TFT LCD Shield w/Touchscreen
	static constexpr int TftScreenDC = 9;
	static constexpr int TftScreenCS = 10;

	static constexpr int TftSDCardCS = 4; // unused

	static constexpr int TftResistiveTouchPanelCS = 8;

	// Ultrasonic Ranger
	static constexpr int Ranger = 7;

	// NCES IoT Base Shield
	static constexpr int NibsWiFiUartRx = 0;
	static constexpr int NibsWiFiUartTx = 1;
	static constexpr int NibsSDCardCS = -1; // unknown, unused

	// That robot
	static constexpr int Servo1 = 4;
	static constexpr int Servo2 = 3;
	static constexpr int Servo3 = 2;

	// AquesTaslk pico
	static constexpr int AquesTalkPico1 = 5;
	static constexpr int AquesTalkPico2 = 6;
}

#endif
