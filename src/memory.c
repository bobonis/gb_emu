#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include "timers.h"
#include "input.h"
#include "rom.h"
#include "gpu.h"
#include "hardware.h"
#include "definitions.h"
#include "sound.h"
#include "serial.h"

#ifndef SOUND
void write_memory_apu(unsigned short address,unsigned char value){

}
unsigned char read_memory_apu(unsigned short address){
    return memory[address];
}
#endif

struct registers registers;
/*
 * Interrupt Enable Register
 * ---------------------------  FFFF
 * Internal RAM
 * ---------------------------  FF80
 * Empty but unusable for I/O
 * ---------------------------  FFC4
 * I/O ports
 * ---------------------------  FF00
 * Empty but unusable for I/O
 * ---------------------------  FEA0
 * Sprite Attrib Memory (OAM)
 * ---------------------------  FE00
 * Echo of 8kB Internal RAM
 * ---------------------------  E000
 * 8kB Internal RAM
 * ---------------------------  C000
 * 8kB switchable RAM bank
 * ---------------------------  A000
 * 8kB Video RAM
 * ---------------------------  8000  --
 * 16kB switchable ROM bank             |
 * ---------------------------  4000    | -- 32 kB Cartrige
 * 16kB ROM bank #0                     |
 * ---------------------------  0000  --
 */
unsigned char memory[0x10000];              /* 64KB System RAM Memory */
unsigned char memory_SRAM[512];



int gpu_reading = 0;

struct dma dmastate;

int sram_active =  0;              /* 1: SRAM cart is active 
                                      0: save file is active */
                                   
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



/**
 * memoryCopy - copies a number of buffer positions into main gameboy memory
 *
 * @start	the position where the copy starts
 * @buffer	the buffer to read from
 * @length	number of elements to be copied
 */
void memoryCopy(unsigned short start, unsigned char *buffer, unsigned short length)
{
    int i;
	
	for (i = start; i <= length; i++){
        memory[i] = buffer[i - start];
    }
}

/**
 * memoryReset - Clear main memory and set initial values to control registers
 *      Handles startup with or without bios.
 */
