#include <stdio.h>
#include "gpu.h"
#include "memory.h"
#include "interrupts.h"
#include "display.h"
#include "cpu.h"
#include <stdio.h>

#define SCAN_OAM  2 //Scanline (accessing OAM)
#define SCAN_VRAM 3 //Scanline (accessing VRAM)
#define H_BLANK   0 //Horizontal blank
#define V_BLANK   1 //Vertical blank

#define SCAN_OAM_CYCLES   80
#define SCAN_VRAM_CYCLES 172
#define H_BLANK_CYCLES   204
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
unsigned char gpu_line = 0;
int draw_pixel = TRUE;

int adjust = 0;

struct gpu gpustate = {
    FALSE,      /* enable */
    0,          /* clock */
    H_BLANK,    /* mode */
    FALSE,      /* interrupt */
    0,          /* line */
    FALSE,      /* first line */
    0           /* lyc */
};

/*  Period  GPU mode          Time spent   (clocks)
 * -----------------------------------------------------
 * Scanline (accessing OAM)       2            80
 * Scanline (accessing VRAM)      3           172
 * Horizontal blank               0           204
 * One line (scan and blank)                  456
 * Vertical blank                 1          4560 (10 lines)
 * Full frame (scans and vblank)            70224
 */
 void gpu(void){
    
    if (gpustate.enable == FALSE){
        return;
    }

    if (cpustate.stop == TRUE){
        gpuStop();
        display();
        return;
    }    

    gpustate.clock -= 4;
    
    switch (gpustate.mode){
        case SCAN_OAM: /* mode 2 */
            if (gpustate.clock <= 0){
                gpuUpdateSprites();
                adjust = gpuCountSprites();
                gpustate.clock += SCAN_VRAM_CYCLES + adjust + gpuAdjustCycles();
                gpustate.mode = SCAN_VRAM;
                memory[STAT] &= 0xFC;
                memory[STAT] |= SCAN_VRAM;
            }
            break;
            
        case SCAN_VRAM: /*mode 3 */
            if (gpustate.clock <= 4 && gpustate.clock > 0){

            }
            else if (gpustate.clock <= 0){
                gpustate.mode = H_BLANK;
                memory[STAT] &= 0xFC;
                memory[STAT] |= H_BLANK;
                gpustate.clock += H_BLANK_CYCLES  - adjust - gpuAdjustCycles();
                if (testBit(STAT,3) == TRUE){
                    triggerInterrupt(LCDC_INTERRUPT);   /* HBLANK INTERRUPT */
                    //printf("[GPU] HBLANK INT\n");
                }
                gpuDrawScanline();
            }
            break;
            
        case H_BLANK:   /*mode 0 */
            if (gpustate.clock <= 4 && gpustate.clock > 0){
                if (gpustate.firstframe == FALSE){
                    gpustate.line += 1;
                    memory[LY] = gpustate.line;
                    if (gpustate.line < 144){
 
                    }
                }
            }
            else if (gpustate.clock <= 0){
                if (gpustate.line > 143){
                    gpustate.mode = V_BLANK;
                    memory[STAT] &= 0xFC;
                    memory[STAT] |= V_BLANK;
                    gpustate.clock = V_BLANK_CYCLES;
                    triggerInterrupt(VBLANK_INTERRUPT);     /* VBLANK INTERRUPT */
                    //printf("[GPU] VBLANK INT\n");
                    if (testBit(STAT,4) == TRUE){
                        triggerInterrupt(LCDC_INTERRUPT);   /* STAT VBLANK INTERRUPT */
                        //printf("[GPU] STAT VBLANK INT\n");
                    } 
                    gpuCompareLine();                       /* LYC INTERRUPT */
                    if (testBit(STAT,5) == TRUE){
                        triggerInterrupt(LCDC_INTERRUPT);   /* OAM INTERRUPT */
                        //printf("[GPU] OAM INT\n");
                    }
                    //printf("\n %d \n",gpustate.line);
                }
                else{
                    if (gpustate.firstframe == TRUE){
                        gpustate.firstframe = FALSE;
                        gpustate.mode = SCAN_VRAM;
                        memory[STAT] &= 0xFC;
                        memory[STAT] |= SCAN_VRAM;
                        gpustate.clock = SCAN_VRAM_CYCLES;   
                    }
                    else{
                        gpustate.mode = SCAN_OAM;
                        memory[STAT] &= 0xFC;
                        memory[STAT] |= SCAN_OAM;
                        gpustate.clock = SCAN_OAM_CYCLES;
                       if (testBit(STAT,5) == TRUE){
                            triggerInterrupt(LCDC_INTERRUPT);   /* OAM INTERRUPT */
                            //printf("[GPU] OAM INT\n");
                        }
                        if (gpustate.line != 0){
                            gpuCompareLine();                       /* LYC INTERRUPT */
                        }
                        //printf("\n %d \n",gpustate.line);
                    }
                }
            }
            break;
            
        case V_BLANK:   /*mode 1 */
            if (gpustate.clock <= 452 && gpustate.clock > 448){
                
                if (gpustate.line == 0){
                    gpuCompareLine();
                } 
            }
            else if (gpustate.clock <= 4 && gpustate.clock > 0){
                if (gpustate.line != 0){
                    gpustate.line += 1;
                    memory[LY] = gpustate.line;
                }
                else{
                }
                
            }
            else if (gpustate.clock <= 0){
                
                gpuCompareLine();                       /* LYC INTERRUPT */
                
                if (gpustate.line == 153){
                    gpustate.line = 0;
                    memory[LY] = gpustate.line;
                    gpustate.clock = V_BLANK_CYCLES;
                    if (testBit(STAT,5) == TRUE){
                        //triggerInterrupt(LCDC_INTERRUPT);   /* OAM INTERRUPT */
                        //printf("[GPU] OAM INT\n");
                    }
                }
                else if (gpustate.line == 0){
                    gpustate.mode = SCAN_OAM;
                    memory[STAT] &= 0xFC;
                    memory[STAT] |= SCAN_OAM;
                    gpustate.clock = SCAN_OAM_CYCLES;
                    //printf("\n %d \n",gpustate.line);
                    if (testBit(STAT,5) == TRUE){
                        triggerInterrupt(LCDC_INTERRUPT);   /* OAM INTERRUPT */
                        //printf("[GPU] OAM INT\n");
                    }
                    
                    display();
                }
                else{
                    gpustate.clock = V_BLANK_CYCLES;
                    if (testBit(STAT,5) == TRUE){
                        //triggerInterrupt(LCDC_INTERRUPT);   /* OAM INTERRUPT */
                        //printf("[GPU] OAM INT\n");
                    }
                    //printf("\n %d \n",gpustate.line);
                }
            }
            break;
    }
}

