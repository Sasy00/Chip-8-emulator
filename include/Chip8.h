#ifndef CHIP_8
#define CHIP_8

#include "Video.h"
#include "Cpu.h"

class Chip8
{
public:
    Chip8();
    inline void cls(){ video.cls(); };
    inline uint8_t drawSprite(uint8_t x, uint8_t y, uint8_t *sprite, uint8_t rows)
    {
        return video.drawSprite(x, y, sprite, rows);
    }
    void init();
    int loadRom(char *filename);
    void update();
    void deinit();

private:
    Video video;
    Cpu cpu;
};

#endif