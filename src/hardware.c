#include "cpu.h"
#include "memory.h"
#include "timers.h"
#include "gpu.h"
#include "hardware.h"
#include "definitions.h"
#include "serial.h"
#include "input.h"

void hardwareTick(void){
    
    updateDMA();
    
    gpu_reading = 1;
    gpu();
    gpu_reading = 0;
   
    timersTick();
    serialUpdateClock();
}

void hardwareReset(void)
{
    cpuReset();
    gpuReset();
    serialReset();
    timersReset();
    memoryReset();
    inputReset();
}