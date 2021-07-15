#ifndef CPU_H
#define CPU_H

#define RAM_SIZE 4096

#include <inttypes.h>

class Cpu
{
public:
    Cpu(class Chip8 *back);
    void init();
    void loadRom(uint8_t *rom, int romsize);
    void execute();

private:
    uint8_t v[16];          //v[0x0] to v[0xF] 
    uint16_t pc;            //program counter
    uint16_t i;
    
    uint8_t dt;             //delay timer
    uint8_t st;             //sound timer
    
    uint8_t sp;             //stack pointer
    uint16_t stack[16];     //stack, used only for storing the pc before a call
    uint8_t ram[RAM_SIZE];

    class Chip8 *chip;
};

#endif