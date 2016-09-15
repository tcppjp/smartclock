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
    aserial.println("ohayo\n"); //しゃべる内容
    dly_tsk (1000);      // 1秒待つ
    realhour=hour();
    realminute=minute();
    realsecond=second();
    aserial.println("genzai\n"); //しゃべる内容
    dly_tsk (1000);
    aserial.println("no\n"); //しゃべる内容
    dly_tsk (1000);
    aserial.println("zikoku\n"); //しゃべる内容
    dly_tsk (1000);
    aserial.println("wa\n"); //しゃべる内容
    dly_tsk (1000);
    aserial.println("<NUMK VAL="); //しゃべる内容
    aserial.println(realminute);
    aserial.println("COUNTER=di>"); //しゃべる内容
    dly_tsk (1000);
    aserial.println("<NUMK VAL="); //しゃべる内容
    aserial.println(realminute); //しゃべる内容
    aserial.println("COUNTER=funn>"); //しゃべる内容
    dly_tsk (1000);
    aserial.println("<NUMK VAL="); //しゃべる内容
    aserial.println(realsecond); //しゃべる内容
    aserial.println("COUNTER=byo->"); //しゃべる内容
    dly_tsk (1000);
    aserial.println("desu\n"); //しゃべる内容
    dly_tsk (1000);
}
