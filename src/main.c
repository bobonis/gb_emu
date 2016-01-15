#include <stdio.h>
#include "memory.h"
#include "cpu.h"
#include "rom.h"
#include "gpu.h"
#include "timers.h"
#include "interrupts.h"
#include "display.h"

int main(int argc, char **argv){

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

    int cycles;
    
	while (1) {
		cycles = execute();
        updateTimers(cycles);
		gpu(cycles);
		//input();
		handleInterrupts();
        //display();
	}
	
	
return 0;
}
