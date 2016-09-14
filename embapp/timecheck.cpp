#include "rca.h"

#include "Time.h"
#include "speak.h"
#include "dancing.h"

#include "timecheck.h"
void timecheck_init(){
  act_tsk(TIMECHECK_TASK);
}

void timecheck_task(intptr_t exinf){
  while(1){
    if(hour()==hour()){
      if(minute()==minute()){
        act_tsk(SPEAK_TASK);
        act_tsk(DANCING_TASK);
      }
    }
    dly_tsk(500);
  }
}
