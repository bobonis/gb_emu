#include <stdio.h>
#include "gpu.h"
#include "memory.h"
#include "interrupts.h"
#include "display.h"
#include <stdio.h>

#define SCAN_OAM  2 //Scanline (accessing OAM)
#define SCAN_VRAM 3 //Scanline (accessing VRAM)
#define H_BLANK   0 //Horizontal blank
#define V_BLANK   1 //Vertical blank

#define OAM     0xFE00 //Sprite attribute memory
#define SPT     0x8000 //Sprite pattern table

#define LCDC    0xFF40 //LCD control
#define STAT    0xFF41 //LCDC status
#define SCY     0xFF42 //Scroll Y
#define SCX     0xFF43 //Scroll X
#define LY      0xFF44 //Vertical line counter
#define LYC     0xFF45 //Vertical line coincidence
#define BGP     0xFF47 //BG & Window Palette Data
#define WY      0xFF4A //Window Y position
#define WX      0xFF4B //Window X position
 
unsigned char framebuffer[144][160][3];
unsigned char gpu_state = SCAN_OAM;
unsigned char gpu_line = 0;
int gpu_cycles = 0;

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
                gpuDrawScanline();
                memory[LY] += 1;          //Scanning a line completed, move to next
                //display();              //temporary solution

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
   //                 display();           
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
            gpu_state = H_BLANK;
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
     int using_window = FALSE;
     int tileset_number;
     int bit_1;
     int bit_2;
     int pixel;
     
     
     unsigned char colour;
     unsigned char red;
     unsigned char green;
     unsigned char blue;
     
     unsigned char posX = readMemory8(SCX);
     unsigned char posY = readMemory8(SCY) + readMemory8(LY);
     unsigned char windowX = readMemory8(WX) - 7;
     unsigned char windowY = readMemory8(WY);
 
     unsigned short tileset_start_addr;    //Tile Set start address in memory
     unsigned short tilemap_start_addr;    //Tile Map start address in memory
     unsigned short window_start_addr;     //Window start address in memory
     
     unsigned short tileset_offset;
     unsigned short tilemap_offset;    

     /*
      * RENDER BACKROUND 
      */

     //Initialize flags and memory addresses
     if (testBit(LCDC,5) == TRUE){         //Window Display Enable (0=Off, 1=On)
        if (readMemory8(LY) >= readMemory8(WY)){//Current scanline is after window position
            using_window = TRUE;
            posY = readMemory8(LY) - windowY;
            if (testBit(LCDC,6) == TRUE){  //Window Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
                window_start_addr = 0x9C00;
            }
            else{
                window_start_addr = 0x9888;
            }
        }
     }

     if (using_window == TRUE){
        if (testBit(LCDC,6) == FALSE){     //BG Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
            tilemap_start_addr = 0x9800;
            using_signed = TRUE;            //tilemap number is signed number
        }
        else{
            tilemap_start_addr = 0x9C00;
        }
     }
     else{
        if (testBit(LCDC,3) == FALSE){     //BG Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
            tilemap_start_addr = 0x9800;
            using_signed = TRUE;            //tilemap number is signed number
        }
        else{
            tilemap_start_addr = 0x9C00;
        }         
     }
     
     if (testBit(LCDC,4) == FALSE){     //BG & Window Tile Data Select (0=8800-97FF, 1=8000-8FFF)
        tileset_start_addr = 0x8800;
     }
     else{
        tileset_start_addr = 0x8000;
     }
     
     //Loop for every pixel in the scanline
     for (pixel=0;pixel<160;pixel++){
        
        //should read tile for every pixel??
        posX +=pixel;
        if (using_window){
            if (pixel >= windowX){
                posX = pixel - windowX;
            }
        }
        
        
        //should read tile for every pixel??
        posX +=pixel;
        if (using_window){
            if (pixel >= windowX){
                posX = pixel - windowX;
            }
        }
        
        
        tilemap_offset = (posX / 8) + ((posY / 8) * 32);    //find tile in tilemap
        
        if (using_signed == TRUE)   //find tileset number
            tileset_number = (signed)readMemory8(tilemap_start_addr + tilemap_offset);
        else
            tileset_number = readMemory8(tilemap_start_addr + tilemap_offset);
            
        if (using_signed == TRUE)   //find tile
            tileset_offset = tileset_start_addr + ((tileset_number + 128) * 16);
        else
            tileset_offset = tileset_start_addr + (tileset_number * 16);
            
        bit_1 = (readMemory8(tileset_offset + ((posY % 8) * 2)) >> (posX % 8)) & 0x01; //do some magic
        bit_2 = (readMemory8(tileset_offset + ((posY % 8) * 2) + 1) >> (posX % 8)) & 0x01; //do some more magic   
        colour = (bit_2 << 1) | bit_1;

        //Pass colour through the palette
        switch (colour){
            case 00:
                colour = testBit(BGP,0) | (testBit(BGP,1) << 1);
                break;
            
            case 01:
                colour = testBit(BGP,2) | (testBit(BGP,3) << 1);
                break;
                
            case 10:
                colour = testBit(BGP,4) | (testBit(BGP,5) << 1);
                break;

            case 11:
                colour = testBit(BGP,6) | (testBit(BGP,7) << 1);          
                break;
        }
        //Set actuall dot colour
        switch (colour){
            case 00:
                red = 0xFF; green = 0xFF; blue = 0xFF;
                break;
            
            case 01:
                red = 0xCC; green = 0xCC; blue = 0xCC;
                break;
                
            case 10:
                red = 0x77; green = 0x77; blue = 0x77;
                break;

            case 11:
                red = 0x00; green = 0x00; blue = 0x00;         
                break;
        }
        //Finaly...
        /*
        framebuffer[pixel][readMemory8(LY)][0] = red;
        framebuffer[pixel][readMemory8(LY)][1] = green;
        framebuffer[pixel][readMemory8(LY)][2] = blue;
        */
        framebuffer[readMemory8(LY)][pixel][0] = red;
        framebuffer[readMemory8(LY)][pixel][1] = green;
        framebuffer[readMemory8(LY)][pixel][2] = blue;
        
     }

     /*
      * RENDER SPRITES 
      */          
      if (testBit(LCDC,1)){
          //gpuRenderSprites();
      }
 }
