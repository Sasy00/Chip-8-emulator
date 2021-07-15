#include "Chip8.h"
#include "stdio.h"
Chip8::Chip8() : video(this), cpu(this)
{
    
}

void Chip8::init()
{
    cpu.init();
    video.init();
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
    video.update();
}

void Chip8::deinit()
{
    video.closeSDL();
}
