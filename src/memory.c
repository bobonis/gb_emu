#include "memory.h"
#include <stdio.h>
#include "timers.h"

#define ZERO_F 			7
#define SUBSTRACT_F		6
#define HALF_CARRY_F	5
#define CARRY_F			4

#define LY      0xFF44 //Vertical line counter

unsigned short test;

struct registers registers;
unsigned char memory[0xFFFF];

void memCopy(unsigned char *memory, unsigned short start, unsigned char *buffer, unsigned short length){
	unsigned short i;
	
	for (i=start;i<=length;i++){
		memory[i] = buffer[i - start];
	}

	
}

void reset (void){
	
	registers.AF = 0x01B0;
	registers.BC = 0x0013;
	registers.DE = 0x00D8;
	registers.HL = 0x014D;
	registers.SP = 0xFFFE;
	registers.PC = 0x0100;
	
	registers.F = 0xB0;

	memory[0xFF05] = 0x00;	// TIMA
	memory[0xFF06] = 0x00;	// TMA
	memory[0xFF07] = 0x04;	// TAC
	memory[0xFF10] = 0x80;	// NR10
	memory[0xFF11] = 0xBF;	// NR11
	memory[0xFF12] = 0xF3;	// NR12
	memory[0xFF14] = 0xBF;	// NR14
	memory[0xFF16] = 0x3F;	// NR21
	memory[0xFF17] = 0x00;	// NR22
	memory[0xFF19] = 0xBF;	// NR24
	memory[0xFF1A] = 0x7F;	// NR30
	memory[0xFF1B] = 0xFF;	// NR31
	memory[0xFF1C] = 0x9F;	// NR32
	memory[0xFF1E] = 0xBF;	// NR33
	memory[0xFF20] = 0xFF;	// NR41
	memory[0xFF21] = 0x00;	// NR42
	memory[0xFF22] = 0x00;	// NR43
	memory[0xFF23] = 0xBF;	// NR30
	memory[0xFF24] = 0x77;	// NR50
	memory[0xFF25] = 0xF3;	// NR51
	memory[0xFF26] = 0xF1;	// NR52
	memory[0xFF40] = 0x91;	// LCDC
	memory[0xFF42] = 0x00;	// SCY
	memory[0xFF43] = 0x00;	// SCX
	memory[0xFF45] = 0x00;	// LYC
	memory[0xFF47] = 0xFC;	// BGP
	memory[0xFF48] = 0xFF;	// OBP0
	memory[0xFF49] = 0xFF;	// OBP1
	memory[0xFF4A] = 0x00;	// WY
	memory[0xFF4B] = 0x00;	// WX
	memory[0xFFFF] = 0x00;	// IE
}

unsigned char readMemory8 (unsigned short address){
    return memory[address];    
}

unsigned short readMemory16 (unsigned short address){
    return (memory[address] | (memory[address + 1] << 8));
}

void setFlag (unsigned char flag){
    switch (flag){
        case 7:
            registers.F = registers.F | 0x80;
            break;
        case 6:
            registers.F = registers.F | 0x40;
            break;
        case 5:
            registers.F = registers.F | 0x20;
            break;
        case 4:
            registers.F = registers.F | 0x10;
            break;
    }
}

void resetFlag (unsigned char flag){
    switch (flag){
        case 7:
            registers.F = registers.F & 0x7F;
            break;
        case 6:
            registers.F = registers.F & 0xBF;
            break;
        case 5:
            registers.F = registers.F & 0xDF;
            break;
        case 4:
            registers.F = registers.F & 0xEF;
            break;
    }
}

int testFlag (unsigned char flag){
    switch (flag){
        case 7:
            if ((registers.F & 0x80) != 0) return 1;
            break;
        case 6:
            if ((registers.F & 0x40) != 0) return 1;
            break;
        case 5:
            if ((registers.F & 0x20) != 0) return 1;
            break;
        case 4:
            if ((registers.F & 0x10) != 0) return 1;
            break;
    }
    
    return 0;
}

void add (unsigned short value1, unsigned short value2){
    if ((value1 + value2) > 255) 
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
    if ((value1 + value2) > 15)  
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);
    
    if ((value1 + value2) == 0)  
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
    
    resetFlag(SUBSTRACT_F);
    
    registers.A = value1 + value2;
}

