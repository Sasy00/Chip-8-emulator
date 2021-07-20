#include "Chip8.h"
#include "stdio.h"
Chip8::Chip8() : video(this), cpu(this)
{
    
}

void Chip8::init()
{
    clock = SDL_GetTicks();
    cpu.init();
    video.init();
    dt = 0;
    st = 0;
}

int Chip8::loadRom(char *filename)
{
    FILE *fr;
    int fsize;
    if(!(fr = fopen(filename,"r")))
    {
        return 1;
    }
    fseek(fr, 0, SEEK_END);
    fsize = ftell(fr);
    fseek(fr, 0, SEEK_SET);

    uint8_t *buffer = new uint8_t[fsize];

    fread(buffer, 1, fsize, fr);   
    fclose(fr);

    cpu.loadRom(buffer, fsize);

    return 0;
}

void Chip8::update()
{
    cpu.execute();
    //video.update();
    uint16_t now = SDL_GetTicks();
    if(now - clock >= 1000/60)
    {
        if(dt > 0)
            dt--;
        if(st > 0)
            st--;
        clock = now;
    }
    
}

void Chip8::deinit()
{
    video.deinit();
}
