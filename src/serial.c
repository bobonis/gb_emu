#include "definitions.h"
#include "interrupts.h"

/*
 * FF01 - SB - Serial transfer data (R/W)
 * 8 Bits of data to be read/written
 * 
 * FF02 - SC - Serial Transfer Control (R/W)
 * 
 *   Bit 7 - Transfer Start Flag (0=No Transfer, 1=Start)
 *   Bit 1 - Clock Speed (0=Normal, 1=Fast) ** CGB Mode Only **
 *   Bit 0 - Shift Clock (0=External Clock, 1=Internal Clock)
 */

int enabled = 0;
unsigned short timer = 0;

void serialSetControl(unsigned char value)
{
    if (value & 0x80){
        enabled = 1;
    }
    else{
        enabled = 0;
        timer = 0;
    }
}


void serialUpdateClock()
{
    if (enabled){
        timer += 2;

    
        if (timer >> 8){
            timer = 0;
            enabled = 0;
            triggerInterrupt(SERIAL_INTERRUPT);
        }
    }
}
