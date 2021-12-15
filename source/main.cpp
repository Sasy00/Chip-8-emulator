/*
    0x000-0x1FF emulator only
    0xF00-0xFFF display
    0xEFF-0XEA0 call stack, internal use other v
*/
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include "Chip8.h"

#define FREQUENCY 500

int main(int argc, char** argv)
{  
    Chip8 chip8;
    
    if(argc != 2)
    {
        printf("ERROR Usage: %s <rom path>\n", argv[0]);
        return 1;
    }
    if(chip8.loadRom(argv[1]))
    {
        printf("ERROR: File %s not found\n", argv[1]);
        return 1;
    }
    
    srand(time(0));
    
    int end = 0;
    SDL_Event e;
    
    int frameDelay = 1000 / FREQUENCY;
    uint32_t frameStart;
    int frameTime;
    
    chip8.init();

    while(!end)
    {
        frameStart = SDL_GetTicks();
        
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                end = 1;
            }
        }
        
        chip8.update();
        
        frameTime = SDL_GetTicks() - frameStart;
        if(frameDelay > frameTime)
        {
            SDL_Delay(frameDelay - frameTime);
        }
    }
    chip8.deinit();
    return 0;
}