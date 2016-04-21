void gpu (void);
void gpu1 (void);
void gpuChangeMode(int mode);
int gpuCheckStatus(void);
void gpuDrawScanline(void);
void gpuRenderSprites(void);
void gpuRenderBackground (void);
void gpuPaintColour (unsigned char colour, unsigned short palette, int *red, int *green, int *blue);
void gpuStop (void);
void gpuSetStatus(unsigned char value);
void gpuCompareLine (void);
int gpuCountSprites(void);
void gpuUpdateSprites(void);

extern unsigned char framebuffer[144][160][3];
extern unsigned char gpu_state;
extern int gpu_reading;

int gpuAdjustCycles (void);

typedef struct
{
    unsigned int number;
    unsigned int draw;
    unsigned char Ypos;
    unsigned char Xpos;
    unsigned char pattern;
    unsigned char priority;
    unsigned char Yflip;
    unsigned char Xflip;
    unsigned char palette;
}sprite;

extern struct gpu
{
    unsigned int enable;
    int clock;
    unsigned int mode;
    unsigned int interrupt;
    unsigned int line;
    unsigned int firstframe;
    unsigned int lyc;
}gpustate;



/*
Stat timing
===========
This timing table is accurate within 4 cycles:
           | stat = 2 | stat = 3 | stat = 0 |
No sprites |    80    |    172   |    204   |
1 sprite   |    80    |    182   |    194   |
2 sprites  |    80    |    192   |    184   |
3 sprites  |    80    |    202   |    174   |
4 sprites  |    80    |    212   |    164   |
5 sprites  |    80    |    222   |    154   |
6 sprites  |    80    |    232   |    144   |
7 sprites  |    80    |    242   |    134   |
8 sprites  |    80    |    252   |    124   |
9 sprites  |    80    |    262   |    114   |
10 sprites |    80    |    272   |    104   |
In other words, each sprite on a line makes stat 3 last 10 cycles longer.
For lines 1 - 143 when stat changes to 2 the line counter is incremented.
Line 153 is little odd timing wise. The line counter stays 153 for ~4 clock cycles
and is then rolls over to 0.
When the line counter is changed it gets checked against the lyc register.
Here is a detailed run of the STAT and LY register together with LYC set to 3 on a
dmg and mgb. The time between each sample is 4 clock cycles:
STAT:
22222222 22233333 33333333 33333333 33333333 33333333 33333300 00000000 00000000 00000000
00000000 00000000 00000000 06666666 66666666 66666777 77777777 77777777 77777777 77777777
77777777 44444444 44444444 44444444 44444444 44444444 44444444 44022222 22222222
  LY:
33333333 33333333 33333333 33333333 33333333 33333333 33333333 33333333 33333333 33333333
33333333 33333333 33333333 44444444 44444444 44444444 44444444 44444444 44444444 44444444
44444444 44444444 44444444 44444444 44444444 44444444 44444444 44555555 55555555
                           ^                                     ^
As you can see, it seems as though the LY register is incremented slightly before the STAT
register is changed, resulting in a short period where STAT goes 0 before going to 2. This
bug/feature has been fixed in the CGB and AGB.
Around lines 152-153-0 the picture becomes as follows:
STAT:
11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
11111111 11111111 11111111 15555555 55555555 55555555 55555555 55555555 55555555 55555555
55555555 55555555 55555555 55555555 55555555 55555555 55555555 55111111 11111111 11111111
11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
11111111 11110222 22222222 22222222 23333333 33333333 33333333 33333333 33333333
  LY:
77777777 77777777 77777777 77777777 77777777 77777777 77777777 77777777 77777777 77777777
77777777 77777777 77777777 88888888 88888888 88888888 88888888 88888888 88888888 88888888
88888888 88888888 88888888 88888888 88888888 88888888 88888888 88900000 00000000 00000000
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
The full STAT/LY value state machine.
=====================================
The timing information below is with sprites disabled.
For STAT we only show the lower 3 bits and for LY only the lower 5 bits of the full
register. Each digit stands for 4 clock cycles (the smallest measurable unit on a
dmg or mgb). When the video hardware is switched on the LY register is set 0 and
the STAT mode is 0. The values for STAT and LY will change as follows:
STAT 000000000000000000003333333333333333333333333333333333333333333000000000000000000000000000000000000000000000000000
  LY 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001  line #0
     ^LY=LYC bit can get set here                                                             LY=LYC bit is reset here^
STAT 222222222222222222223333333333333333333333333333333333333333333000000000000000000000000000000000000000000000000000
  LY 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111112  line #1
     :
     :
STAT 222222222222222222223333333333333333333333333333333333333333333000000000000000000000000000000000000000000000000000
  LY FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0  line #143
STAT 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111
  LY 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001  line #144
     :
     :
STAT 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111
  LY 888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888889  line #152
STAT 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110
  LY 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000  line #153
     ^
     LY=LYC interrupt for 153 can get triggered here
STAT 222222222222222222223333333333333333333333333333333333333333333000000000000000000000000000000000000000000000000000
  LY 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001  line #0
STAT 222222222222222222223333333333333333333333333333333333333333333000000000000000000000000000000000000000000000000000
  LY 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111112  line #1
     :
     etc
*/



/*
ISTAT98.TXT 1998 BY MARTIN KORTH
--------------------------------


Interrupt INT 48h - LCD STAT
----------------------------


The STAT register (FF41) selects the conditions that will generate this
interrupt (expecting that interrupts are enabled via EI or RETI and that
IE.1 (FFFF.1) is set).
  STAT.3        HBLANK  (start of mode 0)
  STAT.4        VBLANK  (start of mode 1) (additional to INT 40)
  STAT.5        OAM     (start of mode 2 and mode 1)
  STAT.6        LY=LYC  (see info about LY=00)


If two STAT-condiditions come true at the same time only one INT 48 is
generated. This happens in combinations
  LYC=01..90  and  OAM     at the same time  (see info about LY=00)
  LYC=90      and  VBLANK  at the same time
  OAM         and  VBLANK  at the same time
HBLANK and LYC=00 and LYC=91..99 are off-grid and cannot hit others.


Some STAT-conditions cause the following STAT-condition to be ignored:
  Past  VBLANK           following  LYC=91..99,00        is ignored
  Past  VBLANK           following  OAM         (at 00)  is ignored
  Past  LYC=00 at 99.2   following  OAMs (at 00 and 01) are ignored
  Past  LYC=01..8F       following  OAM     (at 02..90)  is ignored
  Past  LYC=00..8F       following  HBLANK  (at 00..8F)  is ignored
  Past  LYC=8F           following  VBLANK               is ignored
  Past  HBLANK           following  OAM                  is ignored
  Past  HBLANK at 8F     following  VBLANK               is ignored


If the OAM condition occurs, everything following -is- recognized.
An ignored VBLANK condition means that INT 48h does not produce a V-Blank
interrupt, INT 40h is not affected and still produces V-Blank interrupts.


The last LY period (LY=99) is a shorter than normal LY periodes. It is followed
by the first LY period (LY=00) this period is longer than normal periodes.
  LY value    clks    description
  -------------------------------
  LY=01..8F   456     at the same moment than OAM
  LY=90       456     at the same moment than pseudo-OAM and VBLANK
  LY=91..98   456     during vblank (similiar to LY=01..8F)
  LY=99       ca.56   similiar to LY=91..98, but shorter
  LY=00       ca.856  starts during vblank, then present until second OAM
Because of the pre-started long LY=00 period, LYC=00 occurs within vblank
period and before first OAM (where we would have expected it)


*EOF*

*/