#include "memory.h"
#include <stdio.h>
#include "timers.h"
#include "input.h"
#include "rom.h"

#define ZERO_F 			7
#define SUBSTRACT_F		6
#define HALF_CARRY_F	5
#define CARRY_F			4

#define LY      0xFF44 //Vertical line counter


unsigned short test;

struct registers registers;
unsigned char memory[0xFFFF];
unsigned char memory_backup[256];

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
            memory_backup[i] = memory[i];   // backup address space that is overwitten from bios
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
	   memory[0xFF05] = 0x00;	// TIMA
	   memory[0xFF06] = 0x00;	// TMA
	   memory[0xFF07] = 0x00;	// TAC
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

    int address_map;

    if (( address >= 0x4000 ) && ( address <= 0x7FFF )){ //ROM Memory Bank
        //address_map = address - 0x4000;
        //address_map = address_map + (0x4000 * active_ROM_bank); //move address space to correct Memory Bank
        //printf("cart_ROM %x[%x]=%x\n",address_map,active_ROM_bank,cart_ROM[address_map]);
        return cart_ROM[(active_ROM_bank << 14) + (address - 0x4000)]; //SHL 14 is the same than *16384 (but faster) thx ZBOY
    }
    else if (( address >= 0xA000 ) && ( address <= 0xBFFF )){ //RAM Memory Bank
        address -= 0xA000;
        address += 0x2000 * active_RAM_bank; //move address space to correct RAM Bank
        return cart_RAM[address];
    }
    else if (address == 0xFF00){ //Read Joypad
        return inputReadKeys();
    }
    else {
        return memory[address];
    }
}

unsigned short readMemory16 (unsigned short address){
    
    return (readMemory8(address) | (readMemory8(address + 1) << 8));

}

void writeMemory16 (unsigned short pos, unsigned short value){
    
    writeMemory(pos, (value & 0xFF));
    writeMemory(pos + 1, (value >> 8));

}

void writeMemory (unsigned short pos, unsigned char value){


    if (pos < 0x8000){
        cartridgeSwitchBanks(pos, value);
    }
    else if (pos == 0xFF07){
        memory[pos] = value;    // forgot to write new value..
        updateFrequency();
    }
    /* Writing the value of 1 to the address 0xFF50 unmaps the boot ROM, 
     * and the first 256 bytes of the address space, where it effectively 
     * was mapped, now gets mapped to the beginning of the cartridge’s ROM.
     */
    else if ((pos == 0xFF50) && (value == 0x01)){
        memCopy(memory, 0x0000, memory_backup, 0xFF);
    } 
    else if (pos == 0xFF46){    //DMA
        directMemoryAccess(value);
    }
    else if (pos == LY){        // Writing will reset the counter
        memory[LY] = 0;
    }
    else if (( pos >= 0xA000 ) && ( pos <= 0xBFFF )){ //RAM Memory Bank
        pos -= 0xA000;
        pos += 0x2000 * active_RAM_bank; //move address space to correct RAM Bank
        cart_RAM[pos] = value;
    }    
    else if ( ( pos >= 0xE000 ) && (pos < 0xFE00) ){ // writing to ECHO ram also writes in RAM
        memory[pos] = value ;
        writeMemory(pos - 0x2000, value) ;
    }
    else if ( ( pos >= 0xC000 ) && (pos < 0xE000) ){ // writing to RAM also writes in ECHO RAM or not???
        memory[pos] = value ;
        //memory[pos + 0x2000] = value;
    }
    else if ( ( pos >= 0xFEA0 ) && (pos < 0xFEFF) ){ // this area is restricted
    }
    else if (pos == 0xFF47){
        //printf("[DEBUG] Write at palette 0x%2x\n",value);
        memory[pos] = value;
    }
    else{ //default
        memory[pos] = value;
    }

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
    writeMemory( registers.SP, (value & 0xFF00) >> 8); // Push high part in the stack
    registers.SP--;                            // Decrease stack pointer again
    writeMemory( registers.SP, value & 0x00FF );   // Push low part in the stack    
}

unsigned short stackPop16 (void){
    unsigned short value = 0;

    value = (readMemory8(registers.SP) | (readMemory8(registers.SP + 1) << 8));
    registers.SP += 2;
    return value;
}


void directMemoryAccess(unsigned char value){
    int i;
    unsigned short address = value << 8 ; // source address is data * 100
    for (i = 0 ; i < 0xA0; i++){
        writeMemory(0xFE00+i, readMemory8(address+i)) ;
    }
} 