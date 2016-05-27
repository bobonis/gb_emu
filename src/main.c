#include <stdio.h>
#include "memory.h"
#include "cpu.h"
#include "rom.h"
#include "gpu.h"
#include "timers.h"
#include "interrupts.h"
#include "display.h"
#include "input.h"
#include "definitions.h"

#define version "0.86.6"


int main(int argc, char **argv){

    SDL_Event event;                   //Event handler

    int QUIT = FALSE;
    
	if (argc < 2){
		printf("No Rom\n");
		return 1;
	}
	
	if (loadRom(argv[1])){
		return 1;
	}
    
    if (displayInit()){
        return 1;
    }
    
	
	reset();
    printf("[INFO] Version %s\n",version);
	printf("[INFO] System reset done\n");
    printf("[INFO] BUTTON A  = a\n");
    printf("[INFO] BUTTON B  = s\n");
    printf("[INFO] START     = q\n");
    printf("[INFO] SELECT    = w\n");
    printf("[INFO] DIRECTION = arrow keys\n");
    //display();
    
    int count = 0;
	
    while (!QUIT) {
    
        count++;    
        
        if (count > 100){                   /* Poll for events every 100 loops */  
            while (SDL_PollEvent(&event)) {
                if( event.type == SDL_QUIT ) { 
                    QUIT = TRUE;
                }
        
                if (event.type == SDL_KEYDOWN){
                    if (event.key.keysym.sym == SDLK_ESCAPE){
                        QUIT = TRUE;
                    }
                    if (event.key.keysym.sym == SDLK_F1){
                        displayEnd();
                        return 1;
                    }
                }
                inputHandleEvents(event);
            }
        count = 0;
        }       
              
		execute();
		//inputHandleEvents(event);
	}

    updateMBC2SRAM();   
    displayEnd();
	
return 0;
}