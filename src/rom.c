#include <stdio.h>
#include <stdlib.h>
#include "rom.h"
#include "memory.h"

const unsigned char bioslogo[48] = {
0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 
0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 
0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 
0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 
0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E};

int loadRom(const char *filename){
	
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
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("[INFO] Filesize: %d\n", (int)lSize);
	
	// Allocate memory to contain the whole file
	unsigned char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL){
		printf("[ERROR] Memory error\n"); 
		return 1;
	}

	// Copy the file into the buffer
	size_t result = fread (buffer, 1, lSize, pFile);
	if (result != lSize){
		printf("[ERROR] Reading error\n"); 
		return 1;
	}

//	for (i=0;i<100;i++){
//		printf("0x%04x - 0x%02x\n",i,buffer[i]);
//	}

	// Read cartrige type
	romtype = buffer[0x147];
	switch (romtype){
		case 0x00 :
			printf("[INFO] Catrige type is: 0x%02x - ROM ONLY\n",romtype);
			memCopy(memory,0x0000,buffer,0x7FFF);
			break;
		case 0x01 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC1\n",romtype);
			memCopy(memory,0x0000,buffer,0x3FFF);
			break;			
		case 0x02 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC1+RAM\n",romtype);
			break;		
		case 0x03 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC1+RAM+BATT\n",romtype);
			break;
		case 0x05 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC2\n",romtype);
			break;
		case 0x06 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC2+BATTERY\n",romtype);
			break;
		case 0x08 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+RAM\n",romtype);
			break;			
		case 0x09 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+RAM+BATTERY\n",romtype);
			break;		
		case 0x0B :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MMM01\n",romtype);
			break;
		case 0x0C :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MMM01+SRAM\n",romtype);
			break;
		case 0x0D :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MMM01+SRAM+BATT\n",romtype);
			break;
		case 0x0F :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC3+TIMER+BATT\n",romtype);
			break;			
		case 0x10 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC3+TIMER+RAM+BATT\n",romtype);
			break;
		case 0x11 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC3\n",romtype);
			break;			
		case 0x12 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC3+RAM\n",romtype);
			break;
		case 0x13 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC3+RAM+BATT\n",romtype);
			break;
		case 0x19 :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC5\n",romtype);
			break;
		case 0x1A :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC5+RAM\n",romtype);
			break;
		case 0x1B :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC5+RAM+BATT\n",romtype);
			break;
		case 0x1C :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC5+RUMBLE\n",romtype);
			break;
		case 0x1D :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC5+RUMBLE+SRAM\n",romtype);
			break;
		case 0x1E :
			printf("[INFO] Catrige type is: 0x%02x - ROM+MBC5+RUMBLE+SRAM+BATT\n",romtype);
			break;
		case 0x1F :
			printf("[INFO] Catrige type is: 0x%02x - Pocket Camera\n",romtype);
			break;
		case 0xFD :
			printf("[INFO] Catrige type is: 0x%02x - Bandai TAMA5\n",romtype);
			break;
		case 0xFE :
			printf("[INFO] Catrige type is: 0x%02x - Hudson HuC-3\n",romtype);
			break;
		case 0xFF :
			printf("[INFO] Catrige type is: 0x%02x - Hudson HuC-1\n",romtype);
			break;
		default:
			printf("[ERROR] Unknown cartrige type: 0x%02x\n",romtype);
			return 1;
	}

	// Read ROM size
	romsize = buffer[0x148];
	switch (romsize){
		case 0x00 :
			printf("[INFO] ROM size is: 0x%02x - 256Kbit = 32KByte = 2 banks\n",romsize);
			break;
		case 0x01 :
			printf("[INFO] ROM size is: 0x%02x - 512Kbit = 64KByte = 4 banks\n",romsize);
			break;		
		case 0x02 :
			printf("[INFO] ROM size is: 0x%02x - 1Mbit = 128KByte = 8 banks\n",romsize);
			break;
		case 0x03 :
			printf("[INFO] ROM size is: 0x%02x - 2Mbit = 256KByte = 16 banks\n",romsize);
			break;			
		case 0x04 :
			printf("[INFO] ROM size is: 0x%02x - 4Mbit = 512KByte = 32 banks\n",romsize);
			break;			
		case 0x05 :
			printf("[INFO] ROM size is: 0x%02x - 8Mbit = 1MByte = 64 banks\n",romsize);
			break;				
		case 0x06 :
			printf("[INFO] ROM size is: 0x%02x - 16Mbit = 2MByte = 128 banks\n",romsize);
			break;				
		case 0x52 :
			printf("[INFO] ROM size is: 0x%02x - 9Mbit = 1.1MByte = 72 banks\n",romsize);
			break;			
		case 0x53 :
			printf("[INFO] ROM size is: 0x%02x - 10Mbit = 1.2MByte = 80 banks\n",romsize);
			break;			
		case 0x54 :
			printf("[INFO] ROM size is: 0x%02x - 12Mbit = 1.5MByte = 96 banks\n",romsize);
			break;
		default:
			printf("[ERROR] Unknown ROM size: 0x%02x\n",romsize);
			return 1;
	}

	// Read RAM size
	ramsize = buffer[0x149];
	switch (ramsize){
		case 0x00 :
			printf("[INFO] RAM size is: 0x%02x - None\n",ramsize);
			break;
		case 0x01 :
			printf("[INFO] RAM size is: 0x%02x - 16Kbit = 2KByte = 1 bank\n",ramsize);
			break;		
		case 0x02 :
			printf("[INFO] RAM size is: 0x%02x - 64Kbit = 8KByte = 1 bank\n",ramsize);
			break;
		case 0x03 :
			printf("[INFO] RAM size is: 0x%02x - 256Kbit = 32KByte = 4 banks\n",ramsize);
			break;			
		case 0x04 :
			printf("[INFO] RAM size is: 0x%02x - 1Mbit = 128KByte = 16 banks\n",ramsize);
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
		

	// Copy buffer to Chip8 memory

/*	if((4096-512) > lSize)
	{
		for(i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");
*/
	
	// Close file, free buffer
	fclose(pFile);
	free(buffer);
	printf("[INFO] Rom loaded correctly\n");
	return 0;
}
