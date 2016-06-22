#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

void display (void);
int displayInit(void);
void displayEnd(void);
void setupTexture();
void updateTexture();
void fpsthink();

#ifdef __cplusplus
}
#endif

