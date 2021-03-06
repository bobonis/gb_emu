#include "interrupts.h"
#include "memory.h"
#include "cpu.h"
#include "timers.h"
#include "hardware.h"
#include <stdio.h>
#include "definitions.h"

/*
 * When an interrupt is requested we set the
 * appropriate bit in the Interrupt Request Register
 * If IME was enabled, we serve the next interrupt,
 * else we continue execution
 */
void triggerInterrupt(int interrupt){
    
    
    /* HALT mode is exited when a flag in register IF is set and the 
     * corresponding flag in IE is also set, regardless of the value of IME.
     */
    if (testBit(IER, interrupt)) {
        cpustate.halt = FALSE;
    }
    
    setBit(IFR, interrupt, TRUE);
}



void handleInterrupts(void){

    unsigned int bit;

    for (bit=0;bit<5;bit++){                // Find the requested Interrupt, priority matters
        if ( ((memory[IER] >> bit) & 0x01)  && ((memory[IFR] >> bit) & 0x01) ){
        //if ((testBit(IER,bit)) && (testBit(IFR,bit))){
            cpustate.ime = FALSE;           // Disable master Interrupt
            cpustate.halt = FALSE;          // Resume cpu 

            unsigned int mask = (unsigned int)0x01 << bit;
            memory[IFR] &= ~mask;

            hardwareTick();
            hardwareTick();
            /* This is a hack to skip one machine cycle 
             * when waking from halt. cycle should be 
             * skipped from fetching next opcode
             */
            //if (cpustate.interrupt == FALSE){
                //printf("GOTCHA\n");
                //hardwareTick();
            //}
            //else{
                //cpustate.interrupt = FALSE;
            //}

            stackPush16(registers.PC);      // Push program counter in the stack

            switch (bit){                   // Set program counter to intterupt address
                case VBLANK_INTERRUPT:
                    registers.PC = 0x40;
                    //setBit(IFR,bit,FALSE);          // Reset Interrupt request Register
                    return;
                    break;
                case LCDC_INTERRUPT  :
                    registers.PC = 0x48;
                    //setBit(IFR,bit,FALSE);          // Reset Interrupt request Register
                    return;
                    break;
                case TIMER_INTERRUPT :
                    registers.PC = 0x50;
                    //setBit(IFR,bit,FALSE);          // Reset Interrupt request Register
                    return;
                    break;
                case SERIAL_INTERRUPT:
                    registers.PC = 0x58;
                    //setBit(IFR,bit,FALSE);          // Reset Interrupt request Register
                    return;
                    break;
                case JOYPAD_INTERRUPT:
                    registers.PC = 0x60;
                    //setBit(IFR,bit,FALSE);          // Reset Interrupt request Register
                    return;
                    break;
                default:
                    break;
            }           
        }
    }
}
