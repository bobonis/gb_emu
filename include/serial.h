void serialSetControl(unsigned char value);
void serialUpdateClock(void);
void serialReset(void);
void serialPushBit(void);
void serialSendByte(unsigned char data);
unsigned char serialReceiveByte(void);

extern struct serial{
    unsigned int enabled;
    unsigned int timer;
    unsigned int clock;
    unsigned int totalbits;
}serialstate;