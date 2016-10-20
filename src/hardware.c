#include "cpu.h"
#include "memory.h"
#include "timers.h"
#include "gpu.h"
#include "hardware.h"
#include "definitions.h"
#include "sound.h"
#include <stdio.h>
#include "serial.h"
#include "input.h"


void hardwareTick(void){
    #ifdef SOUND
    sound_tick(4);
    #else
    soundTick();
    #endif
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
    memoryDMAReset();
    inputReset();
    #ifdef SOUND
    init_apu();
    #else
    soundReset();
    #endif
}