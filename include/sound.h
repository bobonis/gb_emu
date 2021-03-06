#include <stdint.h>

#ifdef __cplusplus 
extern "C" {
#endif
void sound_tick(int clockCycles);

void init_apu(void);

void write_memory_apu(unsigned short addr, unsigned char val);

unsigned char read_memory_apu(unsigned short addr);

void end_frame(void);

void soundWriteRegister(unsigned short address,unsigned char value);
unsigned char soundReadRegister(unsigned short address);
void soundReset(void);
void soundResetControl(void);
void soundResetRegisters(void);
void soundResetChannel(unsigned int channel);
void soundResetBufferPointers(void);
void soundTurnOn(void);
void soundTurnOff(void);
void soundTick(void);
void soundTickSampler(void);
void soundTickProgrammableCounter(void);
void soundTickLenghthCounter(void);
void soundTickSweepCounter(void);
void soundTickEnvelope(void);
void soundTickFrequency(void);
void soundTickDuty(int channel);
void soundTicLFSR(void);
void soundMix(void);
void channelEnable(unsigned int);
void channelDisable(unsigned int);
unsigned short channelCalculateSweepFreq(void);
void update_stream(void *, unsigned char *, int);
void switchChannel(int);
void soundSync(int);

void audioInit(void);

#ifdef __cplusplus
}
#endif
