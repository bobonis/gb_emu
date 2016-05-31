#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

extern unsigned char joypad;


unsigned char inputReadKeys();
void inputPressKey(int key);
void inputReleaseKey(int key);
void inputHandleEvents(SDL_Event event);
void inputCheckInterrupt(void); 

#ifdef __cplusplus
}
#endif