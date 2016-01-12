#define IER 0xFFFF // Interrupt Enable Register
#define IFR 0xFF0F // Interrupt Request Register

#define VBLANK_INTERRUPT 0
#define LCDC_INTERRUPT   1
#define TIMER_INTERRUPT  2
#define SERIAL_INTERRUPT 3
#define JOYPAD_INTERRUPT 4


extern unsigned char interruptMaster; // Interrupt Master Enable

void triggerInterrupt(int interrupt);
void handleInterrupts(void);