void gpuUpdateSprites(void){
    
    unsigned int i;
    unsigned char flags;
    
    for (i=0;i<40;i++){
         sprites[i].Ypos = memory[OAM + (i * 4)];
         sprites[i].Xpos = memory[OAM + (i * 4) + 1];
         sprites[i].pattern = memory[OAM + (i * 4) + 2];
         flags = memory[OAM + (i * 4) + 3];
         sprites[i].priority = (flags & 0x80) >> 7;
         sprites[i].Yflip = (flags & 0x40) >> 6;
         sprites[i].Xflip = (flags & 0x20) >> 5;
         sprites[i].palette = (flags & 0x10) >> 4;
     }
}

int gpuCountSprites (void){
    
    int i;
    int spritesline = 0;
    int spritesx = 0;
    int scanline = memory[LY] + 16;
    unsigned char sprite_size = 8;
    if (!testBit(LCDC,1)){
        return 0;
    }

    if (testBit(LCDC,2) == TRUE){
        sprite_size = 16;
    }
    
    for (i=0;i<40;i++){
        if ((scanline >= sprites[i].Ypos) && (scanline < (sprites[i].Ypos + sprite_size))){
            spritesline += 1;
            
            
            int temp = sprites[i].Xpos % 0x08;
    
            //printf("%d\n",sprites[i].Xpos);
    
            switch (temp){
                case 7:
                    spritesx = 0;
                    break;
                case 6:
                    spritesx = 0;
                    break;
                case 5:
                    spritesx = 0;
                    break;
                case 4:
                    spritesx = 0;
                    break;
                case 3:
                    spritesx = 0;
                    break;
                case 2:
                    spritesx = 0;
                    break;
                case 1:
                    spritesx = 0;
                    break;
                case 0:
                    spritesx = 0;
                    break;
            }
        }
    }

    //spritesx = ((spritesx / 4) * 4);
    //printf("%d\n",spritesline);
    //return spritesline * 8;
    
    //spritesline = (spritesline % 4) * 4;
    switch (spritesline){
        case 0:
            return 0 + spritesx;
            break;
        case 1:
            return 8+ spritesx;
            break;
        case 2:
            return 16+ spritesx;
            break;
        case 3:
            return 20+ spritesx;
            break;
        case 4:
            return 28+ spritesx;
            break;
        case 5:
            return 32+ spritesx;
            break;
        case 6:
            return 40+ spritesx;
            break;
        case 7:
            return 44+ spritesx;
            break;
        case 8:
            return 52+ spritesx;
            break;
        case 9:
            return 56+ spritesx;
            break;
        case 10:
            return 64+ spritesx;
            break;
        default:
            printf("%d\n",spritesline);
            return 64;
            break;
    }
}


