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


const unsigned char bios[256] = {
//0    1     2     3     4     5     6     7     8     9     A     B     C     D     E     F

0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E, // 0x0
0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0, // 0x1
0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B, // 0x2
0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9, // 0x3
0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, // 0x4
0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04, // 0x5
0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2, // 0x6
0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, // 0x7
0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20, // 0x8
0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17, // 0x9
0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, // 0xA
0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, // 0xB
0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, // 0xC
0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C, // 0xD
0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20, // 0xE
0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50  // 0xF
};


void memCopy(unsigned char *memory, unsigned short start, unsigned char *buffer, unsigned short length){
	unsigned short i;
	
	for (i=start;i<=length;i++){
		memory[i] = buffer[i - start];
	}

	
}

void reset (void){
	
    if (USINGBIOS){
        int i;
        registers.PC = 0x0000;
        for (i=0;i<=255;i++){
            memory[i] = bios[i];
        }
        
    }
    else
    {
	   registers.AF = 0x01B0;
	   registers.BC = 0x0013;
	   registers.DE = 0x00D8;
	   registers.HL = 0x014D;
	   registers.SP = 0xFFFE;
	   registers.PC = 0x0100;
memory[0xFF00] = 0xdf; //wtf
	   memory[0xFF05] = 0x00;	// TIMA
	   memory[0xFF06] = 0x00;	// TMA
//       	   memory[0xFF07] = 0x00;	// TAC should it be 0x00?
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

void add16 (unsigned short value1){
    unsigned long HL_long = registers.HL + value1;
    
    if ((HL_long&0xffff0000) == 0)
        resetFlag(CARRY_F);
    else
        setFlag(CARRY_F);
    
    registers.HL = (unsigned short)(HL_long&0xffff);

    if (((registers.HL&0x0F)+(value1&0x0F)) > 0x0F)
         setFlag(HALF_CARRY_F);
    else
         resetFlag(HALF_CARRY_F);

    resetFlag(SUBSTRACT_F);

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

void cpu_and (unsigned char value1){
    registers.A = registers.A&value1;
    if (registers.A == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
    resetFlag(SUBSTRACT_F);
    setFlag(HALF_CARRY_F);
    resetFlag(CARRY_F);
}

void or (unsigned char value){
    registers.A |= value;
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

void inc (unsigned char *value1){
    
    if ((*value1 & 0x0F) == 0x0F)
        setFlag(HALF_CARRY_F);
    else 
        resetFlag(HALF_CARRY_F);
        
    *value1 = *value1 + 1;
    
    if (*value1 == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);

    resetFlag(SUBSTRACT_F);

}

void dec (unsigned char *value1){
    
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
/*
Imagine a stack which can store 4 elements
At the beginning SP will have an invalid value 5 (memory[5])
When you push two bytes you will decrement SP by 2 in order to point to memory[3]
and you will add the values in memory[3] and memory[4].
The below code will read from memory[2] and memory[1] and will set SP to 5.
Please check my assumption and provide your feedback
*/

//    value = (memory[registers.SP - 1] | (memory[registers.SP - 2] << 8));
//proposed
    value = (memory[registers.SP+1] | (memory[registers.SP] << 8));
    registers.SP += 2;
    return value;
}