#include "rca.h"

#include <Servo.h>      // サーボのライブラリ
#include <Wire.h>       // I2Cライブラリ

const int svPc = 90;    // サーボの中心角度（Pitch）
const int svRc = 90;    // サーボの中心角度（Roll）
const int svYc = 90;    // サーボの中心角度（Yaw）
int destP = svPc;       // サーボの目標角度（Pitch）
int destR = svRc;       // サーボの目標角度（Roll）
int destY = svYc;       // サーボの目標角度（Yaw）
int spd = 0;            // サーボの速度。0が最速

void dancing_init() // 初期設定
{
  Wire.begin();
  initServo();          // サーボの初期センタリング
  dly_tsk (100);
}

void dancing_task() // メインループ
{
  spd = 0;           // サーボの速度
  destY = svYc + 5; // 体を左に回す指定
  servoMove();       // サーボを動かす
  destY = svYc - 5; // 体を右に回す指定
  servoMove();       // サーボを動かす
  destY = svYc;      // 体を正面に向ける指定
  servoMove();       // サーボを動かす
  destP = svPc + 5; // 首を下に動かす指定
  servoMove();       // サーボを動かす
  delay (500);       // 0.5秒待つ
  destP = svPc     ; // 首を元に戻す指定
  servoMove();       // サーボを動かす
  destR = svRc + 5; // 首を反時計回りに回す指定
  servoMove();       // サーボを動かす
  destR = svRc - 5; // 首を時計回りに回す指定
  servoMove();       // サーボを動かす
  destR = svRc;      // 首を元に戻す指定
  servoMove();       // サーボを動かす
  delay (1000);      // 1秒待つ
}

void servoMove (){       // サーボを動かす関数
  while (posP != destP){
    svP.attach(pinSvP);
    if (posP < destP){
      posP++;
    }else{
      posP--;
    }
    svP.write(posP);
    delay (spd+15);
  }
  while (posR != destR){
    svR.attach(pinSvR);
    if (posR < destR){
      posR++;
    }else{
      posR--;
    }
    svR.write(posR);
    delay (spd+15);
  }
  while (posY != destY){
    svY.attach(pinSvY);
    if (posY < destY){
      posY++;
    }else{
      posY--;
    }
    svY.write(posY);
    delay (spd+15);
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
  delay (500);
//  svC.write(90);
  delay (500);
//  svC.detach ();
  svP.detach ();
  svR.detach ();
  svY.detach ();
}
