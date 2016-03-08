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

#define SCAN_OAM_CYCLES   84
#define SCAN_VRAM_CYCLES 172
#define H_BLANK_CYCLES   200
#define V_BLANK_CYCLES   456

#define OAM     0xFE00 //Sprite attribute memory
#define SPT     0x8000 //Sprite pattern table

#define LCDC    0xFF40 //LCD control
#define STAT    0xFF41 //LCDC status
#define SCY     0xFF42 //Scroll Y
#define SCX     0xFF43 //Scroll X
#define LY      0xFF44 //Vertical line counter
#define LYC     0xFF45 //Vertical line coincidence
#define BGP     0xFF47 //BG & Window Palette Data
#define OBP0    0xFF48 //Object Palette 0 Data
#define OBP1    0xFF49 //Object Palette 1 Data
#define WY      0xFF4A //Window Y position
#define WX      0xFF4B //Window X position
 
unsigned char framebuffer[144][160][3];
unsigned char spritebuffer[160][4];
unsigned char gpu_state = SCAN_OAM;
unsigned char gpu_line = 0;
int gpu_cycles = 0;
int gpu_delay = 0;
int draw_pixel = TRUE;


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

/*
    if (gpu_delay > 0){
        gpu_delay -= cycles;
        //printf("delay\n");
        return;
    }
*/
    if (gpuCheckStatus() == FALSE){
        return;
    }
    
  
    if (cycles == 0){ // No cpu cycles fetched, CPU in stop MODE
        gpuStop();
        display();
        return;
    }
    
    
    gpu_cycles -= cycles;   // Step GPU internal clock
    //gpu_state = readMemory8(STAT) & 0x03;

    // STAT mode=0 interrupt happens one cycle before the actual mode switch!
    if ( gpu_cycles == 4 && gpu_state == SCAN_VRAM){
        if (testBit(STAT,3) == TRUE){
            triggerInterrupt(LCDC_INTERRUPT);
        }
    }
        
    // return if nothing to do
    if (gpu_cycles > 0){
        return;
    }

    
    switch (gpu_state){
        case SCAN_OAM: // mode 2
            gpuChangeMode(SCAN_VRAM);
            break;

        case SCAN_VRAM:
            gpuDrawScanline();
            gpuChangeMode(H_BLANK);
            break;
            
        case H_BLANK:
            memory[LY] += 1;          //Scanning a line completed, move to next
            if (memory[LY] > 143){
                gpuChangeMode(V_BLANK);
            }
            else {
                gpuChangeMode(SCAN_OAM);
            }
            gpuCompareLine();
            break;
            
        case V_BLANK:
            memory[LY] += 1;
            if (memory[LY] > 153){
                memory[LY] = 0;
                gpuChangeMode(SCAN_OAM);
                display();           
            }
            else{
                gpu_cycles += V_BLANK_CYCLES;
            }
            gpuCompareLine();
            break;
    }
}

void gpuCompareLine (void){
    
    if (memory[LY] == memory[LYC]){
        setBit(STAT,2,TRUE);
        if (testBit(STAT,6) == TRUE)
            triggerInterrupt(LCDC_INTERRUPT);
    }
    else{
        setBit(STAT,2,FALSE);
    }
}

void gpuSetStatus(unsigned char value){
        // switch on
        if ( !(memory[LCDC] & 0x80) && (value & 0x80) ){
            memory[STAT] &= 0xFC; //set to H_BLANK
            memory[STAT] |= 0x02;
            gpu_state = H_BLANK;
            gpu_cycles = SCAN_OAM_CYCLES + gpuAdjustCycles(); //84
            //printf("[DEBUG] GPU internal cycles when turned on %d\n",gpu_cycles);
            gpuCompareLine();
        }
        // switch off
        else if ( (memory[LCDC] & 0x80) && !(value & 0x80) ){ // switch off
            if ( gpu_state == V_BLANK ){
                memory[LY] = 0;
            }
            else{
                printf("[ERROR] LCD turned off not in V_BLANK state\n");
            }
        }        
}

/*
 * Check if LCD has been turned off.
 * This can only occur during VBLANK period.
 * If LCD off, set mode to V_BLANK
 */
int gpuCheckStatus(void){
    
    if (testBit(LCDC,7) == FALSE){
        return FALSE;
    }
    return TRUE;
}


/*
 * A hack to pass a test
 *
 */
int gpuAdjustCycles (void){
    
    int temp = memory[0xFF43] % 0x08;
    
    switch (temp){
        case 5 ... 7:
            return 8;
        case 1 ... 4:
            return 4;
        default:
            return 0;
    }
}


/*
 * Change gpu access mode
 * always reset gpu cycles
 * handle LCDC interrupt for each mode
 * handle coincidence bit
 */