void memoryReset(void)
{
    int i;
    
    /*
     * Clear main memory. This action shouldn't be needed, but it seems we are missing
     * some register initializations
     */
    for (i = 0; i < 0x10000; i++)
        memory[i] = 0;
    
    /* load 16kb ROM bank 0 */
    memoryCopy(0x0000, cart_ROM, 0x3FFF);
    
    if (USINGBIOS){
        registers.PC = 0x0000;
        /* map the bios in the main memory */
        for (i = 0; i <= 255; i++){
            memory[i] = bios[i];
        }
        /* init cartrige ram */
        for (i = 0; i < 512; i++){
            memory_SRAM[i] = 0xFF;
        }

        memory[0xFF00] = 0xCF;    
        memory[0xFF07] = 0xF8;
        memory[0xFF10] = 0x80;	/* NR10 */
	    memory[0xFF11] = 0xBF;	/* NR11 */
	    memory[0xFF12] = 0xF3;	/* NR12 */
	    memory[0xFF14] = 0xBF;	/* NR14 */
	    memory[0xFF16] = 0x3F;	/* NR21 */
	    memory[0xFF17] = 0x00;	/* NR22 */
	    memory[0xFF19] = 0xBF;	/* NR24 */
	    memory[0xFF1A] = 0x7F;	/* NR30 */
	    memory[0xFF1B] = 0xFF;	/* NR31 */
	    memory[0xFF1C] = 0x9F;	/* NR32 */
	    memory[0xFF1E] = 0xBF;	/* NR33 */
	    memory[0xFF20] = 0xFF;	/* NR41 */
	    memory[0xFF21] = 0x00;	/* NR42 */
	    memory[0xFF22] = 0x00;	/* NR43 */
	    memory[0xFF23] = 0xBF;	/* NR30 */
	    memory[0xFF24] = 0x77;	/* NR50 */
	    memory[0xFF25] = 0xF3;	/* NR51 */
	    memory[0xFF26] = 0xF1;	/* NR52 */
        memory[0xFF0F] = 0xE0;
        memory[0xFFFF] = 0xE0;
        memory[0xFF46] = 0xFF;
        memory[0xFF48] = 0xFF;
        memory[0xFF49] = 0xFF;
        memory[0xFF4F] = 0xFF;
        memory[0xFF68] = 0xFF;
        memory[0xFF6A] = 0xFF;
        memory[0xFF72] = 0xFF;
        memory[0xFF73] = 0xFF;
        memory[0xFF75] = 0xFF;
        memory[0xFF76] = 0xFF;
        memory[0xFF77] = 0xFF;
    } else {
	   registers.AF = 0x01B0;
	   registers.BC = 0x0013;
	   registers.DE = 0x00D8;
	   registers.HL = 0x014D;
	   registers.SP = 0xFFFE;
	   registers.PC = 0x0100;
	   memory[0xFF05] = 0x00;	/* TIMA */
	   memory[0xFF06] = 0x00;	/* TMA */
	   memory[0xFF07] = 0x00;	/* TAC */
	   memory[0xFF10] = 0x80;	/* NR10 */
	   memory[0xFF11] = 0xBF;	/* NR11 */
	   memory[0xFF12] = 0xF3;	/* NR12 */
	   memory[0xFF14] = 0xBF;	/* NR14 */
	   memory[0xFF16] = 0x3F;	/* NR21 */
	   memory[0xFF17] = 0x00;	/* NR22 */
	   memory[0xFF19] = 0xBF;	/* NR24 */
	   memory[0xFF1A] = 0x7F;	/* NR30 */
	   memory[0xFF1B] = 0xFF;	/* NR31 */
	   memory[0xFF1C] = 0x9F;	/* NR32 */
	   memory[0xFF1E] = 0xBF;	/* NR33 */
	   memory[0xFF20] = 0xFF;	/* NR41 */
	   memory[0xFF21] = 0x00;	/* NR42 */
	   memory[0xFF22] = 0x00;	/* NR43 */
	   memory[0xFF23] = 0xBF;	/* NR30 */
	   memory[0xFF24] = 0x77;	/* NR50 */
	   memory[0xFF25] = 0xF3;	/* NR51 */
	   memory[0xFF26] = 0xF1;	/* NR52 */
	   memory[0xFF40] = 0x91;	/* LCDC */
       memory[0xFF41] = 0x80;   /* LCDS */
	   memory[0xFF42] = 0x00;	/* SCY */
	   memory[0xFF43] = 0x00;	/* SCX */
	   memory[0xFF45] = 0x00;	/* LYC */
	   memory[0xFF47] = 0xFC;	/* BGP */
	   memory[0xFF48] = 0xFF;	/* OBP0 */
	   memory[0xFF49] = 0xFF;	/* OBP1 */
	   memory[0xFF4A] = 0x00;	/* WY */
	   memory[0xFF4B] = 0x00;	/* WX */
	   memory[0xFFFF] = 0x00;	/* IE */     
    }
}

/**
 * memoryDMAReset - Initialize DMA control
 */
void memoryDMAReset(void)
{
    dmastate.timer = 0;
    dmastate.prepare = FALSE;
    dmastate.start = FALSE;
    dmastate.running = FALSE;
    dmastate.address = 0x0000;
}



/******************************************************************************
 * Functions to read gameboy memory                                           *
 *****************************************************************************/


unsigned char readMemory8(unsigned short address)
{
    unsigned char data;
    
    int address_map;
   
    switch (address >> 12) {    /* Get most significant 4 bits */
        case 0x0 ... 0x3 :                                          /* 16kB ROM bank              */
            data = memory[address];
            break;
        case 0x4 ... 0x7 :                                          /* 16kB switchable ROM bank   */
            data = readMemoryROMBank(address);
            break;
        case 0x8 ... 0x9 :                                          /* 8kB Video RAM              */
            data = readMemoryVRAM(address);
            break;
        case 0xA ... 0xB :                                          /* switchable RAM bank        */
            data = readMemoryRAMBank(address);
            break;
        case 0xC ... 0xD :                                          /* 8kB Internal RAM           */
            data = readMemoryWRAM(address);
            break;
        case 0xE :                                                  /* 8kB Echo of Internal RAM   */
            data = readMemoryECHORAM(address);
            break;
        case 0xF :
            if (address < 0xFE00) {                                 /* 8kB Echo of Internal RAM   */
                data = readMemoryECHORAM(address);
            } else if ((address >= 0xFE00) && (address < 0xFEA0)) { /* Sprite Attrib Memory (OAM) */
                data = readMemoryOAM(address);
            } else if ((address >= 0xFEA0) && (address < 0xFF00)) { /* Empty but unusable for I/O */
                /* Reading from this area returns 00h. */
                data = 0x00;
            } else if ((address >= 0xFF00) && (address < 0xFF80)) { /* I/O ports                  */
                data = readMemoryIOPorts(address);
            } else if ((address >= 0xFF80) && (address < 0xFFFF)) { /* Zero Page 127 Bytes        */
                data = memory[address];
            } else if (address == 0xFFFF) {                         /* Interrupt Enable Register  */
                data = memory[address];
            }
            break;
        default :
            data = memory[address];
            break;
    }
    
    if (!gpu_reading)
        hardwareTick();
                
    return data;
}

