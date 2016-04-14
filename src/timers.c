#include <stdio.h>
#include "memory.h"
#include "timers.h"
#include "interrupts.h"
#include "gpu.h"
#include "cpu.h"

#define CLOCKSPEED 4194304
#define FREQ_1   4096 //1024 cycles
#define FREQ_2  16384 // 256 cycles
#define FREQ_3  65536 //  64 cycles
#define FREQ_4 262144 //  16 cycles

#define DIV  0xFF04
#define TIMA 0xFF05
#define TMA  0xFF06
#define TAC  0xFF07

#define DELAY 3
#define SOON 2
#define NOW 1
#define OFF 0

unsigned int timeCounter = 0;
unsigned int cycleCounter = 0; //???????????????????
int divideCounter = 0;
unsigned int maxCycles = CLOCKSPEED / FREQ_1;
int transition = FALSE;



struct timer timerstate = {
    0,          /* Internal register */
    FALSE,      /* TAC Enable */
    0,          /* Frequency */
    FALSE,      /* TIMA overflow */
    0,          /* TIMA */
    0,          /* TMA */
    0,          /* DIV */
    0,          /* TAC */
};

const unsigned int clocks[4] = {512, 8, 32, 128};
const unsigned int clocks2[4] = {1024, 16, 64, 256};

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










void timersTick(void){

    //printf("tick = %6d\n",timerstate.internal);

    unsigned int internal_old = timerstate.internal;
    timerstate.internal += 4;           /* step internal register */

    //if (timerstate.internal >> 16){     /* treat internal register as 16bit */
    //    timerstate.internal = 0;        /* handle internal register overflow */
    //}

    
    if (timerstate.overflow == DELAY){
        timerstate.overflow = NOW;
        timerstate.tima = timerstate.tma;
        printf("[DEBUG] INTERRUPT\n");
        triggerInterrupt(TIMER_INTERRUPT);
    }
    else if (timerstate.overflow == NOW){
        timerstate.overflow = OFF;
    }
    
    if (timerstate.enable){
        if (internal_old & clocks[timerstate.frequency]){
            if ((timerstate.internal & (clocks[timerstate.frequency])) == 0){
                printf("[DEBUG] clocks = %d\n", timerstate.internal);
                printf("[DEBUG] TIMA = %x\n", timerstate.tima);
                timersStepTIMA();
            }
        }
    }
}


void timersStepTIMA (void){
    
    timerstate.tima += 1;
    
    if (timerstate.tima > 0xFF){
        timerstate.overflow = DELAY;
        timerstate.tima = 0x00;
    }
    printf("[DEBUG] TIMA = %x\n", timerstate.tima);
}





unsigned char timersGetDIV(void){
    printf("[DEBUG] DIV = %x\n", timerstate.internal);
    return timerstate.internal >> 8;
}

unsigned char timersGetTIMA(void){
    return timerstate.tima;
}
unsigned char timersGetTAC(void){
    return 0xF8 | (timerstate.enable << 2) | timerstate.frequency;
}
unsigned char timersGetTMA(void){
    return timerstate.tma;
}


void timersSetDIV (void){
    printf("[DEBUG] DIV = %x\n", timerstate.internal);
    if (timerstate.enable){
        if (timerstate.internal & clocks[timerstate.frequency]){
                printf("[DEBUG] TIMA step due to DIV write %d\n",timerstate.internal);
                timersStepTIMA();
                if (timerstate.overflow == DELAY){
                    //triggerInterrupt(TIMER_INTERRUPT);
                }
        }
    }
    
    timerstate.internal = 0;
}


void timersSetTIMA(unsigned char value){

    if (timerstate.overflow == DELAY){
        timerstate.tima = value;
        timerstate.overflow = NOW;
    }
    else if (timerstate.overflow == NOW){
        timerstate.tima = timerstate.tma;
        timerstate.overflow = OFF;       
    }
    else{
        timerstate.tima = value;
    }
    printf("[DEBUG] SET TIMA = %x\n", timerstate.tima);
}

void timersSetTAC (unsigned char value){
    
    unsigned int new_enable = (value & 0x04) >> 2;
    unsigned int new_frequency = value & 0x03;
    
    if ((timerstate.enable == TRUE) && (new_enable == FALSE) ){
        if ((timerstate.internal) & (clocks[timerstate.frequency])){
            printf("[DEBUG] TIMA step due to switch off %d\n",timerstate.internal);
            timersStepTIMA();
            if (timerstate.overflow == DELAY){
                //triggerInterrupt(TIMER_INTERRUPT);
            }
        }
    }
    else if (timerstate.frequency != new_frequency){
        if ((timerstate.internal & clocks[timerstate.frequency]) == 1){
            if ((timerstate.internal & clocks[new_frequency]) == 0){
                if (new_enable){
                    printf("[DEBUG] TIMA step due to frequency change\n");
                    timersStepTIMA();
                    if (timerstate.overflow == DELAY){
                        //triggerInterrupt(TIMER_INTERRUPT);
                    }                  
                }
            }
        }
    }
    
    timerstate.enable = new_enable;
    timerstate.frequency = new_frequency;
    printf("[DEBUG] Timer Enable = %d\n",timerstate.enable);
}



void timersSetTMA (unsigned char value){
    
    timerstate.tma = value;
    
    if (timerstate.overflow == NOW){
        timerstate.tima = timerstate.tma;
        //timerstate.overflow = OFF;
    }
    
}