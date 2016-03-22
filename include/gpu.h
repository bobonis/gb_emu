void gpu (int cycles);
void gpuChangeMode(int mode);
int gpuCheckStatus(void);
void gpuDrawScanline(void);
void gpuRenderSprites(void);
void gpuRenderSprites1(void);
void gpuDrawSprite (unsigned char sprite);
void gpuRenderBackground (void);
void gpuPaintColour (unsigned char colour, unsigned short palette, int *red, int *green, int *blue);
void gpuStop (void);
void gpuSetStatus(unsigned char value);
void gpuCompareLine (void);

extern unsigned char framebuffer[144][160][3];

extern int gpu_reading;
extern int dma_timer;
int gpuAdjustCycles (void);

struct sprite
{
    unsigned char Ypos;
    unsigned char Xpos;
    unsigned char pattern;
    unsigned char priority;
    unsigned char Yflip;
    unsigned char Xflip;
    unsigned char palette;
};