/**
 * readMemoryROMBank - read from switchable ROM bank
 *
 * @address	the position to read from
 */
unsigned char readMemoryROMBank(unsigned short address)
{
    int address_map;
    
    address_map = address - 0x4000;
    address_map = address_map + (0x4000 * active_ROM_bank); /* move address space to correct Memory Bank */
    /* temp = cart_ROM[(active_ROM_bank << 14) + (address - 0x4000)]; //SHL 14 is the same than *16384 (but faster) thx ZBOY */

    return cart_ROM[address_map];
}

/**
 * readMemoryRAMBank - read from switchable RAM bank
 *
 * @address	the position to read from
 */
unsigned char readMemoryRAMBank(unsigned short address)
{
    unsigned char data;
    FILE *fp;
    
    /* if Ram bank is not enabled reads return 0xFF */
    if (!RAM_bank_enabled)
        return 0xFF;

    switch (MBC_type) {
    case MBC1:
        address -= 0xA000;
        address += 0x2000 * active_RAM_bank; /* move address space to correct RAM Bank */
        data = cart_RAM[address];
        break;
    case MBC2:
        if (( address >= 0xA000 ) && ( address <= 0xA1FF )) {
            if (!sram_active) {
                fp = fopen(cart_game, "rb");
                if (fp) {
                    sram_active = 1;
                    fread(memory_SRAM, 1, 512, fp); 
                    /* Only the 4 lower bits are used, the upper bits should be ignored when reading */
                    data = memory_SRAM[address & 0x1FF] | 0xF0;  
                    fclose(fp);
                } else {
                    data = 0xFF;
                    printf("[ERROR] Cannot open SRAM file\n");
                }
            } else {
                /* Only the 4 lower bits are used, the upper bits should be ignored when reading */
                data = memory_SRAM[address & 0x1FF] | 0xF0;  
            }
        } else {
            data = 0xFF; /* if no RAM is mapped reads return 0xFF */
        }
        break;
    case MBC3:
        if (RTC_register_enabled == FALSE) {
            address -= 0xA000;
            address += 0x2000 * active_RAM_bank; /* move address space to correct RAM Bank */
            data = cart_RAM[address];
        } else {
            printf("[MEM] Read from RTC register\n");
            data = 0x00;
        }
        break;
    default:
        data = 0xFF;    /* if no RAM is mapped reads return 0xFF */
        break;
    }

    return data;
}

/**
 * readMemoryVRAM - read from Video RAM. During LCDC mode 3 VRAM can't be accessed.
 *
 * @address	the position to read from
 */
unsigned char readMemoryVRAM(unsigned short address)
{
    unsigned char data;
    
    if ((memory[STAT] & 0x03) == SCAN_VRAM)
        data = 0xFF;
    else
        data = memory[address];

    return data;
}

/**
 * readMemoryWRAM - handle reads from internal RAM
 *
 * @address	the position to read from
 */
unsigned char readMemoryWRAM(unsigned short address)
{
    unsigned char data;
    
    data = memory[address];
    
    return data;
}

/**
 * writeMemoryECHORAM - handle reads to ECHO RAM
 *
 * In CGB/AGB/AGS this area is just a mirror of the WRAM. Reads and writes 
 * from this area are redirected to C000h – DDFFh. In DMG/MGB(/SGB?) this 
 * area works a bit different. With normal cartridges, it has the same behaviour 
 * as the CGB one. With some flashcarts and probably some pirate cartridges both 
 * WRAM and SRAM are active when accessing this area
 *
 * Reads from this area are calculated by reading from WRAM and SRAM, and then 
 * performing a bitwise AND to the two values.
 *
 * @address	the position to read from
 */
