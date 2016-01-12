#include "gpu.h"

#define SCAN_OAM  2 //Scanline (accessing OAM)
#define SCAN_VRAM 3 //Scanline (accessing VRAM)
#define HBLANK    0 //Horizontal blank
#define VBLANK    1 //Vertical blank



#define LY      0xFF44 //Vertical line counter

unsigned char gpu_state = SCAN_OAM;
unsigned char gpu_line = 0;

/*  Period  GPU mode          Time spent   (clocks)
 * -----------------------------------------------------
 * Scanline (accessing OAM)       2            80
 * Scanline (accessing VRAM)      3           172
 * Horizontal blank               0           204
 * One line (scan and blank)                  456
 * Vertical blank                 1          4560 (10 lines)
 * Full frame (scans and vblank)            70224
 */
void gpu (int cycles){
    
    switch (gpu_state){
        case SCAN_OAM:
            if (cycles >= 80){
                
                gpu_state = SCAN_VRAM;
            }
            break;
        case SCAN_VRAM:
            if (cycles >= 252){
                
                gpu_state = HBLANK;
            } 
            break;
        case HBLANK:
            if (cycles >= 456){
                memory[LY] += 1;            //Scanning a line completed, move to next
                
                if (memory[LY] > 143){    
                    gpu_state = VBLANK;   //if next scanline is out of screen, start VBLANK
                }
                else {
                    gpu_state = SCAN_OAM; //else go to next scanline
                }

            } 
            break;
        case VBLANK:
            if (cycles >= 4560){
                cycles = 0;
                memory[LY] = 0;
                gpu_state = SCAN_OAM;
            }
            break;
    }
};
