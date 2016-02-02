#define LOG_FATAL    (1)
#define LOG_ERR      (2)
#define LOG_WARN     (3)
#define LOG_INFO     (4)
#define LOG_DBG      (5)

extern unsigned char *cart_ROM;
extern unsigned char cart_RAM[0x8000];
extern unsigned char active_RAM_bank;
extern int RAM_bank_enabled;
extern unsigned char active_ROM_bank;

int loadRom(const char *filename);
void cartridgeSwitchBanks(unsigned short address, unsigned char value);