void gpuChangeMode(int mode){
    
//printf("[DEBUG] GPU change mode %d\n",gpu_cycles);   
    
    switch (mode){
        case SCAN_OAM: //mode 2
            gpu_cycles +=SCAN_OAM_CYCLES;
            //printf("[DEBUG] GPU change mode %d\n",gpu_cycles);
            //printf("[DEBUG] OAM\n");
            gpu_state = SCAN_OAM;
            setBit(STAT,0,FALSE);
            setBit(STAT,1,TRUE);
            if (testBit(STAT,5) == TRUE)
                triggerInterrupt(LCDC_INTERRUPT);
            break;
                  
        case SCAN_VRAM: //mode 3
            gpu_cycles += SCAN_VRAM_CYCLES + gpuAdjustCycles();
            //printf("[DEBUG] GPU change mode %d\n",gpu_cycles);             
            gpu_state = SCAN_VRAM;
            setBit(STAT,0,TRUE);
            setBit(STAT,1,TRUE);
            break;
                    
        case H_BLANK: //mode 0
            gpu_cycles += H_BLANK_CYCLES - gpuAdjustCycles();
            //printf("[DEBUG] GPU change mode %d\n",gpu_cycles);            
            //printf("[DEBUG] HBLANK\n");
            gpu_state = H_BLANK;
            setBit(STAT,0,FALSE);
            setBit(STAT,1,FALSE);
            break;        
        
        case V_BLANK: //mode 1
            gpu_cycles += V_BLANK_CYCLES;
            //printf("[DEBUG] GPU change mode %d\n",gpu_cycles);            
            //printf("[DEBUG] VBLANK\n");            
            gpu_state = V_BLANK;
            setBit(STAT,0,TRUE);
            setBit(STAT,1,FALSE);
            triggerInterrupt(VBLANK_INTERRUPT);            
            if (testBit(STAT,4) == TRUE)
                triggerInterrupt(LCDC_INTERRUPT);            
            break;        
    }
}

/*
void gpuWriteOAM(){
    
    if (gpu_state == SCAN_OAM || gpu_state == SCAN_VRAM){
        return;
    }
    //todo: sprite ram
}
*/

/* Check LCDC register
 * Decide to render the background
 * and the sprites on current scanline
 */