unsigned char readMemoryECHORAM(unsigned short address)
{
    unsigned char data;
    
    address -= 0x2000; /* Redirect reads to WRAM */
    data = memory[address];
    
    return data;
}


/**
 * readMemoryIOPorts - read IO registers.
 *
 * @address	the position to read from
 */
unsigned char readMemoryIOPorts(unsigned short address)
{
    unsigned char data;
 
    switch (address) {
    case 0xFF00 :                   /* P1 */ 
        data = inputReadKeys();
        memory[0xFF00] = data;
        break;
    case 0xFF01 :                   /* SB */
        data = memory[address];
        break;
    case 0xFF02 :                   /* SC */
        data = memory[address];
        data |= 0x7E;               /* BIT 6,5,4,3,2,1 Not Used */
        break;
    case 0xFF04 :                   /* DIV */
        data = timersGetDIV();
        break;
    case 0xFF05 :                   /* TIMA */
        data = timersGetTIMA();
        break;
    case 0xFF06 :                   /* TMA */
        data = timersGetTMA();
        break;    
    case 0xFF07 :                   /* TAC */
        data = timersGetTAC();      /* BIT 7,6,5,4,3 Not Used */
        break;
    case 0xFF0F :                   /* IF */
        data = memory[address];
        data |= 0xE0;               /* BIT 7,6,5 Not Used */
        break;
    case 0xFF10 :                   /* NR10 */
        data = read_memory_apu(address);
        data |= 0x80;               /* BIT 7 Not Used */
        break;
    case 0xFF11 :                   /* NR11 */
        data = read_memory_apu(address);
        data |= 0x3F;               /* Only Bits 7-6 can be read */
        break;
    case 0xFF12 :                   /* NR12 */
        data = read_memory_apu(address);
        break;
    case 0xFF13 :                   /* NR13 */
        data = read_memory_apu(address);
        data |= 0xFF;               /* Cant be read */
        break;
    case 0xFF14 :                   /* NR14 */
        data = read_memory_apu(address);
        data |= 0xBF;               /* Only Bit 6 can be read */
        break;
    case 0xFF16 :                   /* NR21 */
        data = read_memory_apu(address);
        data |= 0x3F;               /* Only bits 7-6 can be read */
        break;
    case 0xFF17 :                   /* NR22 */
        data = read_memory_apu(address);
        break;
    case 0xFF18 :                   /* NR23 */
        data = read_memory_apu(address);
        data |= 0xFF;               /* Cant be read */
        break;
    case 0xFF19 :                   /* NR24 */
        data = read_memory_apu(address);
        data |= 0xBF;               /* Only Bit 6 can be read */
        break;
    case 0xFF1A :                   /* NR30 */
        data = read_memory_apu(address);
        data |= 0x7F;               /* BIT 6,5,4,3,2,1,0 Not Used */
        break;
    case 0xFF1B :                   /* NR31 */
        data = read_memory_apu(address);
        data |= 0xFF;               /* Cant be read */
        break;
    case 0xFF1C :                   /* NR32 */
        data = read_memory_apu(address);
        data |= 0x9F;               /* BIT 7,4,3,2,1,0 Not Used */
        break;
    case 0xFF1D :                   /* NR33 */
        data = read_memory_apu(address);
        data |= 0xFF;               /* Cant be read */
        break;
    case 0xFF1E :                   /* NR34 */
        data = read_memory_apu(address);
        data |= 0xBF;               /* Only Bit 6 can be read */
        break;
    case 0xFF20 :                   /* NR41 */
        data = read_memory_apu(address);
        data |= 0xFF;               /* Cant be read */
        break;
    case 0xFF21 :                   /* NR42 */
        data = read_memory_apu(address);
        break;
    case 0xFF22 :                   /* NR43 */
        data = read_memory_apu(address);
        break;
    case 0xFF23 :                   /* NR44 */
        data = read_memory_apu(address);
        data |= 0xBF;               /* Only Bit 6 can be read */
        break;
    case 0xFF24 :                   /* NR50 */
        data = read_memory_apu(address);
        break;
    case 0xFF25 :                   /* NR51 */
        data = read_memory_apu(address);
        break;
    case 0xFF26 :                   /* NR52 */
        data = read_memory_apu(address);
        data |= 0x70;               /* Only Bit 7,3,2,1,0 can be read */
        break;
    case 0xFF30 ... 0xFF3F :        /* WAVE pattern RAM */
        data = read_memory_apu(address);
        break;                       
    case 0xFF40 :                   /* LCDC */
        data = memory[address];
        break;            
    case 0xFF41 :                   /* STAT */
        data = memory[address];
        data |= 0x80;               /* BIT 7 Not Used */
        break;
    case 0xFF42 :                   /* SCY */
        data = memory[address];
        break;
    case 0xFF43 :                   /* SCX */
        data = memory[address];
        break;
    case 0xFF44 :                   /* LY */
        data = memory[address];
        break;
    case 0xFF45 :                   /* LYC */
        data = memory[address];
        break;            
    case 0xFF46 :                   /* DMA */
        if (dmastate.running)
            data = 0xFF;
        else
            data = memory[address];
        break;
    case 0xFF47 :                   /* BGP */
        data = memory[address];
        break;
    case 0xFF48 :                   /* OBP0 */
        data = memory[address];
        break;
    case 0xFF49 :                   /* OBP1 */
        data = memory[address];
        break;
    case 0xFF4A :                   /* WY */
        data = memory[address];
        break;
    case 0xFF4B :                   /* WX */
        data = memory[address];
        break;
    case 0xFF76 :                   /* ???? */
        data = memory[address];
        data |= 0xFF;               /* Not Readable */
        break;
    case 0xFF77 :                   /* ???? */
        data = memory[address];
        data |= 0xFF;               /* Not Readable */
        break;
    default :
        data = 0xFF;
        break;  
    }
    
    return data;
}

