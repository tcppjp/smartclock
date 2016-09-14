#include "rca.h"

#include "dancing.h"
#include <Servo.h>      // サーボのライブラリ
#include <Wire.h>       // I2Cライブラリ

const int svPc = 90;    // サーボの中心角度（Pitch）
const int svRc = 90;    // サーボの中心角度（Roll）
const int svYc = 90;    // サーボの中心角度（Yaw）
int destP = svPc;       // サーボの目標角度（Pitch）
int destR = svRc;       // サーボの目標角度（Roll）
int destY = svYc;       // サーボの目標角度（Yaw）
int spd = 0;            // サーボの速度。0が最速

extern void servoMove ();
extern void centering ();
extern void initServo ();

void dancing_init() // 初期設定
{
  initServo();          // サーボの初期センタリング
  dly_tsk (100);
}

void dancing_task(intptr_t exinf) // メインループ
{
  int i=0;
  for(i=0;i<10;i++){
    spd = 0;           // サーボの速度
    destY = svYc + 5; // 体を左に回す指定
    servoMove();       // サーボを動かす
    destY = svYc - 5; // 体を右に回す指定
    servoMove();       // サーボを動かす
    destY = svYc;      // 体を正面に向ける指定
    servoMove();       // サーボを動かす
    destP = svPc + 5; // 首を下に動かす指定
    servoMove();       // サーボを動かす
    dly_tsk (500);       // 0.5秒待つ
    destP = svPc     ; // 首を元に戻す指定
    servoMove();       // サーボを動かす
    destR = svRc + 5; // 首を反時計回りに回す指定
    servoMove();       // サーボを動かす
    destR = svRc - 5; // 首を時計回りに回す指定
    servoMove();       // サーボを動かす
    destR = svRc;      // 首を元に戻す指定
    servoMove();       // サーボを動かす
    dly_tsk (1000);      // 1秒待つ
  }
}

//----------------------------------------------------------------------
// サーボの設定
//const int pinSvC = 12C;   // サーボ(センタリング用)をGPIO 5に割り当てる
const int pinSvP = 4;   // サーボ(Pitch首上下)をGPIO 3に割り当てる
const int pinSvR = 3;  // サーボ(Roll首回転)をGPIO 10に割り当てる
const int pinSvY = 2;  // サーボ(Yaw体回転台)をGPIO 11に割り当てる
//Servo svC;             // サーボのインスタンスを作る(センタリング用)
Servo svP;             // サーボのインスタンスを作る(Pitch 首の縦振り用)
Servo svR;             // サーボのインスタンスを作る(Roll 顔の回転用)
Servo svY;             // サーボのインスタンスを作る(Yaw 体の回転用)
int posP = svPc;        // サーボの現在の角度(Pitch)
int posR = svRc;        // サーボの現在の角度(Roll)
int posY = svYc;        // サーボの現在の角度(Yaw)

//----------------------------------------------------------------------

void servoMove (){       // サーボを動かす関数
  while (posP != destP){
    svP.attach(pinSvP);
    if (posP < destP){
      posP++;
    }else{
      posP--;
    }
    svP.write(posP);
    dly_tsk (spd+15);
  }
  while (posR != destR){
    svR.attach(pinSvR);
    if (posR < destR){
      posR++;
    }else{
      posR--;
    }
    svR.write(posR);
    dly_tsk (spd+15);
  }
  while (posY != destY){
    svY.attach(pinSvY);
    if (posY < destY){
      posY++;
    }else{
      posY--;
    }
    svY.write(posY);
    dly_tsk (spd+15);
  }
  svP.detach ();
  svR.detach ();
  svY.detach ();
}

void centering (){       // サーボをセンタリングする関数
  spd = 1;
  destP = svPc;
  destR = svRc;
  destY = svYc;
  servoMove();
}

void initServo (){       // サーボの初回センタリング
//  svC.attach(pinSvC);
  svP.attach(pinSvP);
  svR.attach(pinSvR);
  svY.attach(pinSvY);
//  svC.write(100);
  svP.write(svPc);
  svR.write(svRc);
  svY.write(svYc);
  dly_tsk (500);
//  svC.write(90);
  dly_tsk (500);
//  svC.detach ();
  svP.detach ();
  svR.detach ();
  svY.detach ();
}
