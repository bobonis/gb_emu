#include "memory.h"
#include "timers.h"
#include "gpu.h"
#include "hardware.h"
#include "definitions.h"
#include "sound.h"
#include <stdio.h>
#include "serial.h"


void hardwareTick(void){
    
    updateDMA();
    
    gpu_reading = 1;
    gpu();
    gpu_reading = 0;
   
    timersTick();
    serialUpdateClock();
}