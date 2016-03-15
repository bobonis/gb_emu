#include <stdio.h>
#include "memory.h"
#include "cpu.h"
#include "rom.h"
#include "gpu.h"
#include "timers.h"
#include "interrupts.h"
#include "display.h"
#include "input.h"


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
	printf("[INFO] System reset done\n");
    printf("[INFO] BUTTON A  = a\n");
    printf("[INFO] BUTTON B  = s\n");
    printf("[INFO] START     = q\n");
    printf("[INFO] SELECT    = w\n");
    printf("[INFO] DIRECTION = arrow keys\n");
    //display();
    
	while (!QUIT) {
        
        SDL_PollEvent( &event );
        if( event.type == SDL_QUIT ) { 
            QUIT = TRUE;
        }
        
        if (event.type == SDL_KEYDOWN){
            if (event.key.keysym.sym == SDLK_ESCAPE){
                QUIT = TRUE;
            }
        }       
              
		execute();
		inputHandleEvents(event);
	}

    displayEnd();
	
return 0;
}
