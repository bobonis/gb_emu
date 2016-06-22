#include <cstdio>
#include "Gb_Apu.h"
#include "Sound_Queue.h"
#include "Multi_Buffer.h"
#include <SDL2/SDL.h>
#include "sound.h"
#include "definitions.h"
    
Gb_Apu apu;
int TimeFrame = 0;
Stereo_Buffer buf;
blip_sample_t out_buf[4096];
Sound_Queue sound_render;

unsigned char read_memory_apu( unsigned short addr )
{
		return apu.read_register( TimeFrame, addr );
}

void write_memory_apu( unsigned short addr, unsigned char data )
{
		apu.write_register( TimeFrame, addr, data );
}

void sound_tick(int clockCycles){
    TimeFrame += clockCycles;
    if (TimeFrame >= FRAME_LENGTH)
    {
        end_frame();
        TimeFrame -= FRAME_LENGTH;
    }
}

void end_frame() {
	    apu.end_frame(TimeFrame);
        buf.end_frame(TimeFrame);
			
		if (buf.samples_avail() >= 4096) {	
		    
            long count = buf.read_samples(out_buf, 4096);
            sound_render.write(out_buf, count );
        }
}    


void init_apu() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
       printf("SDL audio failed to be initialized with error:%s",SDL_GetError()); 
    }
    atexit(SDL_Quit);
    buf.clock_rate(CLOCKSPEED); 
    buf.set_sample_rate(44100);
    apu.treble_eq(-15.0);
    buf.bass_freq(100);
    apu.set_output( buf.center(), buf.left(), buf.right() );
    sound_render.start(44100, 2); 
}