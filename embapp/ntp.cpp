#include "rca.h"

#include "ESP8266.h"
#include "Time.h"
#include "appconfig.h"

#include "ntp.h"

#define NTP_NAME    "ntp.nict.jp"
#define NTP_PORT    (123)

#define WiFi wifi

ESP8266 WiFi;

// UDPローカルポート番号
unsigned int localPort = 8888;

// NTPパケットバッファサイズ
const int NTP_PACKET_SIZE= 48;

// NTP送受信用パケットバッファ
byte packetBuffer[NTP_PACKET_SIZE];

unsigned long sendNTPpacket(String address);   

void ntp_update_cyc(intptr_t exinf)
{
    Serial.println("CycHandler");
    iact_tsk( NTP_TASK );
}

void ntp_init(intptr_t exinf)
{
	int ret;

	Serial.begin(115200);
	Serial.print("NTP demo");

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

    sendNTPpacket(NTP_NAME);

    sta_cyc(NTP_UPDATE_CYC);
}

void ntp_task(intptr_t exinf)
{
    // NTPサーバへ時刻リクエストを送信
    sendNTPpacket(NTP_NAME);
    
    // NTPサーバからのパケット受信
    // バッファに受信データを読み込む
    uint32_t len = WiFi.recv(packetBuffer, NTP_PACKET_SIZE, 10000);
    Serial.print("len = "); Serial.println(len);
    if (len > 0) {
        // 時刻情報はパケットの40バイト目からはじまる4バイトのデータ
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

        // NTPタイムスタンプは64ビットの符号無し固定小数点数（整数部32ビット、小数部32ビット）
        // 1900年1月1日0時との相対的な差を秒単位で表している
        // 小数部は切り捨てて、秒を求めている
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        Serial.print("Seconds since Jan 1 1900 = " );
        Serial.println(secsSince1900);

        // NTPタイムスタンプをUNIXタイムに変換する
        // UNITタイムは1970年1月1日0時からはじまる
        // 1900年から1970年の70年を秒で表すと2208988800秒になる
        const unsigned long seventyYears = 2208988800UL;
        // NTPタイムスタンプから70年分の秒を引くとUNIXタイムが得られる
        unsigned long epoch = secsSince1900 - seventyYears; 
        Serial.print("Unix time = ");
        Serial.println(epoch);

        // Timeライブラリに時間を設定(UNIXタイム)
        // 日本標準時にあわせるために＋9時間しておく
        setTime(epoch + (9 * 60 * 60));

        Serial.print("JST is "); 
        Serial.print( year() );     Serial.print('/');
        Serial.print( month() );    Serial.print('/');
        Serial.print( day() );      Serial.print(' ');
        Serial.print( hour() );     Serial.print(':'); 
        Serial.print( minute() );   Serial.print(':'); 
        Serial.println( second() ); Serial.println();

        WiFi.unregisterUDP();
    }
}

// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(String address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE); 
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49; 
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    // NTP requests are to port 123
    // Udp.beginPacket(address, 123);
    // Udp.write(packetBuffer, NTP_PACKET_SIZE);
    // Udp.endPacket();
    WiFi.registerUDP(address, NTP_PORT);
    WiFi.send(packetBuffer, NTP_PACKET_SIZE);
//    WiFi.unregisterUDP();
}
