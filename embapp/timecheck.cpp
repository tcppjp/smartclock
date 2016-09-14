#include "rca.h"

#include "Time.h"
#include "speak.h"
#include "dancing.h"

#include "timecheck.h"
void timecheck_init(){
  act_tsk(TIMECHECK_TASK);
}

void timecheck_task(intptr_t exinf){
  int alarmhour=hour(); //アラーム設定時刻（時間）
  int alarmminute=minute(); //アラーム設定時刻（分）
  int alarmtime=(alarmhour*60+alarmminute)*60+second(); //アラーム鳴動時刻
  int afteroneminute=0;
//  int realtime=(hour()*60+minute())*60+second(); //現在時刻
  while(1){
    //Serial.println(alarmtime);
    if(alarmtime==(hour()*60+minute())*60+second()){
      while(1){
        //Serial.println("alarmdousachuuu");
        act_tsk(SPEAK_TASK);
        act_tsk(DANCING_TASK);
        afteroneminute=alarmtime+5;
        if(afteroneminute==(hour()*60+minute())*60+second()){ //アラーム鳴動時刻から1分後
          alarmtime=alarmtime+15; //アラーム鳴動設定時刻を5分後に設定し直す
          break; //アラームストップ
        }
        dly_tsk(500);
      }
    }
    dly_tsk(500);
  }
}
