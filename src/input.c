#include "input.h"
#include "memory.h"
#include <stdio.h>

/*
 * 0 = key pressed / 1 = key not pressed
 * ---------------------------
 * Bit 7 - START
 * Bit 6 - SELECT
 * Bit 5 - BUTTON B
 * Bit 4 - BUTTON A
 * Bit 3 - DOWN
 * Bit 2 - UP
 * Bit 1 - LEFT
 * Bit 0 - RIGHT
 */
unsigned char joypad = 0xFF; //Initial value, no keys are pressed

/*
 * P1 (0xFF00) register layout
 * ---------------------------
 * Bit 7 - Not used
 * Bit 6 - Not used
 * Bit 5 - P15 Select Button Keys (0=Select)
 * Bit 4 - P14 Select Direction Keys (0=Select)
 * Bit 3 - P13 Input Down or Start (0=Pressed) (Read Only)
 * Bit 2 - P12 Input Up or Select (0=Pressed) (Read Only)
 * Bit 1 - P11 Input Left or Button B (0=Pressed) (Read Only)
 * Bit 0 - P10 Input Right or Button A (0=Pressed) (Read Only) 
*/
unsigned char inputReadKeys(){

    unsigned char joypad_state = 0xFF;

    
    if (testBit(0xFF00, 4) == 0){    //Direction Keys selected
        joypad_state = joypad & 0x0F;   //Get 4 lower bits that represent direction keys
        joypad_state = joypad_state | 0xF0;
        joypad_state &= 0xEF; //Set BIT 4 = 0
        //resetBit(joypad_state,4);
    }
    else if (testBit(0xFF00, 5) == 0){ //Standard keys selected
        joypad_state = joypad & 0xF0;   //Get 4 lower bits that represent button keys
        joypad_state = (joypad_state >> 4) | 0xF0; //Set bits in H1 to 1
        joypad_state &= 0xDF; //Set BIT 5 = 0
        //resetBit(joypad_state,5);
    }
    //printf ("\nJoypad = %d\n",joypad_state);
    return joypad_state;
    

}