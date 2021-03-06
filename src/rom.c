#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "rom.h"
#include "gpu.h"
#include "memory.h"
#include "definitions.h"

const unsigned char bioslogo[48] = {
0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 
0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 
0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 
0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 
0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E};

unsigned char *cart_RAM;
unsigned char *cart_ROM;
char cart_game[17] = "sram/cartridgerom";
unsigned char active_RAM_bank = 0;
unsigned char total_RAM_banks = 0;
int RAM_bank_enabled = FALSE;		/* We assume that the RAM Bank is disabled at startup */
int RTC_register_enabled = FALSE;
unsigned char RTC_register_mapped = 0x00;
unsigned char active_ROM_bank = 1;
unsigned char total_ROM_banks = 0;

int MBC_type = NOMBC;
int MBC_mode = 0; 					/* 0 - switch ROM bank, 1 - switch RAM bank (default 0) */


int loadRom(const char *filename){
	    
    cart_RAM = (unsigned char*)calloc(0x8000, 1);
    FILE *fp;
	int i;
	unsigned char romtype,romsize,ramsize;
	unsigned short checksum = 25;

	// Check file exists
	printf("[INFO] Loading: %s\n", filename);	
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL){
		printf("[ERROR] File error\n");
		return 1;
	}

	// Check file size
	fseek(pFile , 0 , SEEK_END);
	unsigned long lSize = (unsigned long)ftell(pFile);
	rewind(pFile);
	printf("[INFO] Filesize: %d\n", (int)lSize);
	
	// Allocate memory to contain the whole file
    cart_ROM = (unsigned char*)malloc(sizeof(unsigned char) * lSize);
	if (cart_ROM == NULL){
		printf("[ERROR] Memory error\n"); 
		return 1;
	}

	// Copy the file into the buffer
	size_t result = fread (cart_ROM, 1, lSize, pFile);
	if (result != lSize){
		printf("[ERROR] Reading error\n"); 
		return 1;
	}


    printf("[INFO] Cartridge game is: ");
    for (i=0x134;i<=0x142;i++){
         printf(PRINT_YELLOW "%c" PRINT_RESET,cart_ROM[i]);
         if((i-303)<17){
         cart_game[i-303] = (char)cart_ROM[i];
         }
    }
    printf("\n");
    
	// Read cartrige type
	romtype = cart_ROM[0x147];
	switch (romtype){
		case 0x00 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM ONLY" PRINT_RESET"\n",romtype);
			memoryCopy(0x0000,cart_ROM,0x3FFF);
			break;
		case 0x01 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC1" PRINT_RESET"\n",romtype);
			memoryCopy(0x0000,cart_ROM,0x3FFF);
            MBC_type = MBC1;
			break;			
		case 0x02 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC1+RAM" PRINT_RESET"\n",romtype);
			memoryCopy(0x0000,cart_ROM,0x3FFF);
            MBC_type = MBC1;	
		case 0x03 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC1+RAM+BATT" PRINT_RESET"\n",romtype);
            memoryCopy(0x0000,cart_ROM,0x3FFF);
            MBC_type = MBC1;
			break;
		case 0x05 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC2" PRINT_RESET"\n",romtype);
            memoryCopy(0x0000,cart_ROM,0x3FFF);
            MBC_type = MBC2;
			break;
		case 0x06 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC2+BATTERY" PRINT_RESET"\n",romtype);
            memoryCopy(0x0000,cart_ROM,0x3FFF);
            MBC_type = MBC2;
            if (!(file_exist (cart_game))){
                if((fp = fopen(cart_game,"wb"))==NULL) {
                    printf(PRINT_RED "[DEBUG]Cannot create file." PRINT_RESET "\n");
                }
                else{
                     fp = fopen(cart_game,"wb");
                     fclose(fp);
                }
            }
			break;
		case 0x08 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+RAM" PRINT_RESET"\n",romtype);
			break;			
		case 0x09 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+RAM+BATTERY" PRINT_RESET"\n",romtype);
			break;		
		case 0x0B :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MMM01" PRINT_RESET"\n",romtype);
			break;
		case 0x0C :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MMM01+SRAM" PRINT_RESET"\n",romtype);
			break;
		case 0x0D :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MMM01+SRAM+BATT" PRINT_RESET"\n",romtype);
			break;
		case 0x0F :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC3+TIMER+BATT" PRINT_RESET"\n",romtype);
			break;			
		case 0x10 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC3+TIMER+RAM+BATT" PRINT_RESET"\n",romtype);
            memoryCopy(0x0000,cart_ROM,0x3FFF);
            MBC_type = MBC3;
			break;
		case 0x11 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC3" PRINT_RESET"\n",romtype);
			break;			
		case 0x12 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC3+RAM" PRINT_RESET"\n",romtype);
			break;
		case 0x13 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC3+RAM+BATT" PRINT_RESET"\n",romtype);
            memoryCopy(0x0000,cart_ROM,0x3FFF);
            MBC_type = MBC3;
			break;
		case 0x19 :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC5" PRINT_RESET"\n",romtype);
			break;
		case 0x1A :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC5+RAM" PRINT_RESET"\n",romtype);
			break;
		case 0x1B :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC5+RAM+BATT" PRINT_RESET"\n",romtype);
			break;
		case 0x1C :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC5+RUMBLE" PRINT_RESET"\n",romtype);
			break;
		case 0x1D :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC5+RUMBLE+SRAM" PRINT_RESET"\n",romtype);
			break;
		case 0x1E :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - ROM+MBC5+RUMBLE+SRAM+BATT" PRINT_RESET"\n",romtype);
			break;
		case 0x1F :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - Pocket Camera" PRINT_RESET"\n",romtype);
			break;
		case 0xFD :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - Bandai TAMA5" PRINT_RESET"\n",romtype);
			break;
		case 0xFE :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - Hudson HuC-3" PRINT_RESET"\n",romtype);
			break;
		case 0xFF :
			printf(PRINT_CYAN "[INFO] Cartridge type is: 0x%02x - Hudson HuC-1" PRINT_RESET"\n",romtype);
			break;
		default:
			printf(PRINT_RED "[ERROR] Unknown cartrige type: 0x%02x" PRINT_RESET"\n",romtype);
			return 1;
	}

	// Read ROM size
	romsize = cart_ROM[0x148];
	switch (romsize){
		case 0x00 :
			printf("[INFO] ROM size is: 0x%02x - 256Kbit = 32KByte = 2 banks\n",romsize);
            total_ROM_banks = 2;
			break;
		case 0x01 :
			printf("[INFO] ROM size is: 0x%02x - 512Kbit = 64KByte = 4 banks\n",romsize);
            total_ROM_banks = 4;
			break;		
		case 0x02 :
			printf("[INFO] ROM size is: 0x%02x - 1Mbit = 128KByte = 8 banks\n",romsize);
            total_ROM_banks = 8;
			break;
		case 0x03 :
			printf("[INFO] ROM size is: 0x%02x - 2Mbit = 256KByte = 16 banks\n",romsize);
            total_ROM_banks = 16;
			break;			
		case 0x04 :
			printf("[INFO] ROM size is: 0x%02x - 4Mbit = 512KByte = 32 banks\n",romsize);
            total_ROM_banks = 32;
			break;			
		case 0x05 :
			printf("[INFO] ROM size is: 0x%02x - 8Mbit = 1MByte = 64 banks\n",romsize);
            total_ROM_banks = 64;
			break;				
		case 0x06 :
			printf("[INFO] ROM size is: 0x%02x - 16Mbit = 2MByte = 128 banks\n",romsize);
            total_ROM_banks = 128;
			break;				
		case 0x52 :
			printf("[INFO] ROM size is: 0x%02x - 9Mbit = 1.1MByte = 72 banks\n",romsize);
            total_ROM_banks = 72;
			break;			
		case 0x53 :
			printf("[INFO] ROM size is: 0x%02x - 10Mbit = 1.2MByte = 80 banks\n",romsize);
            total_ROM_banks = 80;
			break;			
		case 0x54 :
			printf("[INFO] ROM size is: 0x%02x - 12Mbit = 1.5MByte = 96 banks\n",romsize);
            total_ROM_banks = 96;
			break;
		default:
			printf("[ERROR] Unknown ROM size: 0x%02x\n",romsize);
			return 1;
	}

	// Read RAM size
	ramsize = cart_ROM[0x149];
	switch (ramsize){
		case 0x00 :
			printf("[INFO] RAM size is: 0x%02x - None\n",ramsize);
            total_RAM_banks = 0;
			break;
		case 0x01 :
			printf("[INFO] RAM size is: 0x%02x - 16Kbit = 2KByte = 1 bank\n",ramsize);
            total_RAM_banks = 1;
			break;		
		case 0x02 :
			printf("[INFO] RAM size is: 0x%02x - 64Kbit = 8KByte = 1 bank\n",ramsize);
            total_RAM_banks = 1;
			break;
		case 0x03 :
			printf("[INFO] RAM size is: 0x%02x - 256Kbit = 32KByte = 4 banks\n",ramsize);
            total_RAM_banks = 4;
			break;			
		case 0x04 : //is it for real?
			printf("[INFO] RAM size is: 0x%02x - 1Mbit = 128KByte = 16 banks\n",ramsize);
            total_RAM_banks = 16;
			break;			
		default:
			printf("[ERROR] Unknown RAM size: 0x%02x\n",ramsize);
			return 1;
	}

	// Check ROM Logo (Bios)
	for (i=0x104;i<=0x133;i++){
		if (memory[i] != bioslogo[i-0x104]){
			printf("[ERROR] Bios check failed at logo check\n");
			printf("[ERROR] BIOS = 0x%02x - ROM = 0x%02x\n",bioslogo[i-0x104],memory[i]);
			return 1;
		}
	}
	printf("[INFO] Bios logo check passed\n");
	// Check ROM Sum (Bios)
	for (i=0x134;i<=0x14D;i++){
		checksum = checksum + memory[i];
	}
	printf("[INFO] Bios Rom SUM is : 0x%04x\n",checksum);
	if ((checksum & 0x000F) != 0){
		printf("[ERROR] Bios Rom SUM failed\n");
		return 1;
	}
	printf("[INFO] Bios Rom SUM passed\n");
		

	// Close file, free buffer
	fclose(pFile);
	//free(buffer);
	printf("[INFO] Rom loaded correctly\n");

	return 0;
}


