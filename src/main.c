#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "memory.h"
#include "cpu.h"
#include "rom.h"
#include "gpu.h"
#include "timers.h"
#include "interrupts.h"

// Window size
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
int modifier = 1;  

void setupTexture();
void updateTexture();
//void display(GLFWwindow* window);
void initGL(SDL_Window* window);

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

//********************************
    
    
    
    SDL_Window *window;                    // Declare a pointer
    SDL_Renderer* renderer;

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2
    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "gb_emu",                           // window title
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
    initGL(window);
    renderer = SDL_CreateRenderer(window, -1, 0);
	
    int cycles;
    
	while (1) {
		cycles = execute();
        updateTimers(cycles);
		gpu(cycles);
		//input();
		handleInterrupts();
        		
        //glClear(GL_COLOR_BUFFER_BIT);
		
		// Swap buffers!
        

        // Select the color for drawing. It is set to red here.
        //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

        // Clear the entire screen to our selected color.
        SDL_RenderClear(renderer);
        updateTexture();
        // Up until now everything was drawn behind the scenes.
        // This will show the new, red contents of the window.
        SDL_RenderPresent(renderer);

        //SDL_Delay(30);  // Pause execution for 3000 milliseconds, for example
	}
	
	
return 0;
}

void initGL(SDL_Window* window)
{
    //glfwMakeContextCurrent(window);
    SDL_GLContext SDL_GL_CreateContext(SDL_Window* window);
    setupTexture(); // init Textures
    //int width, height;
    //glfwGetFramebufferSize(window, &width, &height);
    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    GLint width = dims[2];
    GLint height = dims[3];
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, width, height, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
}


void setupTexture()
{
    int y,x;
	// Clear screen
	for(y = 0; y < SCREEN_HEIGHT; ++y)		
		for(x = 0; x < SCREEN_WIDTH; ++x)
			framebuffer[y][x][0] = framebuffer[y][x][1] = framebuffer[y][x][2] = 0;

	// Create a texture 
	glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)framebuffer);

	// Set up the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 

	// Enable textures
	glEnable(GL_TEXTURE_2D);
}

void updateTexture()
{	
	// Update Texture
    int display_width = SCREEN_WIDTH * modifier;
    int display_height = SCREEN_HEIGHT * modifier;  
	glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)framebuffer);

	glBegin( GL_QUADS );
		glTexCoord2d(0.0, 0.0);		glVertex2d(0.0,			  0.0);
		glTexCoord2d(1.0, 0.0); 	glVertex2d(display_width, 0.0);
		glTexCoord2d(1.0, 1.0); 	glVertex2d(display_width, display_height);
		glTexCoord2d(0.0, 1.0); 	glVertex2d(0.0,			  display_height);
	glEnd();
}
