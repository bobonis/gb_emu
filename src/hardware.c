#include "memory.h"
#include "timers.h"
#include "gpu.h"

void hardwareTick(void){
    
    updateDMA();
    
    gpu_reading = 1;
    gpu();
    gpu_reading = 0;
   
    timersTick();
}