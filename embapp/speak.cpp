#include "rca.h"

#include "time.h"
#include <SoftwareSerial.h>
#include "speak.h"

SoftwareSerial aserial(5,6);

void speak_init() // 初期設定
{
  aserial.begin(9600);
  dly_tsk (100);
  aserial.write('?');
}

void speak_task(intptr_t exinf) // メインループ
{
    int realhour=0;
    int realminute=0;
    int realsecond=0;
    realhour=hour();
    realminute=minute();
    realsecond=second();

    int zone = (realhour * 100 + realminute);
    if (zone >= 1130 && zone < 1230) {
    	// デモ用サービス
	    aserial.println("ohiruyasumi desuyo\n"); //しゃべる内容
	    dly_tsk (4500);      // 1秒待つ
    } else if (zone > 1030) {
	    aserial.println("konnichiwa\n"); //しゃべる内容
	    dly_tsk (2000);      // 1秒待つ
	} else {
	    aserial.println("ohayo\n"); //しゃべる内容
	    dly_tsk (1000);      // 1秒待つ
	}
    aserial.println("genzai\n"); //しゃべる内容
    dly_tsk (1000);
    aserial.println("no\n"); //しゃべる内容
    dly_tsk (1000);
    aserial.println("zikoku\n"); //しゃべる内容
    dly_tsk (1000);
    aserial.println("wa\n"); //しゃべる内容
    dly_tsk (1000);
    aserial.print("<NUMK VAL="); //しゃべる内容
    aserial.print(realhour);
    aserial.println(" COUNTER=di>"); //しゃべる内容
    dly_tsk (1500);
    aserial.print("<NUMK VAL="); //しゃべる内容
    aserial.print(realminute); //しゃべる内容
    aserial.println(" COUNTER=funn>"); //しゃべる内容
    dly_tsk (1500);
    aserial.print("<NUMK VAL="); //しゃべる内容
    aserial.print(realsecond); //しゃべる内容
    aserial.println(" COUNTER=byo->"); //しゃべる内容
    dly_tsk (1700);
    aserial.println("desu\n"); //しゃべる内容
    dly_tsk (1000);

}