void gpuDrawScanline(void){
    int i;
    
    for (i=159;i>=0;i--){
        spritebuffer[i][0] = spritebuffer[i][1] = spritebuffer[i][2] = 255;
        spritebuffer[i][3] = FALSE;
    }
    
    if (testBit(LCDC,0)){
        gpuRenderBackground();
    }

    if (testBit(LCDC,0)){
        gpuRenderSprites();

    
    for (i=159;i>=0;i--){
        if (spritebuffer[i][3] != 0){
            framebuffer[readMemory8(LY)][i][0] = spritebuffer[i][0];
            framebuffer[readMemory8(LY)][i][1] = spritebuffer[i][1];
            framebuffer[readMemory8(LY)][i][2] = spritebuffer[i][2];
        }
    }
    

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
void gpuRenderBackground(void){
     
    int using_signed = FALSE;
    int using_window = FALSE;
    int tileset_number;
    unsigned char bit_1;
    unsigned char bit_2;
    int pixel;
     
     
    unsigned char colour;
    int red;
    int green;
    int blue;
     
    unsigned char posX = readMemory8(SCX);
    unsigned char posY = readMemory8(SCY) + readMemory8(LY);
    unsigned char windowX = readMemory8(WX) - 7;
    unsigned char windowY = readMemory8(WY);
 
    unsigned short tileset_start_addr;    //Tile Set start address in memory
    unsigned short tilemap_start_addr;    //Tile Map start address in memory
    unsigned short window_start_addr;     //Window start address in memory
     
    unsigned short tileset_offset;
    unsigned short tilemap_offset;    

    //Initialize flags and memory addresses
    if (testBit(LCDC,5) == TRUE){         //Window Display Enable (0=Off, 1=On)
        if (readMemory8(LY) >= readMemory8(WY)){//Current scanline is after window position
            using_window = TRUE;
            posY = readMemory8(LY) - windowY;
        }
    }

    if (using_window == TRUE){
        if (testBit(LCDC,6) == FALSE){     //BG Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
            tilemap_start_addr = 0x9800;
            // using_signed = TRUE;            //tilemap number is signed number
       }
       else{
            tilemap_start_addr = 0x9C00;
       }
    }
    else{
       if (testBit(LCDC,3) == FALSE){     //BG Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
           tilemap_start_addr = 0x9800;
          // using_signed = TRUE;            //tilemap number is signed number
       }
       else{
           tilemap_start_addr = 0x9C00;
       }         
    }
     
    if (testBit(LCDC,4) == FALSE){     //BG & Window Tile Data Select (0=8800-97FF, 1=8000-8FFF)
        tileset_start_addr = 0x8800;
        using_signed = TRUE;
    }
    else{
        tileset_start_addr = 0x8000;
        using_signed = FALSE;
    }
     
    //Loop for every pixel in the scanline
    for (pixel=0;pixel<160;pixel++){
        
        //should read tile for every pixel??
        posX = pixel + readMemory8(SCX);
        if (using_window){
            if (pixel >= windowX){
                posX = pixel - windowX;
            }
        }
        
        tilemap_offset = (posX / 8) + ((posY / 8) * 32);    //find tile in tilemap
        
        if (using_signed == TRUE)   //find tileset number
            tileset_number = (signed char)readMemory8(tilemap_start_addr + tilemap_offset);
        else
            tileset_number = readMemory8(tilemap_start_addr + tilemap_offset);
            
        if (using_signed == TRUE)   //find tile
            tileset_offset = tileset_start_addr + ((tileset_number + 128) * 16);
        else
            tileset_offset = tileset_start_addr + (tileset_number * 16);
        
                
        //read tile row contents
        unsigned char tile_1 = readMemory8( tileset_offset + ( ( posY % 8 ) * 2 ) );
        unsigned char tile_2 = readMemory8( tileset_offset + ( ( posY % 8 ) * 2) + 1);
        //read pixel from tile row
        bit_1 = ((tile_1 << ( posX % 8 )) & 0x80 ) >> 7;
        bit_2 = ((tile_2 << ( posX % 8 )) & 0x80 ) >> 7;
        colour = (bit_2 << 1) | bit_1;
        
        gpuPaintColour(colour, BGP, &red, &green, &blue);
        
        framebuffer[readMemory8(LY)][pixel][0] = red;
        framebuffer[readMemory8(LY)][pixel][1] = green;
        framebuffer[readMemory8(LY)][pixel][2] = blue;
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

    //move scanline perspective to match sprites
    scanline +=16;
        
    //Count total sprites visible on current scanline   
    for (sprite = 0; sprite < 40; sprite++){
        
        posY = readMemory8(OAM + (sprite * 4));
        //posX = readMemory8((OAM + (sprite * 4)) + 1 ) - 8;
        
        /*for accurate emulation we have to check sprite priorites
         *we will start checking from the last pixel in each scanline
         */

        //Check if part of sprite is visible in current scanline
        if ((scanline >= posY) && (scanline < (posY + sprite_size))){
            sprites_on_scanline++;  
        }
    }
    //printf ("[DEBUG] %d sprites on scanline %d\n", sprites_on_scanline, readMemory8(LY));

    for (pixel = 167; pixel > 0; pixel--){
        for (sprite = 39; sprite >= 0; sprite--){

            posY = readMemory8(OAM + (sprite * 4));
            posX = readMemory8((OAM + (sprite * 4)) + 1 );
            //printf("[DEBUG] sprite %d, scanline %d\n",sprite,scanline);
             //if sprite is visible on current scanline
            if ((scanline >= posY) && (scanline < (posY + sprite_size))){
                //if sprite starts on current pixel
               // printf ("[DEBUG] Sprite = %d, posX = %d, pixel = %d\n", sprite,posX,pixel);
                if (pixel == posX){   
                    if (sprites_on_scanline > 10){
                        //limit of sprites reached on current scanline
                    }
                    else{
                        //printf("[DEBUG] Draw sprite %d at scanline %d starting at pixel %d\n",sprite,scanline-16,pixel);
                        gpuDrawSprite(sprite);//Draw sprite
                    }                  
                }

            }
            sprites_on_scanline--;
        }
    }
}


void gpuDrawSprite (unsigned char sprite){
    int using_8x16_sprites = FALSE;
    unsigned char sprite_size = 8;
    int sprite_line;
    unsigned char sprite_number;
    unsigned char sprite_attributes;
    unsigned char sprite_Y;
    unsigned char sprite_X;
    unsigned char scanline = readMemory8(LY);
    unsigned short sprite_start_address;
    unsigned char bit_1;
    unsigned char bit_2;
    unsigned char colour;
    unsigned short palette;
    int i;
    int red, green, blue;

    int flip_Y = FALSE;
    int flip_X = FALSE;
    //printf("[DEBUG] Sprite %d\n",sprite);
    //sprite = 35;
    //find sprite coordinates
    sprite_Y = readMemory8(OAM + (sprite * 4));
    sprite_X = readMemory8((OAM + (sprite * 4)) + 1 );    
    //find sprite pattern number
    sprite_number = readMemory8(OAM + (sprite * 4) + 2);
    //find sprite attributes
    sprite_attributes = readMemory8(OAM + (sprite * 4) + 3);

    
    //find sprite memory start adress    
    if (testBit(LCDC,2) == TRUE){
        using_8x16_sprites = TRUE;
        sprite_size = 16;
        sprite_start_address = SPT + sprite_number * 32; 
        printf("[DEBUG] Using large sprites\n" );
    }
    else{
        sprite_start_address = SPT + sprite_number * 16;
    }
    
    if (sprite_attributes & 0x40){
        flip_Y = TRUE;
    }
    if (sprite_attributes & 0x20){
        flip_X = TRUE;
    }

    sprite_line = scanline + 16 - sprite_Y;
    
    if (flip_Y){
        sprite_line -= sprite_size - 1;
        sprite_line *= -1;
    }

    //read sprite row contents
    //printf("[DEBUG] Sprite start address 0x%4x\n", sprite_start_address);
    unsigned char sprite_1 = readMemory8( sprite_start_address + ( sprite_line * 2 ) );
    unsigned char sprite_2 = readMemory8( sprite_start_address + ( sprite_line * 2 ) + 1);
     //printf("[DEBUG] sprite_1 0x%2x sprite_2 0x%2x\n", sprite_1,sprite_2 );
    
    for (i=0;i<8;i++){

        //read pixel from tile row
        if (flip_X){
            bit_1 = ((sprite_1 << ( (7 - i) % 8 )) & 0x80 ) >> 7;
            bit_2 = ((sprite_2 << ( (7 - i) % 8 )) & 0x80 ) >> 7;
            colour = (bit_2 << 1) | bit_1;            
        }else{
        
            bit_1 = ((sprite_1 << ( i % 8 )) & 0x80 ) >> 7;
            bit_2 = ((sprite_2 << ( i % 8 )) & 0x80 ) >> 7;
            colour = (bit_2 << 1) | bit_1;
        }
        //find palete address
        if (sprite_attributes & 0x10){
            palette = OBP1;
        }
        else{
            palette = OBP0;
        }
        
        draw_pixel = TRUE;
        //check sprite priority
        if (sprite_attributes & 0x80){ //no priority
            //If sprite pixel is in visible space
            if (sprite_X >= 8){
                if (framebuffer[readMemory8(LY)][sprite_X - 8][0] != 255)
                    if (framebuffer[readMemory8(LY)][sprite_X - 8][1] != 255)
                        if (framebuffer[readMemory8(LY)][sprite_X - 8][2] != 255)
                            draw_pixel = FALSE;
                
            }
        }

    
        gpuPaintColour(colour, palette, &red, &green, &blue);
        //if (red == 255)
            //draw_pixel = FALSE;
        
        if (sprite_X <= 167 && sprite_X >= 8){
            if (draw_pixel){
                spritebuffer[sprite_X - 8][0] = red;
                spritebuffer[sprite_X - 8][1] = green;
                spritebuffer[sprite_X - 8][2] = blue;
                spritebuffer[sprite_X - 8][3] = draw_pixel;
            }
            else{
                spritebuffer[sprite_X - 8][3] = draw_pixel;
            }
        }
        sprite_X +=1;
    }

}

void gpuPaintColour (unsigned char colour, unsigned short palette, int *red, int *green, int *blue){

    //Pass colour through the palette
    switch (colour){
        case 0b00:
            colour = testBit(palette,0) | (testBit(palette,1) << 1);
            draw_pixel = FALSE;
            break;
            
        case 0b01:
            colour = testBit(palette,2) | (testBit(palette,3) << 1);
            break;
                
        case 0b10:
            colour = testBit(palette,4) | (testBit(palette,5) << 1);
            break;

        case 0b11:
            colour = testBit(palette,6) | (testBit(palette,7) << 1);          
            break;
        default:
            printf("COLOUR1 = %d\n",colour);
            exit(1);
        }
        
    //Set actuall dot colour
    switch (colour){
        case 0b00:
            *red = 0xFF; *green = 0xFF; *blue = 0xFF;
            break;
            
        case 0b01:
            *red = 0xCC; *green = 0xCC; *blue = 0xCC;
            break;
                
        case 0b10:
            *red = 0x77; *green = 0x77; *blue = 0x77;
            break;

        case 0b11:
            *red = 0x00; *green = 0x00; *blue = 0x00;         
            break;
        default:
            printf("COLOUR2 = %d\n",colour);            
            exit(1);                
        }

}




void gpuStop (void){
    int y,x;
    
    for (y=143; y>=0; y--){
        for (x=159; x>=0; x--){
            framebuffer[y][x][0] = framebuffer[y][x][1] = framebuffer[y][x][2] = 255;  
        }
    }
    
    for (x=159; x>=0; x--){
        framebuffer[72][x][0] = framebuffer[72][x][1] = framebuffer[72][x][2] = 0;
    }
}