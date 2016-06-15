#include <stdint.h>

#ifdef __cplusplus 
extern "C" {
#endif
void sound_tick(int clockCycles);

void init_apu(void);

void write_memory_apu(unsigned short addr, unsigned char val);

unsigned char read_memory_apu(unsigned short addr);

void end_frame(void);

#ifdef __cplusplus
}
#endif