/**
 * readMemoryOAM - read from sprite attribute memory
 *
 * - Can't be read when DMA is active
 * - Can't be read during GPU mode 2 and 3
 * @address	the position to read from
 */
unsigned char readMemoryOAM(unsigned short address)
{
    unsigned char data;
 
    if (dmastate.running) {
        printf("[OAM] Tried reading OAM while DMA transfer\n");
        data = 0xFF;
    } else if (gpustate.mode == 2 || gpustate.mode == 3) {
        printf("[OAM] Tried reading OAM while unreadable\n");
        data = 0xFF;
    } else {
        data = memory[address];
    }    

    return data;
}

/**
 * readMemory16 - read 2 bytes from memory
 *
 * @address	the position to read from
 */
unsigned short readMemory16(unsigned short address)
{
    return (readMemory8(address) | (readMemory8(address + 1) << 8));
}



/******************************************************************************
 * Functions to write gameboy memory                                          *
 *****************************************************************************/



void writeMemory8(unsigned short address, unsigned char data)
{
    switch (address >> 12) {    /* Get most significant 4 bits */
        case 0x0 ... 0x7 :                                          /* Cartrige Memory Map        */
            /* Writes to ROM area are interpreted by the Memory Bank Controller (MBC) chip */
            cartridgeSwitchBanks(address, data);
            break;
        case 0x8 ... 0x9 :                                          /* 8kB Video RAM              */
            writeMemoryVRAM(address, data);
            break;
        case 0xA ... 0xB :                                          /* switchable RAM bank        */
            writeMemoryRAMBank(address, data);
            break;
        case 0xC ... 0xD :                                          /* 8kB Internal RAM           */
            writeMemoryWRAM(address, data);
            break;
        case 0xE :                                                  /* 8kB Echo of Internal RAM   */
            writeMemoryECHORAM(address, data);
            break;
        case 0xF :
            if (address < 0xFE00) {                                 /* 8kB Echo of Internal RAM   */
                writeMemoryECHORAM(address, data);
            } else if ((address >= 0xFE00) && (address < 0xFEA0)) { /* Sprite Attrib Memory (OAM) */
                writeMemoryOAM(address, data);
            } else if ((address >= 0xFEA0) && (address < 0xFF00)) { /* Empty but unusable for I/O */
                /* writes are ignored */
            } else if ((address >= 0xFF00) && (address < 0xFF80)) { /* I/O ports                  */
                writeMemoryIOPorts(address, data);
            } else if ((address >= 0xFF80) && (address < 0xFFFF)) { /* Zero Page 127 Bytes        */
                memory[address] = data;
            } else if (address == 0xFFFF) {                         /* Interrupt Enable Register  */
                memory[address] = data;
            }
            break;
        default :
            memory[address] = data;
            break;
    }

    if (!gpu_reading)
        hardwareTick();
}

