#include "interrupts.h"
#include "memory.h"

unsigned char interruptMaster;


/*
 * When an interrupt is requested we set the
 * appropriate bit in the Interrupt Request Register
 */
void triggerInterrupt(int interrupt){
    setBit(IFR, interrupt, TRUE);
}



void handleInterrupts(void){
    
    int bit;
    
    if (interruptMaster == TRUE){ // Check that Interrupts are enabled
        for (bit=0;bit<5;bit++){  // Find the requested Interrupt, priority matters
            if ((testBit(IER,bit)) && (testBit(IFR,bit))){
                
                interruptMaster = FALSE;   // Disable master Interrupt
                stackPush16(registers.PC); // Push program counter in the stack
                
                switch (bit){              // Set program counter to intterupt address
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
                setBit(IFR,bit,FALSE);     // Reset Interrupt request Register
            }
        }
    }
}