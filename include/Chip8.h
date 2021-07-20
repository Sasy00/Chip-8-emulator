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
    inline void setDT(uint8_t x) {dt = x;}
    inline void setST(uint8_t x) {st = x;}
    inline uint8_t getDT() const {return dt;}
    inline uint8_t getST() const {return st;}

private:
    Video video;
    Cpu cpu;
    uint8_t dt;             //delay timer
    uint8_t st;             //sound timer
    uint32_t clock;
};

#endif

/*if*/