/**
 * writeMemoryVRAM - handle writes to VRAM
 *
 * - Can't be writen during GPU mode 3
 *
 * @address	the position to write to
 */
void writeMemoryVRAM(unsigned short address, unsigned char data)
{
    if (gpustate.mode != 3)
        memory[address] = data;
}

/**
 * writeMemoryVRAM - handle writes to cartridge RAM banks
 *
 * @data    a byte of information to be written
 * @address	the position to write to
 */
void writeMemoryRAMBank(unsigned short address, unsigned char data)
{
    if (RAM_bank_enabled) {
        switch (MBC_type) {
            case MBC1 :
                address -= 0xA000;
                address += 0x2000 * active_RAM_bank; /*move address space to correct RAM Bank */
                cart_RAM[address] = data;
                break;
            case MBC2 :
                if ( address <= 0xA1FF ) {
                    address &= 0x1FF;
                    /* Only the 4 lower bits are used, the upper bits should be ignored */
                    memory_SRAM[address] = data & 0x0F;
                    sram_active = 1;
                } else {
                    /* I'm not sure if this is writtable
                    memory[address] = data; */
                }
                break;
            case MBC3 :
                if (RTC_register_enabled == FALSE) {
                    address -= 0xA000;
                    address += 0x2000 * active_RAM_bank; /*move address space to correct RAM Bank */
                    cart_RAM[address] = data;  
                } else {
                    printf("[MEM] Write to RTC register\n");
                }
            default :
                /* if no RAM is mapped there is nowere to write */
                break;
        }
    }  
}

/**
 * writeMemoryWRAM - handle writes to internal RAM
 *
 * @data    a byte of information to be written
 * @address	the position to write to
 */
void writeMemoryWRAM(unsigned short address, unsigned char data)
{
    memory[address] = data;
}

/**
 * writeMemoryECHORAM - handle writes to ECHO RAM
 *
 * In CGB/AGB/AGS this area is just a mirror of the WRAM. Reads and writes 
 * from this area are redirected to C000h – DDFFh. In DMG/MGB(/SGB?) this 
 * area works a bit different. With normal cartridges, it has the same behaviour 
 * as the CGB one. With some flashcarts and probably some pirate cartridges both 
 * WRAM and SRAM are active when accessing this area
 *
 * Writes to this area are redirected to both WRAM (C000h – DDFFh) and 
 * SRAM (A000h – BDFFh) areas. This is probably caused because both chip enable 
 * signals are set to '1' because of a too simple chip selection circuit.
 *
 * @data    a byte of information to be written
 * @address	the position to write to
 */
void writeMemoryECHORAM(unsigned short address, unsigned char data)
{
    address -= 0x2000; /* Redirect writes to WRAM */
    memory[address] = data;
}

/**
 * writeMemoryOAM - write to sprite attribute memory
 *
 * - Can't be written when DMA is active
 * - Can't be written during GPU mode 2 and 3
 *
 * @data    a byte of information to be written
 * @address	the position to read from
 */
void writeMemoryOAM(unsigned short address, unsigned char data)
{
    if (dmastate.running) {
        printf("[DMA] Tried writing OAM while DMA active\n");
    } else if (gpustate.mode == 2 || gpustate.mode == 3) {
        printf("[DMA] Tried writing OAM in mode %d\n",gpustate.mode);
    } else {
        memory[address] = data;
    }
}

/**
 * writeMemoryIOPorts - write IO registers.
 *
 * @data    a byte of information to be written
 * @address	the position to write to
 */
