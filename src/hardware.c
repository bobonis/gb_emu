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
#include <SDL2/SDL.h>


void hardwareTick(void){

    static unsigned long int cycles = 0;
    static unsigned long int frametime = 0;
    static long int frames = 0;
    static long int lastTime = 0;
    long int currentTime = 0;

    cycles += 4;

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

    if (cycles % CLOCKSPERFRAME == 0) {
        hardwareSync();
        frames += 1;
        if (frames == 60) {
            currentTime = SDL_GetTicks();
            printf("time for 60 frames = %d\n",currentTime - lastTime);
            lastTime = currentTime;
            frames = 0;
        }
    }
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

void hardwareSync(void) {

    static unsigned long int lastTime = 0;
    
    unsigned long int currentTime = SDL_GetTicks();
    unsigned long sleepTime = 0;

    if (currentTime - lastTime < 17) {
        sleepTime = 17 - (currentTime - lastTime);
        //printf("[HW] Delay START\n");
        SDL_Delay(sleepTime);
        //printf("[HW] Delay END\n");
    }
    
    lastTime = SDL_GetTicks();
}