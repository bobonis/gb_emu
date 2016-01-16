typedef int bool;
#define TRUE 1
#define FALSE 0

void memCopy(unsigned char *memory, unsigned short start, unsigned char *buffer, unsigned short length);
void reset (void);
void setFlag (unsigned char flag);
void resetFlag (unsigned char flag);
int testFlag (unsigned char flag);
void add (unsigned short value1, unsigned short value2);
void adc (unsigned short value1, unsigned short value2);
void xor (unsigned short value1);
void comp (unsigned char value1);
void sub (unsigned char value1);
void inc (unsigned char *value1);
void dec (unsigned char *value1);
void writeMemory (unsigned short pos, unsigned char value);
unsigned char readMemory (unsigned short pos);
void setBit(unsigned short pos, unsigned char bit, bool value);
bool testBit(unsigned short pos, unsigned char bit);
void stackPush16 (unsigned short value);
unsigned short stackPop16 (void);
unsigned char readMemory8 (unsigned short address);
unsigned short readMemory16 (unsigned short address);
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
struct registers{
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
}extern registers;
//alam
extern unsigned short test;
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
 
extern unsigned char crom[0x8000];
//extern unsigned char vram[0x2000];
//extern unsigned char sram[0x2000];
//extern unsigned char iram[0x2000];

extern unsigned char memory[0xFFFF];


