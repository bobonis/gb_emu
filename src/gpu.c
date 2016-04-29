#include <stdio.h>
#include "gpu.h"
#include "memory.h"
#include "interrupts.h"
#include "display.h"
#include "cpu.h"
#include "definitions.h"

sprite sprites[40];

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
                gpuDrawScanline();
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
                }
            }
            break;
            
        case H_BLANK:   /*mode 0 */
            if (gpustate.clock <= 4 && gpustate.clock > 0){
                if (gpustate.firstframe == FALSE){
                    //gpuDrawScanline();
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
                    if (testBit(STAT,4) == TRUE){
                        triggerInterrupt(LCDC_INTERRUPT);   /* STAT VBLANK INTERRUPT */
                    } 
                    else if (testBit(STAT,5) == TRUE){
                        triggerInterrupt(LCDC_INTERRUPT);   /* OAM INTERRUPT */
                    }
                    else{
                        gpuCompareLine();                       /* LYC INTERRUPT */
                    }
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
                        }
                        else if (gpustate.line != 0){
                            gpuCompareLine();                       /* LYC INTERRUPT */
                        }
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
            }
            else if (gpustate.clock <= 0){
                
                gpuCompareLine();                       /* LYC INTERRUPT */
                
                if (gpustate.line == 153){
                    gpustate.line = 0;
                    memory[LY] = gpustate.line;
                    gpustate.clock = V_BLANK_CYCLES;
                    if (testBit(STAT,5) == TRUE){
                        //triggerInterrupt(LCDC_INTERRUPT);   /* OAM INTERRUPT */
                    }
                }
                else if (gpustate.line == 0){
                    gpustate.mode = SCAN_OAM;
                    memory[STAT] &= 0xFC;
                    memory[STAT] |= SCAN_OAM;
                    gpustate.clock = SCAN_OAM_CYCLES;
                    if (testBit(STAT,5) == TRUE){
                        triggerInterrupt(LCDC_INTERRUPT);   /* OAM INTERRUPT */
                    }
                    
                    display();
                }
                else{
                    gpustate.clock = V_BLANK_CYCLES;
                    if (testBit(STAT,5) == TRUE){
                        //triggerInterrupt(LCDC_INTERRUPT);   /* OAM INTERRUPT */
                        //printf("[GPU] OAM INT\n");
                    }
                }
            }
            break;
        default:
            break;
    }
}

