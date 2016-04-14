#include "memory.h"
#include "timers.h"
#include "gpu.h"

void hardwareTick(void){
    
    updateDMA();
    
    gpu_reading = 1;
//    gpu();
    gpu1();
    gpu_reading = 0;
    
    //updateTimers(4);
    timersTick();
}