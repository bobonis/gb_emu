#include "interrupts.h"
#include "memory.h"
#include "cpu.h"
#include "timers.h"
#include <stdio.h>

/*
 * When an interrupt is requested we set the
 * appropriate bit in the Interrupt Request Register
 * If IME was enabled, we serve the next interrupt,
 * else we continue execution
 */
void triggerInterrupt(int interrupt){
    
    if (cpustate.ime == TRUE){
        cpustate.halt = FALSE;
        cpustate.interrupt = TRUE;
    }
    else{
        cpustate.halt = FALSE;
    }

    setBit(IFR, interrupt, TRUE);
}



void handleInterrupts(void){

    int bit;

    for (bit=0;bit<5;bit++){                // Find the requested Interrupt, priority matters
        if ((testBit(IER,bit)) && (testBit(IFR,bit))){
            cpustate.ime = FALSE;           // Disable master Interrupt
            cpustate.halt = FALSE;          // Resume cpu 

            updateTimers(4);
            updateTimers(4);
            /* This is a hack to skip one machine cycle 
             * when waking from halt. cycle should be 
             * skipped from fetching next opcode
             */
            if (cpustate.interrupt == FALSE){
                updateTimers(4);
            }
            else{
                cpustate.interrupt = FALSE;
            }

            stackPush16(registers.PC);      // Push program counter in the stack

            switch (bit){                   // Set program counter to intterupt address
                case VBLANK_INTERRUPT:
                    registers.PC = 0x40; break;
                case LCDC_INTERRUPT  :
                    registers.PC = 0x48; break;
                case TIMER_INTERRUPT :
                    registers.PC = 0x50; break;
                case SERIAL_INTERRUPT:
                    registers.PC = 0x58; break;
                case JOYPAD_INTERRUPT:
                    registers.PC = 0x60; break;
            }

            setBit(IFR,bit,FALSE);          // Reset Interrupt request Register
        }
    }
}