/* 4 bytes for each sprite starting at 0xFE00
 * byte 0 - sprite Y position
 * byte 1 - sprite X position 
 * byte 2 - pattern number 
 * byte 3 - attributes
 *   Bit7: Sprite to Background Priority
 *   Bit6: Y flip
 *   Bit5: X flip
 *   Bit4: Palette number
 *   Bit3: Not used in standard gameboy
 *   Bit2-0: Not used in standard gameboy 
 */
void gpuRenderSprites(void){
    
    
    
    int sprite;
    int scanline = readMemory8(LY);
    unsigned char posY;
    unsigned char posX;
    int pixel;
    int sprites_on_scanline = 0;
    unsigned char sprite_size = 8;

    if (testBit(LCDC,2) == TRUE){
        //using_8x16_sprites = TRUE;
        sprite_size = 16;
    }

   
    for (sprite = 0; sprite < 40; sprite++){
        
        posY = readMemory8(OAM + (sprite * 4)) - 16;
        posX = readMemory8((OAM + (sprite * 4)) + 1 ) - 8;
        
        /*for accurate emulation we have to check sprite priorites
         *we will start checking from the last pixel in each scanline
         */
        
        //Check if part of sprite is visible in current scanline
        if ((scanline >= posY) && (scanline < (posY + sprite_size))){
            sprites_on_scanline++;  //Count total sprites visible in current scanline
        }
    }

    for (pixel = 159; pixel >=0; pixel--){
        for (sprite = 39; sprite >= 0; sprite--){
        
            posY = readMemory8(OAM + (sprite * 4)) - 16;
            posX = readMemory8((OAM + (sprite * 4)) + 1 ) - 8;
            
            if ((scanline >= posY) && (scanline < (posY + sprite_size))){ //if sprite is visible on current scanline
                if (pixel = posX){   //if sprite starts on current pixel
                    if (sprites_on_scanline > 10){
                        //limit of sprites reached on current scanline
                    }
                    else{ 
                        gpuDrawSprite(sprite);//Draw sprite
                    }                  
                }

            }
            sprites_on_scanline--;
        }
    }
}


void gpuDrawSprite (int sprite){
    int using_8x16_sprites = FALSE;
    unsigned char sprite_size = 8;
    unsigned char sprite_number;
    unsigned char sprite_Y;
    unsigned char sprite_X;
    unsigned char scanline = readMemory8(LY);
    unsigned short sprite_start_address;
    
    //find sprite coordinates
    sprite_Y = readMemory8(OAM + (sprite * 4));
    sprite_X = readMemory8((OAM + (sprite * 4)) + 1 );    
    //find sprite pattern number
    sprite_number = readMemory8(OAM + (sprite * 4) + 2);

    
    //find sprite memory start adress    
    if (testBit(LCDC,2) == TRUE){
        using_8x16_sprites = TRUE;
        sprite_size = 16;
       // sprite_start_address = SPT + sprite_number *  
    }
    else{
       // sprite_start_address = 
    }



}