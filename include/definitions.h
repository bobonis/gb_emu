/* HARDWARE */

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define MAXSPRITES 10

#define SCAN_OAM  2 /* Scanline (accessing OAM) */
#define SCAN_VRAM 3 /* Scanline (accessing VRAM) */
#define H_BLANK   0 /* Horizontal blank */
#define V_BLANK   1 /* Vertical blank */

#define SCAN_OAM_CYCLES   80
#define SCAN_VRAM_CYCLES 172
#define H_BLANK_CYCLES   204
#define V_BLANK_CYCLES   456

#define CLOCKSPEED 4194304
#define FREQ_1   4096 /* 1024 cycles */
#define FREQ_2  16384 /*  256 cycles */
#define FREQ_3  65536 /*   64 cycles */
#define FREQ_4 262144 /*   16 cycles */

/* CPU FLAGS */

#define ZERO_F          7
#define SUBSTRACT_F     6
#define HALF_CARRY_F    5
#define CARRY_F         4

/* MEMORY ADDRESS */

#define SB      0xFF01
#define SC      0xFF02
#define DIV     0xFF04
#define TIMA    0xFF05
#define TMA     0xFF06
#define TAC     0xFF07
#define IER     0xFFFF /* Interrupt Enable Register */
#define IFR     0xFF0F /* Interrupt Request Register */
#define OAM     0xFE00 /* Sprite attribute memory */
#define SPT     0x8000 /* Sprite pattern table */
#define LCDC    0xFF40 /* LCD control */
#define STAT    0xFF41 /* LCDC status */
#define SCY     0xFF42 /* Scroll Y */
#define SCX     0xFF43 /* Scroll X */
#define LY      0xFF44 /* Vertical line counter */
#define LYC     0xFF45 /* Vertical line coincidence */
#define BGP     0xFF47 /* BG & Window Palette Data */
#define OBP0    0xFF48 /* Object Palette 0 Data */
#define OBP1    0xFF49 /* Object Palette 1 Data */
#define WY      0xFF4A /* Window Y position */
#define WX      0xFF4B /* Window X position */

/* INTERRUPTS */

#define VBLANK_INTERRUPT 0
#define LCDC_INTERRUPT   1
#define TIMER_INTERRUPT  2
#define SERIAL_INTERRUPT 3
#define JOYPAD_INTERRUPT 4

/* SYMBOLS */

#define TRUE 1
#define FALSE 0
#define DELAY 3
#define SOON 2
#define NOW 1
#define OFF 0

/* FLAGS */
#define USINGBIOS TRUE
#define VSYNC TRUE



#define LOG_FATAL    (1)
#define LOG_ERR      (2)
#define LOG_WARN     (3)
#define LOG_INFO     (4)
#define LOG_DBG      (5)

#define PRINT_RED     "\x1b[31m"
#define PRINT_GREEN   "\x1b[32m"
#define PRINT_YELLOW  "\x1b[33m"
#define PRINT_BLUE    "\x1b[34m"
#define PRINT_MAGENTA "\x1b[35m"
#define PRINT_CYAN    "\x1b[36m"
#define PRINT_RESET   "\x1b[0m"