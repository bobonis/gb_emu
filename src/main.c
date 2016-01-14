#include <stdio.h>
#include <SDL2/SDL.h>
#include "memory.h"
#include "cpu.h"
#include "rom.h"
#include "gpu.h"
#include "timers.h"
#include "interrupts.h"



int main(int argc, char **argv){

	if (argc < 2){
		printf("No Rom\n");
		return 1;
	}
	
	if (loadRom(argv[1])){
		return 1;
	}
	
	reset();
	printf("[INFO] System reset done\n");
    
    SDL_Window *window;                    // Declare a pointer
    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2
    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        160,                               // width, in pixels
        144,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );

    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

	
    int cycles;
    
	while (1) {
		cycles = execute();
        updateTimers(cycles);
		gpu(cycles);
		//input();
		handleInterrupts();
        //SDL_Delay(30);  // Pause execution for 3000 milliseconds, for example
	}
	
	
return 0;
}





