#include "definitions.h"
#include "interrupts.h"
#include "serial.h"
#include "memory.h"

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

struct serial serialstate = {
    FALSE,      /* enabled */
    0,          /* timer */
    0,          /* clock */
    0           /* totalbits */
};

void serialReset()
{
    serialstate.enabled = FALSE;
    serialstate.timer = 0; /* according to gambatte, initial value is 8 and after bios CC */
    serialstate.clock = 0;
    serialstate.totalbits = 0;
}

void serialSetControl(unsigned char value)
{
    if (value & 0x80){
        serialstate.enabled = TRUE;
        serialstate.clock = value & 0x01;
        serialstate.totalbits = 0;
    } else {
        serialstate.enabled = FALSE;
    }
}


void serialUpdateClock()
{
    serialstate.timer += 1;         /* not sure if this is the correct interval */

    if (serialstate.timer > 0xFF){  /* internal timer is a 8 bit register */
        
        serialstate.timer = 0;      /* handle timer ovlerflow */
        
        if (serialstate.enabled == TRUE){
            if (serialstate.clock == 1){
                serialPushBit();
            }
        }
    }
}

void serialPushBit()
{
    serialstate.totalbits += 1;

    if (serialstate.totalbits > 7){
        
        /* send and receive a byte */
        serialSendByte(memory[0xFF01]);
        memory[0xFF01] = serialReceiveByte();
        
        /* at the end of transfer, bit 7 of SB is automatically set to zero */
        serialstate.enabled = FALSE;
        memory[0xFF02] &= 0x7F;
        
        /* trigger serial interrupt at the end of transfer */
        triggerInterrupt(SERIAL_INTERRUPT);
    }
}


void serialSendByte(unsigned char data)
{
    /* it's a damn dummy */
}

unsigned char serialReceiveByte()
{
    return 0xFF;
}