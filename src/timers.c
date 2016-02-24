#include <stdio.h>
#include "memory.h"
#include "timers.h"
#include "interrupts.h"
#include "gpu.h"

#define CLOCKSPEED 4194304
#define FREQ_1   4096 //1024 cycles
#define FREQ_2  16384 // 256 cycles
#define FREQ_3  65536 //  64 cycles
#define FREQ_4 262144 //  16 cycles

#define DIV  0xFF04
#define TIMA 0xFF05
#define TMA  0xFF06
#define TAC  0xFF07 

unsigned int timeCounter = 0;
unsigned int cycleCounter = 4; //???????????????????
int divideCounter = 0;
unsigned int maxCycles = CLOCKSPEED / FREQ_1;
int transition = FALSE;


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

void updateFrequency(unsigned char value){

    unsigned char speed_old = memory[TAC] & 0x03;
    unsigned char speed_new = value & 0x03;

    if (testBit(TAC,2) == 0){
        if (value & 0x04){
            transition = TRUE;
        }
    }
        
    if (speed_new == speed_old){ // if no speed changed
        memory[TAC] = value & 0x07;
        printf("[DEBUG] Timer Updated - %x\n",speed_new);
        return;
    }

    //cycleCounter = 0;                        // clock is reset when frequency is changed
    //writeMemory(TIMA, readMemory8(TMA));
    
    switch (speed_new){
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
    
    memory[TAC] = value & 0x07;
   //printf("[DEBUG] Timer Updated - %x\n",speed_new);
}

void updateTimers(int cycles){

        gpu_reading = 1;
		gpu(cycles);
        gpu_reading = 0;    
    divideCounter += cycles;
    
    if (divideCounter >= 256){
        divideCounter -= 256;
        memory[DIV]++;
    }
    
    //printf("[DEBUG] counter=%2d  DIV= %2d\n",divideCounter,memory[DIV]);
    printf("tick\n");    
    
    if (testBit(TAC,2) == 0){
        //printf("[DEBUG] Cycles= %07d, Timer= %7d\n",cycles,memory[TIMA]);
        return;             // verify that master timer is enabled
    }
    
    if (transition){
        cycles += 4;
        transition = FALSE;
    }
    //printf("[DEBUG] Timers before   - Cycles=%07d,Cyclecounter=%07d,MaxCycles=%07d,Timer=%07d\n",cycles,cycleCounter,maxCycles,memory[TIMA]);
    cycleCounter += cycles;
    timeCounter = memory[TIMA];    // Read current timer value
    

    while (cycleCounter >= maxCycles){
        if (timeCounter == 255){
            timeCounter = memory[TMA]; // Start timer from modulo
            triggerInterrupt(TIMER_INTERRUPT);
        }
        else{
            timeCounter++;
        }
        cycleCounter -= maxCycles;
    }
   // printf("[DEBUG] Timers after    - Cycles=%07d,Cyclecounter=%07d,MaxCycles=%07d,Timer=%07d\n\n",cycles,cycleCounter,maxCycles,timeCounter);
   // printf("[DEBUG] Cycles= %07d, Timer= %7d\n",cycles,timeCounter);
    memory[TIMA] = timeCounter;
}


