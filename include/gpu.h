void gpu (int cycles);
void gpuChangeMode(int mode);
int gpuCheckStatus(void);
void gpuDrawScanline(void);
void gpuRenderSprites(void);
void gpuDrawSprite (int sprite);
void gpuRenderBackground (void);
extern unsigned char framebuffer[144][160][3];