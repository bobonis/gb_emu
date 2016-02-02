#include "display.h"
#include "gpu.h"
#include "input.h"

// Window size
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
int modifier = 4;  

const char* WINDOW_TITLE = "GB Emu";    //Window Title

  
SDL_Window* window = NULL;         //The window we'll be rendering to
SDL_Surface* screenSurface = NULL; //The surface contained by the window 
SDL_Event event;                   //Event handler
SDL_GLContext context;


void display (void){
    
    //SDL_PollEvent( &event );
    
    //if( event.type == SDL_QUIT ) { 
    //    displayEnd(); 
    //}
                    
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    updateTexture();
    //setupTexture();
    //SDL_UpdateWindowSurface( window );
    SDL_GL_SwapWindow(window);
    //SDL_Delay(50);
}



int displayInit(void){
    
    //Initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) { 
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return 1; 
    }
    
    /* Request opengl 3.2 context.
     * SDL doesn't have the ability to choose which profile at this time of writing,
     * but it should default to the core profile */
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    /* Turn on double buffering with a 24bit Z buffer.
     * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    //Create window
    int width = SCREEN_WIDTH * modifier;
    int height = SCREEN_HEIGHT * modifier;
    window = SDL_CreateWindow( WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL ); 
    if ( window == NULL ) { 
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        return 1;
    }
    
    /* Create our opengl context and attach it to our window */
    context = SDL_GL_CreateContext(window);
    if ( context == NULL ) { 
        printf( "Context could not be created! SDL_Error: %s\n", SDL_GetError() );
        return 1;
    }
    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    if (SDL_GL_SetSwapInterval(0) == 0)
        printf("[DEBUG] Vsync Disabled\n");
    else
        printf("[DEBUG] Error with Vsync.SDL_Error: %s\n", SDL_GetError() );  

    setupTexture(); // init Textures

    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    glViewport(0, 0, dims[2], dims[3]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, width, height, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    
    return 0;
}

void displayEnd(void){
     
    SDL_DestroyWindow( window );   //Destroy window
    SDL_Quit();                    //Quit SDL subsystems
    exit(1); 
}



void setupTexture()
{
    int y,x;
	// Clear screen
	for(y = 0; y < SCREEN_HEIGHT; ++y)		
		for(x = 0; x < SCREEN_WIDTH; ++x)
			framebuffer[y][x][0] = framebuffer[y][x][1] = framebuffer[y][x][2] = 100;

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