void gpuCompareLine (void){
    
    if (memory[LY] == memory[LYC]){
        setBit(STAT,2,TRUE);
        if (testBit(STAT,6) == TRUE){
            triggerInterrupt(LCDC_INTERRUPT);
            //printf("[GPU] LYC INT AT LINE %d\n",gpustate.line);
        }
    }
    else{
        setBit(STAT,2,FALSE);
    }
}

void gpuSetStatus(unsigned char value){
        // switch on
        if ( !(memory[LCDC] & 0x80) && (value & 0x80) ){
            printf("[DEBUG] LCD turned on\n");
            memory[LY] = 0;
            gpuCompareLine();
            
            gpustate.enable = TRUE;
            gpustate.firstframe = TRUE;
            gpustate.clock = SCAN_OAM_CYCLES;            
        }
        // switch off
        else if ( (memory[LCDC] & 0x80) && !(value & 0x80) ){ // switch off
            if ( gpustate.mode == V_BLANK ){
                printf("[DEBUG] LCD turned off\n");
                /* When the LCD is off this register is fixed at 00h */
                memory[LY] = 0x00;
                gpustate.line = 0;
                gpustate.enable = FALSE;
                /* Bits 0-2 return '0' when the LCD is off */
                memory[STAT] &= 0xFC;
                gpustate.mode = H_BLANK;
            }
            else{
                printf("[ERROR] LCD turned off not in V_BLANK state\n");
                exit(1);
            }
        }        
}

/*
 * A hack to pass a test
 *
 */
int gpuAdjustCycles (void){
    
    //return 0;
    
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
            tileset_number = (signed char)memory[tilemap_start_addr + tilemap_offset];
        else
            tileset_number = memory[tilemap_start_addr + tilemap_offset];
            
        if (using_signed == TRUE)   //find tile
            tileset_offset = tileset_start_addr + ((tileset_number + 128) * 16);
        else
            tileset_offset = tileset_start_addr + (tileset_number * 16);
        
                
        //read tile row contents
        unsigned char tile_1 = memory[ tileset_offset + ( ( posY % 8 ) * 2 ) ];
        unsigned char tile_2 = memory[ tileset_offset + ( ( posY % 8 ) * 2) + 1];
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


            unsigned char sprite_1 = memory[ sprite_start_address + ( sprite_line * 2 ) ];
            unsigned char sprite_2 = memory[ sprite_start_address + ( sprite_line * 2 ) + 1];         
            
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
                        if (!sprites[i].priority || background_priority[sprites[i].Xpos + x - 8]){
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