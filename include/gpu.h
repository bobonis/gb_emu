void gpu (int cycles);
void gpuChangeMode(int mode);
int gpuCheckStatus(void);
void gpuDrawScanline(void);

extern unsigned char framebuffer[160][144][3];