void writeMemoryIOPorts(unsigned short address, unsigned char data)
{
    switch (address) {
    case 0xFF00 :                   /* P1 */ 
        data |= 0xC0;               /* BIT 7,6 Not Used */
        data &= 0xF0;               /* BIT 3,2,1,0 Not Writable */
        memory[address] &= 0x0F;    /* clear upper half */
        memory[address] |= data;    /* set upper half */
        inputCheckInterrupt();
        break;
    case 0xFF01 :                   /* SB */
        memory[address] = data;
        break;
    case 0xFF02 :                   /* SC */
        data |= 0x7E;               /* BIT 6,5,4,3,2,1 Not Used */
        serialSetControl(data);
        memory[address] = data;
        break;
    case 0xFF04 :                   /* DIV */
        timersSetDIV();
        break;
    case 0xFF05 :                   /* TIMA */
        timersSetTIMA(data);
        break;
    case 0xFF06 :                   /* TMA */
        timersSetTMA(data);
        break;    
    case 0xFF07 :                   /* TAC */
        timersSetTAC(data);
        break;
    case 0xFF0F :                   /* IF */
        data |= 0xE0;               /* BIT 7,6,5 Not Used */
        memory[address] = data;
        break;
    case 0xFF10 :                   /* NR10 */
        data |= 0x80;               /* BIT 7 Not Used */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF11 :                   /* NR11 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF12 :                   /* NR12 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF13 :                   /* NR13 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF14 :                   /* NR14 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF16 :                   /* NR21 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF17 :                   /* NR22 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF18 :                   /* NR23 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF19 :                   /* NR24 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF1A :                   /* NR30 */
        data |= 0x7F;               /* BIT 6,5,4,3,2,1,0 Not Used  */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF1B :                   /* NR31 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF1C :                   /* NR32 */
        data |= 0x9F;               /* BIT 7,4,3,2,1,0 Not Used  */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF1D :                   /* NR33 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF1E :                   /* NR34 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF20 :                   /* NR41 */
        data |= 0xC0;               /* BIT 7,6 Not Used  */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF21 :                   /* NR42 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF22 :                   /* NR43 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF23 :                   /* NR44 */
        data |= 0x3F;               /* BIT 5,4,3,2,1,0 Not Used  */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF24 :                   /* NR50 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF25 :                   /* NR51 */
        write_memory_apu(address,data);
        memory[address] = data;
        break;
    case 0xFF26 :                   /* NR52 */
        data |= 0x70;               /* BIT 6,5,4 Not Used */
        write_memory_apu(address,data);
        memory[address] = data | 0x01; /* Nasty Hack */
        break;
    case 0xFF30 ... 0xFF3F :        /* WAVE pattern RAM */
        write_memory_apu(address,data);
        memory[address] = data;
        break;                       
    case 0xFF40 :                   /* LCDC */
        gpuSetStatus(data);
        memory[address] = data;
        break;            
    case 0xFF41 :                   /* STAT */
        data |= 0x80;               /* BIT 7 Not Used */
        data &= 0xFC;
        data |= memory[address] & 0x03;    /* bobonis assumes 3 LSB cannot be set */
        memory[address] = data;
        gpuCheckStatSignal();
        
        //Road Rush 
        /*if(gpustate.enable && ((gpustate.mode == 0) || (gpustate.mode == 0)))
        {
            printf("[MEM] Trigger IRQ from STAT Bug, mode = %d, LY = %d\n",gpustate.mode, gpustate.line);
            triggerInterrupt(LCDC_INTERRUPT);
        }*/
        break;
    case 0xFF42 :                   /* SCY */
        memory[address] = data;
        break;
    case 0xFF43 :                   /* SCX */
        memory[address] = data;
        break;
    case 0xFF44 :                   /* LY */
        memory[address] = 0x00;     /* Writing will reset the counter */
        memory[LY] = 0;
        gpustate.line = 0;
        break;
    case 0xFF45 :                   /* LYC */
        memory[address] = data;
        if (gpustate.enable){
            gpuCheckLYC();
            gpuCheckStatSignal();
        }
        break;            
    case 0xFF46 :                   /* DMA */
        dmastate.prepare = TRUE;
        dmastate.address = data << 8;
        break;
    case 0xFF47 :                   /* BGP */
        memory[address] = data;
        break;
    case 0xFF48 :                   /* OBP0 */
        memory[address] = data;
        break;
    case 0xFF49 :                   /* OBP1 */
        memory[address] = data;
        break;
    case 0xFF4A :                   /* WY */
        memory[address] = data;
        break;
    case 0xFF4B :                   /* WX */
        memory[address] = data;
        break;
    case 0xFF50 :
        /* Writing the value of 1 to the address 0xFF50 unmaps the boot ROM, 
         * and the first 256 bytes of the address space, where it effectively 
         * was mapped, now gets mapped to the beginning of the cartridge’s ROM. */
        if (data == 0x01)
            memoryCopy(0x0000, cart_ROM, 0xFF);
        break;
    case 0xFF76 :                   /* ???? */
        memory[address] = 0x00;     /* Not Readable */
        break;
    case 0xFF77 :                   /* ???? */
        memory[address] = 0x00;     /* Not Readable */
        break;
    default :
        memory[address] = 0xFF;
        break;  
    }
}

