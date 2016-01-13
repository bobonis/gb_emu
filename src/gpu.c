#include "gpu.h"
#include "memory.h"
#include "interrupts.h"

#define SCAN_OAM  2 //Scanline (accessing OAM)
#define SCAN_VRAM 3 //Scanline (accessing VRAM)
#define H_BLANK   0 //Horizontal blank
#define V_BLANK   1 //Vertical blank


#define LCDC    0xFF40 //LCD control
#define STAT    0xFF41 //LCDC status
#define SCY     0xFF42 //Scroll Y
#define SCX     0xFF43 //Scroll X
#define LY      0xFF44 //Vertical line counter
#define LYC     0xFF45 //Vertical line coincidence
#define WY      0xFF4A //Window Y position
#define WX      0xFF4B //Window X position
 

unsigned char gpu_state = SCAN_OAM;
unsigned char gpu_line = 0;
int gpu_cycles;

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

    if (gpuCheckStatus() == FALSE){
        return;
    }    
    
    gpu_cycles += cycles;
    
    switch (gpu_state){
        
        case SCAN_OAM:
            if (gpu_cycles >= 80){
                gpuChangeMode(SCAN_VRAM);
            }
            break;

        case SCAN_VRAM:
            if (gpu_cycles >= 172){
                gpuChangeMode(H_BLANK);
            } 
            break;
            
        case H_BLANK:
            if (gpu_cycles >= 204){
                memory[LY] += 1;          //Scanning a line completed, move to next
                                
                if (memory[LY] > 143){
                    gpuChangeMode(V_BLANK);                      
                }
                else {
                    gpuChangeMode(SCAN_OAM);
                }

            } 
            break;
            
        case V_BLANK:
            if (gpu_cycles >= 456){
                memory[LY] += 1;
                gpu_cycles = 0;
                
                if (memory[LY] > 153){
                    memory[LY] = 0;
                    gpuChangeMode(SCAN_OAM);            
                }
            }
            break;
    }
};

/*
 * Check if LCD has been turned off.
 * This can only occur during VBLANK period.
 * If LCD off, set mode to V_BLANK
 */
int gpuCheckStatus(void){
    
    if (testBit(LCDC,7) == FALSE){
        gpu_cycles = 0;             //reset gpu timers
        memory[LY] = 0;             //set first scanline
        setBit(STAT,0,TRUE);
        setBit(STAT,1,FALSE);
        return FALSE;
    }
    
    return TRUE;
}

/*
 * Change gpu access mode
 * always reset gpu cycles
 * handle LCDC interrupt for each mode
 * handle coincidence bit
 */
void gpuChangeMode(int mode){
    
    gpu_cycles = 0;
    
    switch (mode){
        
        case SCAN_OAM: //mode 2
            if (testBit(STAT,5) == TRUE)
                triggerInterrupt(LCDC_INTERRUPT);
            gpu_state = SCAN_OAM;
            setBit(STAT,0,FALSE);
            setBit(STAT,1,TRUE);
            break;
                  
        case SCAN_VRAM: //mode 3

            gpu_state = SCAN_VRAM;
            setBit(STAT,0,TRUE);
            setBit(STAT,1,TRUE);
            break;
                    
        case H_BLANK: //mode 0
            if (testBit(STAT,3) == TRUE)
                triggerInterrupt(LCDC_INTERRUPT);
            gpu_state = SCAN_OAM;
            setBit(STAT,0,FALSE);
            setBit(STAT,1,FALSE);            
            break;        
        
        case V_BLANK: //mode 1
            if (testBit(STAT,4) == TRUE)
                triggerInterrupt(LCDC_INTERRUPT);
            gpu_state = V_BLANK;
            triggerInterrupt(VBLANK_INTERRUPT);
            setBit(STAT,0,TRUE);
            setBit(STAT,1,FALSE);
            break;        
    }
    
    if (memory[LY] == memory[LYC]){
        setBit(STAT,2,TRUE);
        if (testBit(STAT,6) == TRUE)
                triggerInterrupt(LCDC_INTERRUPT);
    }
    else{
        setBit(STAT,2,FALSE);
    }
    
}

/*
 * Region	    Usage
 * ---------    -----------------------------
 * 8000-87FF	Tile set #1: tiles 0-127
 * 8800-8FFF	Tile set #1: tiles 128-255
 *              Tile set #0: tiles -1 to -128
 * 9000-97FF	Tile set #0: tiles 0-127
 * 9800-9BFF	Tile map #0
 * 9C00-9FFF	Tile map #1
 */
 
 void gpuDrawScanline(void){
     
     int using_signed = FALSE;
     int tileset_number;
     
     unsigned char tile[16];
     
     unsigned char posX = readMemory8(SCX);
     unsigned char posY = readMemory8(SCY) + readMemory8(LY);
 
     int tileset_memory_addr;     
     int tileset_offset;
     int tilemap_memory_addr;     
     int tilemap_offset;





     //find which tilemap is in use
     if (testBit(LCDC,3) == FALSE){
        tilemap_memory_addr = 0x9800;
        using_signed = TRUE;
     }
     else{
        tilemap_memory_addr = 0x9C00;
     }
     
     //find tile in tilemap
     tilemap_offset = (posX / 8) + ((posY / 8) * 32);
     
     //find tileset number
     if (using_signed == TRUE)
        tileset_number = (signed)readMemory8(tilemap_memory_addr + tilemap_offset);
     else
        tileset_number = readMemory8(tilemap_memory_addr + tilemap_offset);

     //find tile
     if (testBit(LCDC,4) == FALSE){
        tileset_memory_addr = 0x8800;
     }
     else{
        tileset_memory_addr = 0x8000;
     }
     if (using_signed == TRUE)
        tileset_offset = tileset_memory_addr + ((tileset_number + 128) * 16);
     else
        tileset_offset = tileset_memory_addr + (tileset_number * 16);
     
     //read tile
 }