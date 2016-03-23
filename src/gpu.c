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
 
struct sprite sprites[40];
struct sprite sprites_shorted[40];
int background_priority[160];
unsigned char framebuffer[144][160][3];
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
            gpu_state = H_BLANK;
            gpu_cycles = SCAN_OAM_CYCLES + gpuAdjustCycles(); //84
            gpuCompareLine();
        }
        // switch off
        else if ( (memory[LCDC] & 0x80) && !(value & 0x80) ){ // switch off
            if ( gpu_state == V_BLANK ){
                memory[LY] = 0;
                memory[STAT] &= 0xFC; //set to H_BLANK
                gpu_state = H_BLANK;
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
    //return;
    for (i=159;i>=0;i--){
        background_priority[i] = 0;
    }
    
    if (testBit(LCDC,0)){
        gpuRenderBackground();
    }

    if (testBit(LCDC,1)){
        gpuRenderSprites();
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
     
    unsigned char posX = memory[SCX];
    unsigned char posY = memory[SCY] + memory[LY];
    unsigned char windowX = memory[WX] - 7;
    unsigned char windowY = memory[WY];
 
    unsigned short tileset_start_addr;    //Tile Set start address in memory
    unsigned short tilemap_start_addr;    //Tile Map start address in memory
     
    unsigned short tileset_offset;
    unsigned short tilemap_offset;    

    //Initialize flags and memory addresses
    if (testBit(LCDC,5) == TRUE){         //Window Display Enable (0=Off, 1=On)
        if (memory[LY] >= memory[WY]){//Current scanline is after window position
            using_window = TRUE;
            posY = memory[LY] - windowY;
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
        posX = pixel + memory[SCX];
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
        
        background_priority[pixel] = colour == 0x00;
        
        framebuffer[memory[LY]][pixel][0] = red;
        framebuffer[memory[LY]][pixel][1] = green;
        framebuffer[memory[LY]][pixel][2] = blue;
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
     
     /* UPDATE SPRITE MEMORY */
     
     unsigned char i;
     unsigned char flags;
     
     for (i=0;i<40;i++){
         sprites[i].Ypos = readMemory8(OAM + (i * 4));
         sprites[i].Xpos = readMemory8(OAM + (i * 4) + 1);
         sprites[i].pattern = readMemory8(OAM + (i * 4) + 2);
         flags = readMemory8(OAM + (i * 4) + 3);
         sprites[i].priority = (flags & 0x80) >> 7;
         sprites[i].Yflip = (flags & 0x40) >> 6;
         sprites[i].Xflip = (flags & 0x20) >> 5;
         sprites[i].palette = (flags & 0x10) >> 4;
     }
     
     
    /* SHORT SPRITES */
      
    
    /* DRAW SPRITES */
    
    //move scanline perspective to match sprites
    int x;
    int scanline = memory[LY] + 16;
    int sprite_line;
    unsigned char sprite_size = 8;
    unsigned short sprite_start_address;
    unsigned char bit_1;
    unsigned char bit_2;
    unsigned char colour;
    unsigned short palette;
    int red, green, blue;
    
    if (testBit(LCDC,2) == TRUE){
        sprite_size = 16;
    }

   
    for (i=0;i<40;i++){
        if ((scanline >= sprites[i].Ypos) && (scanline < (sprites[i].Ypos + sprite_size))){
            
            if (sprites[i].palette){
                palette = OBP1;
            }
            else{
                palette = OBP0;
            }
            
            sprite_line = scanline - sprites[i].Ypos;
            if (sprites[i].Yflip){
                sprite_line -= sprite_size - 1;
                sprite_line *= -1;
            }
            
            if (testBit(LCDC,2) == TRUE){
                sprite_size = 16;
                sprite_start_address = SPT + (sprites[i].pattern & 0xFE)* 16; 
                //printf("[DEBUG] Using large sprites\n" );
            }
            else{
                sprite_start_address = SPT + sprites[i].pattern * 16;
            }


            unsigned char sprite_1 = readMemory8( sprite_start_address + ( sprite_line * 2 ) );
            unsigned char sprite_2 = readMemory8( sprite_start_address + ( sprite_line * 2 ) + 1);         
            
            for (x=0;x<8;x++){
                
                if (sprites[i].Xflip){
                    bit_1 = ((sprite_1 << ( (7 - x) % 8 )) & 0x80 ) >> 7;
                    bit_2 = ((sprite_2 << ( (7 - x) % 8 )) & 0x80 ) >> 7;
                    colour = (bit_2 << 1) | bit_1;            
                }else{
                    bit_1 = ((sprite_1 << ( x % 8 )) & 0x80 ) >> 7;
                    bit_2 = ((sprite_2 << ( x % 8 )) & 0x80 ) >> 7;
                    colour = (bit_2 << 1) | bit_1;
                }
                
                gpuPaintColour(colour, palette, &red, &green, &blue);
                
                
                if ((sprites[i].Xpos + x >= 8) && (sprites[i].Xpos + x <= 167)){
                    if (colour != 0x00){
                        if (!sprites[i].priority || background_priority[sprites[i].Xpos + x]){
                            framebuffer[memory[LY]][sprites[i].Xpos + x - 8][0] = red;
                            framebuffer[memory[LY]][sprites[i].Xpos + x - 8][1] = green;
                            framebuffer[memory[LY]][sprites[i].Xpos + x - 8][2] = blue;
                        }
                    }
                }
            }
        }
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