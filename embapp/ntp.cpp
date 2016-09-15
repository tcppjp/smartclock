#include "rca.h"

#include "ESP8266.h"
#include "Time.h"
#include "appconfig.h"

#include "ntp.h"
#include "utils.h"

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

void sendNTPpacket(String address);   

void ntp_setup()
{
	int ret;

	SCSerialPrintLn("ntp_setup: starting");

	// Connect to WiFi access point.
	SCSerialPrint("Connecting to ");
	SCSerialPrintLn(STA_SSID);

	ret = WiFi.begin(Serial5, 115200);

	if(ret == 1) {
		SCFatalError("Cannot communicate with ESP8266.");
	} else if(ret == 2) {
		SCSerialPrintLn("ntp_setup: FW Version mismatch.");
		SCSerialPrint("ntp_setup: FW Version:");
		SCSerialPrintLn(WiFi.getVersion().c_str());
		SCSerialPrint("ntp_setup: Supported FW Version:");
		SCSerialPrintLn(ESP8266_SUPPORT_VERSION);
        SCFatalError("Unsupported ESP8266 FW.");
	} else {
		SCSerialPrint("begin ok\r\n");
	}

	SCSerialPrint("ntp_setup: FW Version:");
	SCSerialPrintLn(WiFi.getVersion().c_str());

    act_tsk(NTP_TASK);
}

void ntp_task(intptr_t exinf)
{
    if (WiFi.setOprToStation()) {
        SCSerialPrint("ntp_task: to station ok\r\n");
    } else {
        SCSerialPrint("ntp_task: to station err\r\n");
    }

    for (int i = 0; i < 10; ++i) {
        if (WiFi.joinAP(STA_SSID, STA_PASSWORD)) {
            SCSerialPrint("ntp_task: Join AP success\r\n");
            SCSerialPrint("ntp_task: IP: ");
            SCSerialPrintLn(WiFi.getLocalIP().c_str());
            break;
        } else {
            SCSerialPrint("ntp_task: Join AP failure\r\n");
        }
    }

    if (WiFi.stopServer()) {
        SCSerialPrint("ntp_task: Stop server ok\r\n");
    } else {
        SCSerialPrint("ntp_task: Stop server err\r\n");
    }

    if (WiFi.disableMUX()) {
        SCSerialPrint("ntp_task: single ok\r\n");
    } else {
        SCSerialPrint("ntp_task: single err\r\n");
    }

    SCSerialPrintLn("ntp_task: setup done");


    while (true) {
        SCSerialPrintLn("ntp_task: updating");

        // NTPサーバへ時刻リクエストを送信
        sendNTPpacket(NTP_NAME);
        
        // NTPサーバからのパケット受信
        // バッファに受信データを読み込む
        uint32_t len = WiFi.recv(packetBuffer, NTP_PACKET_SIZE, 10000);
        if (len > 0) {
            // 時刻情報はパケットの40バイト目からはじまる4バイトのデータ
            unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
            unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

            // NTPタイムスタンプは64ビットの符号無し固定小数点数（整数部32ビット、小数部32ビット）
            // 1900年1月1日0時との相対的な差を秒単位で表している
            // 小数部は切り捨てて、秒を求めている
            unsigned long secsSince1900 = highWord << 16 | lowWord;

            // NTPタイムスタンプをUNIXタイムに変換する
            // UNITタイムは1970年1月1日0時からはじまる
            // 1900年から1970年の70年を秒で表すと2208988800秒になる
            const unsigned long seventyYears = 2208988800UL;
            // NTPタイムスタンプから70年分の秒を引くとUNIXタイムが得られる
            unsigned long epoch = secsSince1900 - seventyYears; 

            // Timeライブラリに時間を設定(UNIXタイム)
            // 日本標準時にあわせるために＋9時間しておく
            setTime(epoch + (9 * 60 * 60));

            SCSerialPrint("ntp_task: JST is "); 
            SCSerialPrint( year() );     SCSerialPrint('/');
            SCSerialPrint( month() );    SCSerialPrint('/');
            SCSerialPrint( day() );      SCSerialPrint(' ');
            SCSerialPrint( hour() );     SCSerialPrint(':'); 
            SCSerialPrint( minute() );   SCSerialPrint(':'); 
            SCSerialPrintLn( second() ); SCSerialPrintLn();

            WiFi.unregisterUDP();
        } else {
            SCSerialPrintLn("ntp_task: update failed"); 
        }

        dly_tsk(30000);
    }
}

// send an NTP request to the time server at the given address 
void sendNTPpacket(String address)
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
