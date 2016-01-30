void gpu (int cycles);
void gpuChangeMode(int mode);
int gpuCheckStatus(void);
void gpuDrawScanline(void);
void gpuRenderSprites(void);
void gpuDrawSprite (unsigned char sprite);
void gpuRenderBackground (void);
void gpuPaintColour (unsigned char colour, unsigned short palette, int *red, int *green, int *blue);
extern unsigned char framebuffer[144][160][3];