#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char *cart_ROM;
extern unsigned char *cart_RAM;
extern unsigned char active_RAM_bank;
extern int RAM_bank_enabled;
extern unsigned char active_ROM_bank;
extern int RTC_register_enabled;
extern unsigned char RTC_register_mapped;
extern int MBC_type;
extern char cart_game[];

int loadRom(const char *filename);
void cartridgeSwitchBanks(unsigned short address, unsigned char value);
int file_exist (char *);


#ifdef __cplusplus
}
#endif
