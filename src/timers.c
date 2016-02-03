#include <stdio.h>
#include "memory.h"
#include "timers.h"
#include "interrupts.h"

#define CLOCKSPEED 4194304
#define FREQ_1   4096 //1024 cycles
#define FREQ_2  16384 // 256 cycles
#define FREQ_3  65536 //  64 cycles
#define FREQ_4 262144 //  16 cycles

#define DIV  0xFF04
#define TIMA 0xFF05
#define TMA  0xFF06
#define TAC  0xFF07 

int timeCounter = 0;
int cycleCounter = 0;
int divideCounter = 0;
int maxCycles = CLOCKSPEED / FREQ_1;


/* 
 * Address Register Details
 * ------------------------
 * 0xFF04  Divider  Counts up at a fixed 16384Hz;
 *                  reset to 0 whenever written to
 * 0xFF05  Counter  Counts up at the specified rate
 *                  Triggers INT 0x50 when going 255->0
 * 0xFF06  Modulo   When Counter overflows to 0,
 *                  it's reset to start at Modulo
 * 0xFF07  Control	
 *       Bits Function Details
 *       ---------------------
 *       0-1  Speed    00:   4096Hz
 *                     01: 262144Hz
 *                     10:  65536Hz
 *                     11:  16384Hz
 *       2    Running  1 to run timer 
 *                     0 to stop
 *       3-7  Unused
 */

void updateFrequency(void){

    unsigned char speed = readMemory(TAC);
    speed = speed & 0x03;
    cycleCounter = 0;                        // clock is reset when frequency is changed
    
    switch (speed){
        case 0:
            maxCycles = CLOCKSPEED / FREQ_1;
            break;
        case 1:
            maxCycles = CLOCKSPEED / FREQ_4;
            break;
        case 2:
            maxCycles = CLOCKSPEED / FREQ_3;
            break;
        case 3:
            maxCycles = CLOCKSPEED / FREQ_2;
            break;
    }
}

void updateTimers(int cycles){
    
    divideCounter += cycles;
    
    if (divideCounter >= 255){
        divideCounter = 0;
        memory[DIV]++;
    }
    
    
    if (testBit(TAC,2) == 0){
        return;             // verify that master timer is enabled
    }
    
    cycleCounter += cycles;
    //printf("[DEBUG] Timers    - Cycles=%07d,MaxCycles=%07d,Timer=%07d\n",cycleCounter,maxCycles,timeCounter);

    if (cycleCounter > maxCycles){
        if (timeCounter == 255){
            timeCounter = 0;
            triggerInterrupt(TIMER_INTERRUPT);
        }
        else{
            timeCounter++;
        }
        cycleCounter = 0;
    }
}


