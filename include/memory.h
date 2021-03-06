#ifdef __cplusplus
extern "C" {
#endif

typedef int bool;

void memoryCopy(unsigned short start, unsigned char *buffer, unsigned short length);
void memoryReset (void);
void memoryDMAReset(void);
void setFlag (unsigned char flag);
void resetFlag (unsigned char flag);
int testFlag (unsigned char flag);

/* Read functions */
unsigned char readMemory8(unsigned short address);
unsigned char readMemoryROMBank(unsigned short address);
unsigned char readMemoryRAMBank(unsigned short address);
unsigned char readMemoryVRAM(unsigned short address);
unsigned char readMemoryIOPorts(unsigned short address);
unsigned char readMemoryOAM(unsigned short address);
unsigned char readMemoryECHORAM(unsigned short address);
unsigned char readMemoryWRAM(unsigned short address);
unsigned short readMemory16(unsigned short address);

/* Write functions */
void writeMemory8(unsigned short pos, unsigned char value);
void writeMemoryVRAM(unsigned short address, unsigned char data);
void writeMemoryRAMBank(unsigned short address, unsigned char data);
void writeMemoryWRAM(unsigned short address, unsigned char data);
void writeMemoryECHORAM(unsigned short address, unsigned char data);
void writeMemoryOAM(unsigned short address, unsigned char data);
void writeMemoryIOPorts(unsigned short address, unsigned char data);
void writeMemory16(unsigned short pos, unsigned short value);





void setBit(unsigned short pos, unsigned char bit, bool value);
bool testBit(unsigned short pos, unsigned char bit);
void stackPush16 (unsigned short value);
unsigned short stackPop16 (void);

void updateDMA (void);
void updateMBC2SRAM (void);

/* The GameBoy has eight 8-bit registers A,B,C,D,E,F,H,L 
 * and two 16-bit registers SP & PC.
 * Some instructions, however, allow you to use the 
 * registers A,B,C,D,E,H, & L as 16-bit registers by pairing 
 * them up in the following manner: AF,BC,DE, & HL.
 * 		----------------
 * 		| 15..8 | 7..0 |
 * 		----------------
 * 		|   A   |   F  |
 * 		|   B   |   C  |
 * 		|   D   |   E  |
 * 		|   H   |   L  |
 * 		|      SP      |
 * 		|      PC      |
 *      ----------------
 */
extern struct registers{
	union{
		struct {
			unsigned char F;
			unsigned char A;
		};
		unsigned short AF;
	};
	union{
		struct {
			unsigned char C;
			unsigned char B;
		};
		unsigned short BC;
	};
	union{
		struct {
			unsigned char E;
			unsigned char D;
		};
		unsigned short DE;
	};
	union{
		struct {
			unsigned char L;
			unsigned char H;
		};
		unsigned short HL;
	};
	unsigned short SP;
	unsigned short PC;
}registers;
//alam
extern unsigned short test;

extern struct dma{
    unsigned int timer;
    unsigned int prepare;
    unsigned int start;
    unsigned int running;
    unsigned short address;
}dmastate;

extern unsigned char crom[0x8000];
//extern unsigned char vram[0x2000];
//extern unsigned char sram[0x2000];
//extern unsigned char iram[0x2000];

extern unsigned char memory[0x10000];
extern unsigned char memory_SRAM[512];

#ifdef __cplusplus
}
#endif