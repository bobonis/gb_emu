extern unsigned char *cart_ROM;
extern unsigned char *cart_RAM;
extern unsigned char active_RAM_bank;
extern int RAM_bank_enabled;
extern unsigned char active_ROM_bank;

int loadRom(const char *filename);
void cartridgeSwitchBanks(unsigned short address, unsigned char value);