void adc (unsigned short value1, unsigned short value2){
    if ((value1 + value2 + testFlag(CARRY_F)) > 255) 
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
    
    if ((value1 + value2 + testFlag(CARRY_F)) > 15)  
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);
    
    if ((value1 + value2 + testFlag(CARRY_F)) == 0)  
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);

    resetFlag(SUBSTRACT_F);

    registers.A = value1 + value2 + testFlag(CARRY_F);
}

void xor (unsigned short value1){
    registers.A = registers.A ^ value1;
    if (registers.A == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    resetFlag(CARRY_F);
}

void comp (unsigned char value){
    if (registers.A == value)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
        
    setFlag(SUBSTRACT_F);
    
    if ((value & 0x0f) > (registers.A & 0x0f))
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);
    
    if (registers.A < value)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
    
}

void sub (unsigned char value){

    if (value > registers.A)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);

    setFlag(SUBSTRACT_F);
    
    if ((value & 0x0f) > (registers.A & 0x0f))
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);
    
    if (registers.A == value)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
        
    registers.A -= value;

    
}


void dec (unsigned char *value1){
    // checking register before decremented
    // do we really care about HALF_CARRY??
    if (*value1 & 0x0F)
        resetFlag(HALF_CARRY_F);
    else 
        setFlag(HALF_CARRY_F);
        
    *value1 = *value1 - 1;
    
    if (*value1 == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);

    setFlag(SUBSTRACT_F);

}


void writeMemory (unsigned short pos, unsigned char value){
    memory[pos] = value;
    
    switch (pos){
        case 0xFF07:
            updateFrequency();
            break;
        case LY:    // Writing will reset the counter
            memory[LY] = 0;
    }
}

unsigned char readMemory (unsigned short pos){
    return memory[pos];
}

void setBit(unsigned short pos, unsigned char bit, bool value){
    switch (bit){
        case 0:
            if (value == TRUE) memory[pos] = (memory[pos] | 0x01);
            else               memory[pos] = (memory[pos] & 0xFE);
            break;
        case 1:
            if (value == TRUE) memory[pos] = (memory[pos] | 0x02);
            else               memory[pos] = (memory[pos] & 0xFD);
            break;
        case 2:
            if (value == TRUE) memory[pos] = (memory[pos] | 0x04);
            else               memory[pos] = (memory[pos] & 0xFB);
            break;
        case 3:
            if (value == TRUE) memory[pos] = (memory[pos] | 0x08);
            else               memory[pos] = (memory[pos] & 0xF7);
            break;
        case 4:
            if (value == TRUE) memory[pos] = (memory[pos] | 0x10);
            else               memory[pos] = (memory[pos] & 0xEF);
            break;
        case 5:
            if (value == TRUE) memory[pos] = (memory[pos] | 0x20);
            else               memory[pos] = (memory[pos] & 0xDF);
            break;
        case 6:
            if (value == TRUE) memory[pos] = (memory[pos] | 0x40);
            else               memory[pos] = (memory[pos] & 0xBF);
            break;
        case 7:
            if (value == TRUE) memory[pos] = (memory[pos] | 0x80);
            else               memory[pos] = (memory[pos] & 0x7F);
            break;
    }
}
bool testBit(unsigned short pos, unsigned char bit){
    switch (bit){
        case 0:
            if ((memory[pos] & 0x01) != 0) return TRUE;  
            break;
        case 1:
            if ((memory[pos] & 0x02) != 0) return TRUE; 
            break;
        case 2:
            if ((memory[pos] & 0x04) != 0) return TRUE; 
            break;
        case 3:
            if ((memory[pos] & 0x08) != 0) return TRUE; 
            break;
        case 4:
            if ((memory[pos] & 0x10) != 0) return TRUE;
            break;
        case 5:
            if ((memory[pos] & 0x20) != 0) return TRUE; 
            break;
        case 6:
            if ((memory[pos] & 0x40) != 0) return TRUE; 
            break;
        case 7:
            if ((memory[pos] & 0x80) != 0) return TRUE; 
            break;
    }
    return FALSE;
}

void stackPush16 (unsigned short value){
    registers.SP--;                            // Decrease stack pointer
    memory[registers.SP] = (value & 0x00FF);   // Push low part in the stack
    registers.SP--;                            // Decrease stack pointer again
    memory[registers.SP] = ((value & 0xFF00) >> 8); // Push high part in the stack
}

unsigned short stackPop16 (void){
    unsigned short value = 0;
    value = (memory[registers.SP - 1] | (memory[registers.SP - 2] << 8));
    registers.SP += 2;
}