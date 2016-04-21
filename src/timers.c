#include "timers.h"
#include "interrupts.h"
#include "definitions.h"

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

    unsigned int internal_old = timerstate.internal;    /* keep old timer value */
    
    timerstate.internal += 4;                           /* step internal register */

    if (timerstate.overflow == DELAY){
        timerstate.overflow = NOW;
        timerstate.tima = timerstate.tma;
        triggerInterrupt(TIMER_INTERRUPT);
    }
    else if (timerstate.overflow == NOW){
        timerstate.overflow = OFF;
    }
    
/* 
 * This is the implementation of the falling edge detector
 * In case the previous selected bit of the multiplexer was set
 * and now it's not set, then the TIMA register is increased 
 */
    if (timerstate.enable){
        if (internal_old & clocks[timerstate.frequency]){
            if ((timerstate.internal & (clocks[timerstate.frequency])) == 0){
                timersStepTIMA();
            }
        }
    }
}

/* 
 * When TIMA overflows, the value from TMA is loaded and IF timer flag 
 * is set to 1, but this doesn't happen immediately. Timer interrupt is 
 * delayed 1 cycle (4 clocks) from the TIMA overflow. It could be less 
 * clocks, but the CPU can't check that. 
 */  
void timersStepTIMA (void){
    
    timerstate.tima += 1;
  
    if (timerstate.tima > 0xFF){
        timerstate.overflow = DELAY;
        timerstate.tima = 0x00;
    }
}

/* 
 * Functions to read internal time registers 
 */
unsigned char timersGetDIV(void){
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

/* 
 * Functions to set internal time registers 
 */
 
/* 
 * When writing to DIV register the TIMA register can be increased if 
 * the counter has reached half the clocks it needs to increase because 
 * the selected bit by the multiplexer will go from 1 to 0
 */
void timersSetDIV (void){

    if (timerstate.enable){
        if (timerstate.internal & clocks[timerstate.frequency]){
            timersStepTIMA();
            /* Have to verify if a delayed interrupt is lost
            if (timerstate.overflow == DELAY){
                triggerInterrupt(TIMER_INTERRUPT);
            }
            */
        }
    }
    timerstate.internal = 0;                            /* Internal counter is being reset */
}

/* 
 * During the delay you can prevent the IF flag from being set and prevent 
 * the TIMA from being reloaded from TMA by writing a value to TIMA. That 
 * new value will be the one that stays in the TIMA register after the instruction. 
 *
 * If you write to TIMA during the cycle that TMA is being loaded to it, the write 
 * will be ignored and TMA value will be written to TIMA instead.
 */
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
}

/*
 * When disabling the timer, if the corresponding bit in the system 
 * counter is set to 1, the falling edge detector will see a change 
 * from 1 to 0, so TIMA will increase.
 *
 * When changing TAC register value, if the old selected bit by the 
 * multiplexer was 0, the new one is 1, and the new enable bit of 
 * TAC is set to 1, it will increase TIMA.
 */
 /* CAN BE OPTIMIZED */
void timersSetTAC (unsigned char value){
    
    unsigned int new_enable = (value & 0x04) >> 2;
    unsigned int new_frequency = value & 0x03;
    
    if ((timerstate.enable == TRUE) && (new_enable == FALSE) ){
        if ((timerstate.internal) & (clocks[timerstate.frequency])){
            timersStepTIMA();
            /* Have to verify if a delayed interrupt is lost
            if (timerstate.overflow == DELAY){
                triggerInterrupt(TIMER_INTERRUPT);
            }
            */
        }
    }
    else if (timerstate.frequency != new_frequency){
        if ((timerstate.internal & clocks[timerstate.frequency]) == 1){
            if ((timerstate.internal & clocks[new_frequency]) == 0){
                if (new_enable){
                    timersStepTIMA();
                    /* Have to verify if a delayed interrupt is lost
                    if (timerstate.overflow == DELAY){
                        triggerInterrupt(TIMER_INTERRUPT);
                    }
                    */                 
                }
            }
        }
    }
    
    timerstate.enable = new_enable;
    timerstate.frequency = new_frequency;
}

/*
 * If TMA is written the same cycle it is loaded to TIMA, TIMA is also 
 * loaded with that value.
 */
void timersSetTMA (unsigned char value){
    
    timerstate.tma = value;
    
    if (timerstate.overflow == NOW){
        timerstate.tima = timerstate.tma;
    }
}