/**
 * writeMemory16 - write 2 bytes in memory.
 *
 * @data    a byte of information to be written
 * @address	the position to write to
 */
void writeMemory16(unsigned short address, unsigned short data)
{
    writeMemory8(address, (data & 0xFF));
    writeMemory8(address + 1, (data >> 8));
}



/******************************************************************************
 * Helper function to read / set / unset bits                                 *
 *****************************************************************************/
 

void setFlag(unsigned char flag)
{
    switch (flag) {
    case 7 :
        registers.F = registers.F | 0x80;
        break;
    case 6 :
        registers.F = registers.F | 0x40;
        break;
    case 5 :
        registers.F = registers.F | 0x20;
        break;
    case 4 :
        registers.F = registers.F | 0x10;
        break;
    default:
        break;
    }
}

void resetFlag (unsigned char flag)
{
    switch (flag) {
    case 7 :
        registers.F = registers.F & 0x7F;
        break;
    case 6 :
        registers.F = registers.F & 0xBF;
        break;
    case 5 :
        registers.F = registers.F & 0xDF;
        break;
    case 4 :
        registers.F = registers.F & 0xEF;
        break;
    default:
        break;
    }
}

int testFlag (unsigned char flag)
{
    switch (flag) {
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
    default:
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
        default:
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
        default:
            break;
    }
    return FALSE;
}

void stackPush16 (unsigned short value){

    if (!gpu_reading)
        hardwareTick();
        
    registers.SP--;                            // Decrease stack pointer
    writeMemory8( registers.SP, (value & 0xFF00) >> 8); // Push high part in the stack
    registers.SP--;                            // Decrease stack pointer again
    writeMemory8( registers.SP, value & 0x00FF );   // Push low part in the stack    
}

unsigned short stackPop16 (void){
    unsigned short value = 0;
    
    //reading is done this way to support correct timing
    unsigned short temp1 = readMemory8(registers.SP);
    unsigned short temp2 = readMemory8(registers.SP + 1) << 8;
    value =  temp1 | temp2;
    registers.SP += 2;
    return value;
}

/*
 * Expected timing (fresh DMA):
 *  M = 0: write to $FF46 happens
 *  M = 1: nothing (OAM still accessible)
 *  M = 2: new DMA starts, OAM reads will return $FF

 * Expected timing (restarted DMA):
 *  M = 0: write to $FF46 happens. Previous DMA is running (OAM *not* accessible)
 *  M = 1: previous DMA is running (OAM *not* accessible)
 *  M = 2: new DMA starts, OAM reads will return $FF
 */
void updateDMA (){

    if (dmastate.start){
        //directMemoryAccess(dmastate.address);
        dmastate.running = TRUE;
        dmastate.start = FALSE;
        dmastate.timer = 0;
        //dmastate.timer = 640;
    }
    
    if (dmastate.prepare){
        dmastate.start = TRUE;
        dmastate.prepare = FALSE;
    }

    if (dmastate.running){
        
        gpu_reading = 1;
        memory[0xFE00 + dmastate.timer] = readMemory8(dmastate.address + dmastate.timer);
        gpu_reading = 0;
        
        dmastate.timer += 1;

        if (dmastate.timer > 160){
            dmastate.running = FALSE;
            dmastate.timer = 0;
        }
    }

}

/*
 * The below function will copy the SRAM of MBC2 to a file upon an exit.
 * In that way we will emulate the SRAM of the cartridge.
 */
void updateMBC2SRAM (){
        FILE *fp;
    if (MBC_type == MBC2){
        if((fp=fopen(cart_game, "rb"))==NULL) {
            printf(PRINT_RED "[DEBUG] Cannot open file." PRINT_RESET"\n");
        }
        else{
             if (sram_active){
             //printf(PRINT_MAGENTA "[DEBUG] SRAM value =  %d" PRINT_RESET"\n",memory_SRAM[0]);
             fp=fopen(cart_game, "wb");
             //printf(PRINT_MAGENTA "[DEBUG] SRAM value =  %d" PRINT_RESET"\n",memory_SRAM[0]);
             fwrite(memory_SRAM, 1, 512, fp);
             fclose(fp);
             }
       }
    }
}
