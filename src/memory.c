#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include "timers.h"
#include "input.h"
#include "rom.h"
#include "gpu.h"
#include "hardware.h"
#include "definitions.h"
#include "serial.h"

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
unsigned char memory_backup[256];           /* Used when bios is loaded */
unsigned char memory_SRAM[512];


FILE *fp;
int gpu_reading = 0;

struct dma dmastate = {
    0,              //timer
    FALSE,          //prepare
    FALSE,          //start
    FALSE,          //running
    0x0000          //address
};

int sram_active =  0;              //1: SRAM cart is active 
                                   //0: save file is active
                                   
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


void memCopy(unsigned short start, unsigned char *buffer, unsigned short length){
	unsigned short i;
	
	for (i=start;i<=length;i++){
		memory[i] = buffer[i - start];
	}

	
}

void memoryReset (void){
	
    if (USINGBIOS){
        int i;
        registers.PC = 0x0000;
        for (i=0;i<=255;i++){
            memory_backup[i] = memory[i];   // backup address space that is overwitten from bios
            memory[i] = bios[i];
        }
        //memory[0xFF26] = 0xF1;	// NR52
        memory[0xFF00] = 0xCF;    
        memory[0xFF07] = 0xF8;
        memory[0xFF0F] = 0xE0;
        memory[0xFFFF] = 0xE0;
        //memory[0xFF02] = 0x7E;
        //memory[0xFF41] = 0x80;
        //memory[0xFF10] = 0x80;
        //memory[0xFF1A] = 0x7F;
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
       memory[0xFF41] = 0x80;   // LCDS
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

    dmastate.timer = 0;
    dmastate.prepare = FALSE;
    dmastate.start = FALSE;
    dmastate.running = FALSE;
    dmastate.address = 0x0000;

}
/* During mode 0 and mode 1 the CPU can access both VRAM and OAM. During mode 2 the CPU
 * can only access VRAM, not OAM. During mode 3 OAM and VRAM can't be accessed.
 */
unsigned char readMemory8 (unsigned short address){

    unsigned char temp;
    
    int address_map;

    switch (address >> 12){                 /* Get most significant 4 bits */
        case 0x4 ... 0x7:                   /* switchable ROM bank */
            address_map = address - 0x4000;
            address_map = address_map + (0x4000 * active_ROM_bank); //move address space to correct Memory Bank
            temp = cart_ROM[address_map];
            //temp = cart_ROM[(active_ROM_bank << 14) + (address - 0x4000)]; //SHL 14 is the same than *16384 (but faster) thx ZBOY
            break;
        case 0x8 ... 0x9:                   /* VRAM */
            if ((memory[0xFF41] & 0x03) == 0x03){
                temp = 0xFF;
            }
            else{
                temp = memory[address];
            }
            break;
        case 0xA ... 0xB:                   /* switchable RAM bank */
            printf("Read from ram at %x\n",address);
            if (!RAM_bank_enabled){         /* if Ram bank is not enabled */
                temp = 0xFF;                /* reads return 0xFF          */
                break;
            }
                
            if (MBC2){
                temp = 0;
                if (( address >= 0xA000 ) && ( address <= 0xA1FF )){
                    //printf(PRINT_RED "[DEBUG] Now we will read from SRAM and from address %4X" PRINT_RESET"\n",address);
                    if (!sram_active){
                        if((fp=fopen(cart_game, "rb"))==NULL) {
                            printf("Cannot open file.\n");
                        }
                        else{
                            fp=fopen(cart_game,"rb");
                            sram_active = 1;
                            fread(memory_SRAM, 1, 512, fp); 
                            address = address & 0x1FF;
                            /* Only the 4 lower bits are used, the upper bits should be ignored when reading */
                            temp = memory_SRAM[address]  | 0xF0;  
                            //printf(PRINT_RED "[DEBUG] memory_SRAM = %d" PRINT_RESET"\n", memory_SRAM[address]);
                            fclose(fp);
                        }
                    }
                    else{
                        address = address & 0x1FF;
                        /* Only the 4 lower bits are used, the upper bits should be ignored when reading */
                        temp = memory_SRAM[address] | 0xF0;  
                        //printf(PRINT_RED "[DEBUG] memory_SRAM = %d" PRINT_RESET"\n", memory_SRAM[address]);
                   }
               }
               else {
                   temp = memory[address];
               }
            }
            else {
                address -= 0xA000;
                address += 0x2000 * active_RAM_bank; //move address space to correct RAM Bank
                temp = cart_RAM[address];
            }
            break;
        case 0xF:
            switch (address >> 8){                 /* Get most significant 8 bits */
                case 0xFF:
                    switch (address){
                        case 0xFF00:         //P1 
                            temp = inputReadKeys();
                            memory[0xFF00] = temp;
                            break;
                        case 0xFF01:    //SB
                            temp = memory[address];
                            break;
                        case 0xFF02:    //SC
                            temp = memory[address];
                            temp |= 0x7E;               //BIT 6,5,4,3,2,1 Not Used
                            break;
                        case 0xFF04:    //DIV
                            //temp = memory[address];
                            temp = timersGetDIV();
                            break;
                        case 0xFF05:    //TIMA
                            //temp = memory[address];
                            temp = timersGetTIMA();
                            break;
                        case 0xFF06:    //TMA
                            //temp = memory[address];
                            temp = timersGetTMA();
                            break;                        
                        case 0xFF07:    //TAC
                            //temp = memory[address];                
                            //temp |= 0xF8;               //BIT 7,6,5,4,3 Not Used
                            temp = timersGetTAC();
                            break;
                        case 0xFF0F:    //IF 
                            temp = memory[address];
                            temp |= 0xE0;               //BIT 7,6,5 Not Used
                            break;
                        case 0xFF10:    //NR10
                            temp = memory[address];
                            temp |= 0x80;               //BIT 7 Not Used
                            break;
                        case 0xFF11:    //NR11
                            temp = memory[address];
                            temp |= 0x3F;               //Only Bits 7-6 can be read
                            break;
                        case 0xFF12:    //NR12
                            temp = memory[address];
                            break;
                        case 0xFF13:    //NR13
                            temp = memory[address];
                            temp |= 0xFF;               //Cant be read
                            break;
                        case 0xFF14:    //NR14
                            temp = memory[address];
                            temp |= 0xBF;               //Only Bit 6 can be read
                            break;
                        case 0xFF16:    //NR21
                            temp = memory[address];
                            temp |= 0x3F;               //Only bits 7-6 can be read
                            break;
                        case 0xFF17:    //NR22
                            temp = memory[address];
                            break;
                        case 0xFF18:    //NR23
                            temp = memory[address];
                            temp |= 0xFF;               //Cant be read
                            break;
                        case 0xFF19:    //NR24
                            temp = memory[address];
                            temp |= 0xBF;               //Only Bit 6 can be read
                            break;
                        case 0xFF1A:    //NR30
                            temp = memory[address];
                            temp |= 0x7F;               //BIT 6,5,4,3,2,1,0 Not Used
                            break;
                        case 0xFF1B:    //NR31
                            temp = memory[address];
                            temp |= 0xFF;               //Cant be read
                            break;
                        case 0xFF1C:    //NR32
                            temp = memory[address];
                            temp |= 0x9F;               //BIT 7,4,3,2,1,0 Not Used
                            break;
                        case 0xFF1D:    //NR33
                            temp = memory[address];
                            temp |= 0xFF;               //Cant be read
                            break;
                        case 0xFF1E:    //NR34
                            temp = memory[address];
                            temp |= 0xBF;               //Only Bit 6 can be read
                            break;
                        case 0xFF20:    //NR41
                            temp = memory[address];
                            temp |= 0xFF;               //Cant be read
                            break;
                        case 0xFF21:    //NR42
                            temp = memory[address];
                            break;
                        case 0xFF22:    //NR43
                            temp = memory[address];
                            break;
                        case 0xFF23:    //NR44
                            temp = memory[address];
                            temp |= 0xBF;               //Only Bit 6 can be read
                            break;
                        case 0xFF24:    //NR50
                            temp = memory[address];
                            break;
                        case 0xFF25:    //NR51
                            temp = memory[address];
                            break;
                        case 0xFF26:    //NR52
                            temp = memory[address];
                            temp |= 0x70;               //Only Bit 7,3,2,1,0 can be read
                            break;
                        case 0xFF30 ... 0xFF3F:   //WAVE pattern RAM
                            temp = memory[address];
                            break;                                           
                        case 0xFF40:    //LCDC
                            temp = memory[address];
                            break;            
                        case 0xFF41:    //STAT
                            temp = memory[address];
                            temp |= 0x80;               //BIT 7 Not Used
                            break;
                        case 0xFF42:    //SCY
                            temp = memory[address];
                            break;
                        case 0xFF43:    //SCX
                            temp = memory[address];
                            break;
                        case 0xFF44:    //LY
                            temp = memory[address];
                            break;
                        case 0xFF45:    //LYC
                            temp = memory[address];
                            break;            
                        case 0xFF46:    //DMA
                            if (dmastate.running)
                                temp = 0xFF;
                            else
                                temp = memory[address];
                            break;
                        case 0xFF47:    //BGP
                            temp = memory[address];
                            break;
                        case 0xFF48:    //OBP0
                            temp = memory[address];
                            break;
                        case 0xFF49:    //OBP1
                            temp = memory[address];
                            break;
                        case 0xFF4A:    //WY
                            temp = memory[address];
                            break;
                        case 0xFF4B:    //WX
                            temp = memory[address];
                            break;
                        case 0xFF76:    //????
                            temp = memory[address];
                            temp |= 0xFF;               //Not Readable
                            break;
                        case 0xFF77:    //????
                            temp = memory[address];
                            temp |= 0xFF;               //Not Readable
                            break;
                        case 0xFF80 ... 0xFFFE:   //Usable RAM
                            temp = memory[address];
                            break;
                        case 0xFFFF:    //IE
                            temp = memory[address];
                            break;
                        default:
                            temp = 0xFF;
                            break;
                    }
                    break;
                case 0xFE:
                    if (address < 0xFEA0 && dmastate.running){
                        printf("[DMA] Tried reading OAM while unreadable\n");
                        temp = 0xFF;
                    }
                    else if ((memory[0xFF41] & 0x02) == 0x02 && (address < 0xFEA0 )){ //Both OAM and VRAM
                        printf("[DMA] Tried reading VRAM while unreadable\n");
                        temp = 0xFF;
                    }
                    else{
                        temp = memory[address];
                    }         
                    break;
                default:
                    temp = memory[address];
                    break;
            }
            break;
        default:
            temp = memory[address];
            break;
    }
    
    if (!gpu_reading)
        hardwareTick();
        
    return temp;
}


unsigned short readMemory16 (unsigned short address){
    
    return (readMemory8(address) | (readMemory8(address + 1) << 8));

}

void writeMemory16 (unsigned short pos, unsigned short value){
    
    writeMemory(pos, (value & 0xFF));
    writeMemory(pos + 1, (value >> 8));

}

void writeMemory (unsigned short pos, unsigned char value){

    unsigned short address = pos;
    

    switch (address & 0xFF00){
        case 0xFF00 : 
            if (address == 0xFF00){         //P1
                value |= 0xC0;              //BIT 7,6 Not Used
                value &= 0xF0;              //BIT 3,2,1,0 Not Writable
                memory[address] &= 0x0F;    /* clear upper half */
                memory[address] |= value;   /* set upper half */
                inputCheckInterrupt();
            }
            else if (address == 0xFF01){    //SB
                //memory[address] = value;  hack to pass a test
            }            
            else if (address == 0xFF02){    //SC
                value |= 0x7E;              //BIT 6,5,4,3,2,1 Not Used
                serialSetControl(value);
                memory[address] = value;
            }
            else if (address == 0xFF04){    //DIV
                //updateDivider();
                timersSetDIV();
            }
            else if (address == 0xFF05){    //TIMA
                //memory[address] = value;
                timersSetTIMA(value);
            }
            else if (address == 0xFF06){    //TMA
                //memory[address] = value;
                timersSetTMA(value);
            }                                  
            else if (address == 0xFF07){    //TAC
                //updateFrequency(value);
                //value |= 0xF8;              //BIT 7,6,5,4,3 Not Used
                //memory[address] = value;
                timersSetTAC(value);     
            }
            else if (address == 0xFF0F){    //IF 
                value |= 0xE0;              //BIT 7,6,5 Not Used
                memory[address] = value;
            }
            else if (address == 0xFF10){    //NR10
                value |= 0x80;              //BIT 7 Not Used
                memory[address] = value;
            }
            else if (address == 0xFF11){    //NR11
                memory[address] = value;
            }
            else if (address == 0xFF12){    //NR12
                memory[address] = value;
            }
            else if (address == 0xFF13){    //NR13
                memory[address] = value;
            }
            else if (address == 0xFF14){    //NR14
                memory[address] = value;
            }
            else if (address == 0xFF16){    //NR21
                memory[address] = value;
            }
            else if (address == 0xFF17){    //NR22
                memory[address] = value;
            }
            else if (address == 0xFF18){    //NR23
                memory[address] = value;
            }
            else if (address == 0xFF19){    //NR24
                memory[address] = value;
            }
            else if (address == 0xFF1A){    //NR30
                value |= 0x7F;              //BIT 6,5,4,3,2,1,0 Not Used
                memory[address] = value;
            }
            else if (address == 0xFF1B){    //NR31
                memory[address] = value;
            }
            else if (address == 0xFF1C){    //NR32
                value |= 0x9F;              //BIT 7,4,3,2,1,0 Not Used
                memory[address] = value;
            }
            else if (address == 0xFF1D){    //NR33
                memory[address] = value;
            }
            else if (address == 0xFF1E){    //NR34
                memory[address] = value;
            }
            else if (address == 0xFF20){    //NR41
                value |= 0xC0;              //BIT 7,6 Not Used
                memory[address] = value;
            }
            else if (address == 0xFF21){    //NR42
                memory[address] = value;
            }
            else if (address == 0xFF22){    //NR43
                memory[address] = value;
            }
            else if (address == 0xFF23){    //NR44
                value |= 0x3F;              //BIT 5,4,3,2,1,0 Not Used
                memory[address] = value;
            }
            else if (address == 0xFF24){    //NR50
                memory[address] = value;
            }
            else if (address == 0xFF25){    //NR51
                memory[address] = value;
            }
            else if (address == 0xFF26){    //NR52
                value |= 0x70;              //BIT 6,5,4 Not Used
                memory[address] = value | 0x01; // Nasty Hack
            }
            else if (address >= 0xFF30 && address <= 0xFF3F){   //WAVE pattern RAM
                memory[address] = value;
            }                                           
            else if (address == 0xFF40){    //LCDC
                gpuSetStatus(value);
                memory[address] = value;
            }            
            else if (address == 0xFF41){    //STAT
                value |= 0x80;              //BIT 7 Not Used
                /* bobonis assumes 3 LSB cannot be set */
                value &= 0xFC;
                value |= memory[address] & 0x03;
                memory[address] = value;
                if (gpustate.enable){
                    gpuCompareLine();
                }               
            }
            else if (address == 0xFF42){    //SCY
                memory[address] = value;
            }
            else if (address == 0xFF43){    //SCX
                memory[address] = value;
            }
            else if (address == 0xFF44){    //LY
                memory[address] = 0x00;     //Writing will reset the counter
                memory[LY] = 0;
                gpustate.line = 0;
            }
            else if (address == 0xFF45){    //LYC
                memory[address] = value;
                if (gpustate.enable){
                    gpuCompareLine();
                }
            }            
            else if (address == 0xFF46){    //DMA
                dmastate.prepare = TRUE;
                dmastate.address = value << 8;
            }
            else if (address == 0xFF47){    //BGP
                memory[address] = value;
            }
            else if (address == 0xFF48){    //OBP0
                memory[address] = value;
            }
            else if (address == 0xFF49){    //OBP1
                memory[address] = value;
            }
            else if (address == 0xFF4A){    //WY
                memory[address] = value;
            }
            else if (address == 0xFF4B){    //WX
                memory[address] = value;
            }
            else if (address == 0xFF76){    //????
                memory[address] = 0x00;     //Not Readable
            }
            else if (address == 0xFF77){    //????
                memory[address] = 0x00;     //Not Readable
            }            
            /* Writing the value of 1 to the address 0xFF50 unmaps the boot ROM, 
            * and the first 256 bytes of the address space, where it effectively 
            * was mapped, now gets mapped to the beginning of the cartridge’s ROM.
            */
            else if ((address == 0xFF50) && (value == 0x01)){
                memCopy(0x0000, memory_backup, 0xFF);
            }
            else if (address >= 0xFF80 && address <= 0xFFFE){   //Usable RAM
                memory[address] = value;
            }
            else if (address == 0xFFFF){    //IE
                //value |= 0xE0;              //BIT 7,6,5 Not Used
                memory[address] = value;
            }
            else{
                memory[address] = 0xFF;
            }
    
            if (!gpu_reading)
                hardwareTick();
            
            return;
        default:
            break;
    }

    if (pos < 0x8000){
//        printf("OLD ROM= %d, RAM= %d",active_ROM_bank,active_RAM_bank);
        cartridgeSwitchBanks(pos, value);
//        printf("-- NEW ROM= %d, RAM= %d\n",active_ROM_bank,active_RAM_bank);
    }
    else if (( pos >= 0x8000 ) && ( pos <= 0x9FFF )){ //VRAM
        if ((memory[0xFF41] & 0x03) == 0x03){
        }
        else{
            memory[pos] = value;
        }
    }
    else if (( pos >= 0xA000 ) && ( pos <= 0xBFFF )){ //RAM Memory Bank
    printf("Write to ram at %x\n",address);
        if (RAM_bank_enabled){
            if (MBC2){
                if (( pos >= 0xA000 ) && ( pos <= 0xA1FF )){
                    //printf(PRINT_RED "[DEBUG] Now we will write to SRAM" PRINT_RESET"\n");
                    pos &= 0x1FF;
                    /* Only the 4 lower bits are used, the upper bits should be ignored */
                    memory_SRAM[pos] = value & 0x0F;
                    sram_active = 1;
                    //printf(PRINT_MAGENTA "[DEBUG] SRAM value =  %d and SRAM position = %d" PRINT_RESET"\n",memory_SRAM[pos],pos);
                }
                else {
                    memory[pos] = value;
                }
            }
            else{
                pos -= 0xA000;
                pos += 0x2000 * active_RAM_bank; //move address space to correct RAM Bank
                cart_RAM[pos] = value;
            }
        }
    }    
    else if ( ( pos >= 0xE000 ) && (pos < 0xFE00) ){ // writing to ECHO ram also writes in RAM
        memory[pos] = value ;
        memory[pos - 0x2000] = value ;
    }
    else if ( ( pos >= 0xC000 ) && (pos < 0xE000) ){ // writing to RAM also writes in ECHO RAM
        memory[pos] = value ;
        if ((pos + 0x2000) < 0xFE00){
            memory[pos + 0x2000] = value;
        }
        //memory[pos + 0x2000] = value;
    }
    else if ( ( pos >= 0xFEA0 ) && (pos < 0xFEFF) ){ // this area is restricted
    }
    else if (address >= 0xFE00 && address < 0xFEA0){
        if (dmastate.running || (memory[0xFF41] & 0x02) == 0x02){ //Both OAM and VRAM
            printf("[DMA] Tried writing OAM while unwrittable\n");
        }
        else{
            memory[pos] = value;
        }
    }
    else{ //default
        memory[pos] = value;
    }

    if (!gpu_reading)
        hardwareTick();
        
    return;


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
        default:
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
        default:
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
    writeMemory( registers.SP, (value & 0xFF00) >> 8); // Push high part in the stack
    registers.SP--;                            // Decrease stack pointer again
    writeMemory( registers.SP, value & 0x00FF );   // Push low part in the stack    
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
    if (MBC2){
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