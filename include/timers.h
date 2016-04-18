void updateTimers(int cycles);
void updateFrequency(unsigned char value);
void updateDivider(void);

void timersTick(void);
void timersStepTIMA (void);
unsigned char timersGetDIV(void);
unsigned char timersGetTIMA(void);
unsigned char timersGetTAC(void);
unsigned char timersGetTMA(void);
void timersSetDIV (void);
void timersSetTIMA(unsigned char value);
void timersSetTAC (unsigned char value);
void timersSetTMA (unsigned char value);

extern struct timer{
    unsigned short internal;
    unsigned int enable;
    unsigned int frequency;
    unsigned int overflow;
    unsigned int tima;
    unsigned int tma;
    unsigned int div;
    unsigned int tac;
}timerstate;