/*
 * Locations    Register                         Details
 *  
 * 0000-1FFF	Enable external RAM	             4 bits wide 
 *                                               value of 0x0A enables RAM,
 *                                               any other value disables
 * 2000-3FFF    ROM bank (low 5 bits)            Switch between banks 1-31 (value 0 is seen as 1)
 * 4000-5FFF    ROM bank (high 2 bits)           ROM mode: switch ROM bank "set" {1-31}-{97-127}
 *                                               RAM mode: switch RAM bank 0-3
 * 6000-7FFF	Mode                             0: ROM mode (no RAM banks, up to 2MB ROM)
 *                                               1: RAM mode (4 RAM banks, up to 512kB ROM)
 */
void cartridgeSwitchBanks(unsigned short address, unsigned char value){
    
    //printf("[DEBUG] Switch bank, old bank is %x address is %x, value is %x,\n",active_ROM_bank,address,value);

    /* Before you can read or write to a RAM bank you have to enable
     * it by writing a XXXX1010 into 0000-1FFF area*. To
     * disable RAM bank operations write any value but
     * XXXX1010 into 0000-1FFF area. Disabling a RAM bank
     * probably protects that bank from false writes
     * during power down of the GameBoy. (NOTE: Nintendo
     * suggests values $0A to enable and $00 to disable
     * RAM bank!!)
     */
    if (address <= 0x1FFF){
        if (MBC_type == MBC1){
            if (( value & 0x0F ) == 0x0A ){
                /* if (MBC_mode)   //RAM mode This seems incorrect */
                    RAM_bank_enabled = TRUE;    //Enable external RAM
            }
            else{
                /* if (MBC_mode)   //RAM mode This seems incorrect too */
                    RAM_bank_enabled = FALSE;   //Disable external RAM
            }
        }
		else if (MBC_type == MBC2){
			/* The least significant bit of the upper address byte must be '0' to enable/disable cart RAM. */
         	if (address & 0x0100) 
				return;
			/* Writing any value with 0Ah in the lower 4 bits enables RAM, and any other value disables it. */
            if (( value & 0x0F ) == 0x0A )
	            RAM_bank_enabled = TRUE;    /* Enable external RAM */
            else
                RAM_bank_enabled = FALSE;   /* Disable external RAM */
		}
        else if (MBC_type == MBC3){
            /* Writing to this address range any value with 0Ah in the lower 4 bits 
               enables RAM and RTC registers, and any other value disables them. 
               Usually, 00h is used to disable them and 0Ah is used to enable them.
            */
            if (( value & 0x0F ) == 0x0A ){
                    RAM_bank_enabled = TRUE;    //Enable external RAM
            }
            else{
                    RAM_bank_enabled = FALSE;   //Disable external RAM
                    RTC_register_enabled = FALSE;
            }
        }
    }
    else if (( address >= 0x2000 ) && ( address <= 0x3FFF )){
        if (MBC_type == MBC1){
            if (MBC_mode == 0){     //ROM mode 16/8
                value &= 0x1F; // keep 5 LSB
                active_ROM_bank &= 0x60; // Turn off 5 LSB 0XX00000
                active_ROM_bank |= value; // merge value
            }
            else{     //ROM mode 4/32
                value &= 0x1F; // keep 5 LSB
                active_ROM_bank = value;
            }
            
            while (active_ROM_bank > total_ROM_banks){
                active_ROM_bank = active_ROM_bank - total_ROM_banks;
            }
            
            if (active_ROM_bank == 0){ /* Bank 0 is not allowed */
                active_ROM_bank = 1;
            }

		}
        else if (MBC_type == MBC2){
			/* The least significant bit of the upper address byte must be '1' to select a ROM bank. */
         	if (address & 0x0100){ 
	        	active_ROM_bank = value & 0x0F;
				/* handle the case when more that available rom banks are set */
				active_ROM_bank &= total_ROM_banks - 1;
            	if (active_ROM_bank == 0) /* Bank 0 is not allowed */
            		active_ROM_bank = 1;
			}
		}
        else if (MBC_type == MBC3){
            /* The 7 lower bits of the value written here is the ROM bank that will be mapped to 4000h-3FFFh */
            active_ROM_bank = value & 0x7F;
            /* handle the case when more that available rom banks are set */
            while (active_ROM_bank > total_ROM_banks){
                active_ROM_bank = active_ROM_bank - total_ROM_banks;
            }
            if (active_ROM_bank == 0){ /* Bank 0 is not allowed */
                active_ROM_bank = 1;
            }
		}
    }
    else if (( address >= 0x4000 ) && ( address <= 0x5FFF )){
        if (MBC_type == MBC1){
            if (MBC_mode == 0){     //ROM mode  16/8
                value &= 0x03;  // keep 2 LSB 000000BB
                value <<= 5; // Move LSB to MSB 0BB00000
                active_ROM_bank &= 0x9F;    // Turn off 2 MSB X00XXXXX
                active_ROM_bank |= value; // merge value XBBXXXXX

                while (active_ROM_bank > total_ROM_banks){
                    active_ROM_bank = active_ROM_bank - total_ROM_banks;
                }

                if (active_ROM_bank == 0){ // Bank 0 is not allowed
                    active_ROM_bank = 1;
                }
            }
            else{                   //RAM mode
                active_RAM_bank = value & 0x03;

                while (active_RAM_bank > total_RAM_banks){
                    active_RAM_bank = active_RAM_bank - total_RAM_banks;
                }
            }
        }
        else if (MBC_type == MBC3){
            switch (value){
                /* writing a value in range for 00h-03h maps the corresponding external RAM Bank (if any) into memory at A000-BFFF */
                case 0x00 ... 0x07 :
                    active_RAM_bank = value;
                    while (active_RAM_bank > total_RAM_banks){
                        active_RAM_bank = active_RAM_bank - total_RAM_banks;
                    }
                    RTC_register_enabled = FALSE;
                    break;
                /* writing a value of 08h-0Ch, this will map the corresponding RTC register into memory at A000-BFFF */
                case 0x08 ... 0x0C :
                    RTC_register_enabled = TRUE;
                    RTC_register_mapped = value;
                    break;
                default :
                    break;
            }
        }
        
    }
    else if (( address >= 0x6000 ) && ( address <= 0x7FFF )){

        if (MBC_type == MBC1){
            if (( value & 0x01 ) == 0 ){
                MBC_mode = 0;   //ROM mode (no RAM banks, up to 2MB ROM)
                active_RAM_bank = 0;
                 /*only RAM Bank 00h can be used during Mode 0,
                   and only ROM Banks 00-1Fh can be used during Mode 1.
                  */
            }
            else{
                MBC_mode = 1;   //RAM mode (4 RAM banks, up to 512kB ROM)
            }
        }
        
    }
}

/*
 * The below function will check if a file exists.
 *
 */
 
 int file_exist (char *fp)
{
  struct stat buffer;   
  return (stat (fp, &buffer) == 0);
}