#define IER 0xFFFF // Interrupt Enable Register
#define IFR 0xFF0F // Interrupt Request Register

#define VBLANK 0
#define LCDC   1
#define TIMER  2
#define SERIAL 3
#define JOYPAD 4


extern unsigned char interruptMaster; // Interrupt Master Enable

void triggerInterrupt(int interrupt);
void handleInterrupts(void);