void gpuUpdateSprites(void){
    
    unsigned int i;
    unsigned char flags;
    
    for (i=0;i<40;i++){
         sprites[i].number = i;
         sprites[i].draw = FALSE;
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
    unsigned int scanline = gpustate.line + 16;
    unsigned char sprite_size = 8;
    if (!testBit(LCDC,1)){
        return 0;
    }

    if (testBit(LCDC,2) == TRUE){
        sprite_size = 16;
    }
    
    for (i=0;i<40;i++){
        if ((scanline >= sprites[i].Ypos) && (scanline < (sprites[i].Ypos + sprite_size))){
            
            //if (sprites[i].Xpos > 16){
                sprites[i].draw = TRUE;
                spritesline += 1;
            //}
            
            
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
                default:
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
            //printf("%d\n",spritesline);
            return 64;
            break;
    }
}


void gpuCompareLine (void){
    
    if (memory[LY] == memory[LYC]){
        setBit(STAT,2,TRUE);
        if (testBit(STAT,6) == TRUE){
            triggerInterrupt(LCDC_INTERRUPT);
        }
    }
    else{
        setBit(STAT,2,FALSE);
    }
}

void gpuSetStatus(unsigned char value){
        // switch on
        if ( !(memory[LCDC] & 0x80) && (value & 0x80) ){
            //printf("[DEBUG] LCD turned on\n");
            memory[LY] = 0;
            gpuCompareLine();
            
            gpustate.enable = TRUE;
            gpustate.firstframe = TRUE;
            gpustate.clock = SCAN_OAM_CYCLES;            
        }
        // switch off
        else if ( (memory[LCDC] & 0x80) && !(value & 0x80) ){ // switch off
            if ( gpustate.mode == V_BLANK ){
                //printf("[DEBUG] LCD turned off\n");
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
    unsigned char posY = memory[SCY] + gpustate.line;
    unsigned char windowX = memory[WX] - 7;
    unsigned char windowY = memory[WY];
 
    unsigned short tileset_start_addr;    //Tile Set start address in memory
    unsigned short tilemap_start_addr;    //Tile Map start address in memory
    unsigned short tilemap_start_addr_window;    //Tile Map start address in memory
    unsigned short tilemap_start_addr_background;    //Tile Map start address in memory


     
    unsigned short tileset_offset;
    unsigned short tilemap_offset;    

    //Initialize flags and memory addresses
                    
    if (testBit(LCDC,5) == TRUE){         //Window Display Enable (0=Off, 1=On)
        if (gpustate.line >= memory[WY]){ //Current scanline is after window position
            using_window = TRUE;
            posY = gpustate.line - windowY;
        }
    }

    if (using_window == TRUE){
        if (testBit(LCDC,6) == FALSE){     //BG Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
            tilemap_start_addr_window = 0x9800;
            // using_signed = TRUE;            //tilemap number is signed number
       }
       else{
            tilemap_start_addr_window = 0x9C00;
       }
    }

    if (testBit(LCDC,3) == FALSE){     //BG Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
        tilemap_start_addr_background = 0x9800;
        // using_signed = TRUE;            //tilemap number is signed number
       }
    else{
        tilemap_start_addr_background = 0x9C00;
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
                tilemap_start_addr = tilemap_start_addr_window;
            }
            else{
                tilemap_start_addr = tilemap_start_addr_background;
            }
        }
        else{
            tilemap_start_addr = tilemap_start_addr_background;
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
     
    /* SHORT SPRITES */

    /* sprites with lower X coordinate are drawn last */
    /* if X coordinates match, sprites with lower OAM index are drawn last */ 
    int i,j;
    int found = 255;
    int start = 168;
    int end = 0;
    int counter = 0;

    sprite sprites_shorted[MAXSPRITES];
    
    for (i=0;i<MAXSPRITES;i++){                     /* For the maximum number of displayed sprites */
        for (j=0;j<40;j++){                         /* Loop for every sprite */
            if (sprites[j].draw){                   /* If this is the first time for this sprite */
                if (sprites[j].Xpos < start && sprites[j].Xpos > end){
                    found = j;                      /* We found a sprite */
                    start = sprites[j].Xpos;        /* Look if there is another with higher X coordinate */
                }
            }
        }

        if (found != 255){                          /* We found a sprite to draw */
            sprites_shorted[counter] = sprites[found];
            counter += 1;                           /* Count how many sprites we have found so far */
            sprites[found].draw = FALSE;            /* Set it to FALSE so we don't pick the same sprite again */
            end = sprites[found].Xpos;              /* On next iter look for sprites with lower X coordinate */
            found = 255;
            start = 168;                              /* Start from lower X coordinate again */
        }
        else{
            start = 168;
            end = 0;
        }
    }


    /* DRAW SPRITES */


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

   
    for (i=counter-1;i>=0;i--){
            
            if (sprites_shorted[i].palette){
                palette = OBP1;
            }
            else{
                palette = OBP0;
            }
            
            sprite_line = scanline - sprites_shorted[i].Ypos;
            if (sprites_shorted[i].Yflip){
                sprite_line -= sprite_size - 1;
                sprite_line *= -1;
            }
            
            if (testBit(LCDC,2) == TRUE){
                sprite_size = 16;
                sprite_start_address = SPT + (sprites_shorted[i].pattern & 0xFE)* 16; 
                //printf("[DEBUG] Using large sprites\n" );
            }
            else{
                sprite_start_address = SPT + sprites_shorted[i].pattern * 16;
            }


            unsigned char sprite_1 = memory[ sprite_start_address + ( sprite_line * 2 ) ];
            unsigned char sprite_2 = memory[ sprite_start_address + ( sprite_line * 2 ) + 1];         
            
            for (x=0;x<8;x++){
                
                if (sprites_shorted[i].Xflip){
                    bit_1 = ((sprite_1 << ( (7 - x) % 8 )) & 0x80 ) >> 7;
                    bit_2 = ((sprite_2 << ( (7 - x) % 8 )) & 0x80 ) >> 7;
                    colour = (bit_2 << 1) | bit_1;            
                }else{
                    bit_1 = ((sprite_1 << ( x % 8 )) & 0x80 ) >> 7;
                    bit_2 = ((sprite_2 << ( x % 8 )) & 0x80 ) >> 7;
                    colour = (bit_2 << 1) | bit_1;
                }
                
                gpuPaintColour(colour, palette, &red, &green, &blue);
                
                if ((sprites_shorted[i].Xpos + x >= 8) && (sprites_shorted[i].Xpos + x <= 167)){
                    if (draw_pixel){
                        if (!sprites_shorted[i].priority || background_priority[sprites_shorted[i].Xpos + x - 8]){
                            framebuffer[memory[LY]][sprites_shorted[i].Xpos + x - 8][0] = red;
                            framebuffer[memory[LY]][sprites_shorted[i].Xpos + x - 8][1] = green;
                            framebuffer[memory[LY]][sprites_shorted[i].Xpos + x - 8][2] = blue;
                        }
                    }
                }
            }
        }
 }

void gpuPaintColour (unsigned char colour, unsigned short palette, int *red, int *green, int *blue){
    draw_pixel = TRUE;
    /* Pass colour through the palette */
    switch (colour){
        case 0b00:
            //colour = testBit(palette,0) | (testBit(palette,1) << 1);
            colour = (memory[palette] & 0x03);
            draw_pixel = FALSE;
            break;
            
        case 0b01:
            //colour = testBit(palette,2) | (testBit(palette,3) << 1);
            colour = (memory[palette] & 0x0C) >> 2;
            break;
                
        case 0b10:
            //colour = testBit(palette,4) | (testBit(palette,5) << 1);
            colour = (memory[palette] & 0x30) >> 4;
            break;

        case 0b11:
            //colour = testBit(palette,6) | (testBit(palette,7) << 1);
            colour = (memory[palette] & 0xC0) >> 6;      
            break;
        default:
            printf("COLOUR1 = %d\n",colour);
            exit(1);
        }
        
    /* Set actuall pixel colour */
    switch (colour){
        case 0b00:
            *red = 0xEF; *green = 0xFF; *blue = 0xDE; /* White */
            break;
            
        case 0b01:
            *red = 0xAD; *green = 0xDF; *blue = 0x94; /* Light Grey */
            break;
                
        case 0b10:
            *red = 0x52; *green = 0x92; *blue = 0x73; /* Dark Grey */
            break;

        case 0b11:
            *red = 0x18; *green = 0x34; *blue = 0x42; /* Black */
            break;
        default:
            printf("COLOUR2 = %d\n",colour);            
            exit(1);                
        }
}

void gpuStop (void){
    
    unsigned int y,x;
    
    for (y=0; y<=143; y++){
        for (x=0; x<=159; x++){
            framebuffer[y][x][0] = framebuffer[y][x][1] = framebuffer[y][x][2] = 255;  
        }
    }
    
    for (x=0; x<=159; x++){
        framebuffer[72][x][0] = framebuffer[72][x][1] = framebuffer[72][x][2] = 0;
    }
}