#include "rca.h"

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
    aserial.println("ohayo\n"); //しゃべる内容
    dly_tsk (1000);      // 1秒待つ
}
