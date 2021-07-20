#include "Cpu.h"
#include "Chip8.h"
Cpu::Cpu(Chip8 *a) : chip(a)
{
}

void Cpu::init()
{
    pc = 0x200;
    sp = 0;

    uint8_t fonts[] = {0xf0,0x90,0x90,0x90,0xf0,
                       0x20,0x60,0x20,0x20,0x70,
                       0xf0,0x10,0xf0,0x80,0xf0,
                       0xf0,0x10,0xf0,0x10,0xf0,
                       0x90,0x90,0xf0,0x10,0x10,
                       0xf0,0x80,0xf0,0x10,0xf0,
                       0xf0,0x80,0xf0,0x90,0xf0,
                       0xf0,0x10,0x20,0x40,0x40,
                       0xf0,0x90,0xf0,0x90,0xf0,
                       0xf0,0x90,0xf0,0x10,0xf0,
                       0xf0,0x90,0xf0,0x90,0x90,
                       0xe0,0x90,0xe0,0x90,0xe0,
                       0xf0,0x80,0x80,0x80,0xf0,
                       0xe0,0x90,0x90,0x90,0xe0,
                       0xf0,0x80,0xf0,0x80,0xf0,
                       0xf0,0x80,0xf0,0x80,0x80};
    for(int i = 0; i < 16*5; ++i)
    {
        ram[i] = fonts[i];
    }
}

void Cpu::loadRom(uint8_t *rom, int romsize)
{
    for(int i = 0; i < romsize; ++i)
    {
        ram[0x200 + i] = rom[i]; 
    }
}

void Cpu::execute()
{   
    //fetch opcode
    uint16_t opcode = (uint16_t)((ram[pc] << 8) | (ram[pc + 1]));
    if(opcode == 0x00E0)    //cls
    {
        chip->cls();
    }
    else if(opcode == 0x00EE)   //ret
    {
        pc = stack[sp - 1];
        sp--;
    }
    else
    {
        uint8_t first = (opcode & 0xF000) >> 12;
        switch(first)
        {
            case 1: //jp addr
            {
                pc = (opcode & 0x0FFF) - 2; 
                break; 
            }
            case 2: //call addr
            {
                stack[sp] = pc;
                ++(sp);
                pc = (opcode & 0x0FFF)-2;  
                break;
            }
            case 3://se vx, byte
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) opcode & 0x00FF;
                if(v[op1] == op2)
                    pc += 2;
                break;
            }
            case 4://sne vx, byte
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) opcode & 0x00FF;
                if(v[op1] != op2)
                    pc += 2;
                break;
            }
            case 5://se vx, vy
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00F0) >> 4);
                if(v[op1] == v[op2])
                {
                    pc += 2;
                }
                break;
            }
            case 6://ld vx, byte
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00FF));
                v[op1] = op2;
                break;
            }
            case 7://add vx, byte
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00FF));
                v[op1] += op2;
                break;
            }
            case 8:
            {
                uint8_t mode = (uint8_t) ((opcode & 0x000F));
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00F0) >> 4);
                switch(mode)
                {
                    case 0://ld vx,vy
                        v[op1] = v[op2];
                        break;
                    case 1:
                        v[op1] |= v[op2];
                        break;
                    case 2:
                        v[op1] &= v[op2];
                        break;
                    case 3:
                        v[op1] ^= v[op2];
                        break;
                    case 4:
                        v[op1] += v[op2]; break;
                    case 5:
                        v[op1] -= v[op2]; break;
                    case 6:
                        v[0x0F] = v[op1] & 0x01;
                        v[op1] = v[op1] >> 1;
                        break;
                    case 7:
                    {
                        if(v[op2] > v[op1])
                            v[0x0F] = 1;
                        else
                            v[0x0F] = 0;
                        v[op1] = v[op2] - v[op1];
                        break;
                    }
                    case 0x0E:
                        v[0x0F] = (v[op1] & 0x80)>>7; //1000 0000
                        v[op1] = v[op1] << 1;
                        break;
                    default: break;
                }
                break;
            }
            case 9://sne vx, vy
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00F0) >> 4);
                if(v[op1] != v[op2])
                    pc += 2;
                break;
            }
            case 0x0A://ld I, addr
            {
                i = opcode & 0x0FFF;
                break;
            }
            case 0x0B://jp v0, addr
            {
                pc = (opcode & 0x0FFF) + v[0] - 2;
                break;
            }
            case 0x0C://rnd vx, byte
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00FF));
                v[op1] = ((uint8_t)(rand() % 256)) & op2;
                break;
            }
            case 0x0D://drw vx,vy, nibble
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00F0) >> 4);
                uint8_t op3 = (uint8_t) ((opcode & 0x000F));
                uint8_t *buffer = new uint8_t[op3];
                for(int i = 0; i < op3; ++i)
                {
                    buffer[i] = ram[this->i + i];
                }
                v[0x0F] = chip->drawSprite(v[op1], v[op2], buffer, op3);
                delete buffer;
                break;   
            }
            case 0x0E://
            {
                uint8_t mode = (uint8_t)(opcode & 0x00FF);
                switch(mode)
                {
                    case 0x9E: //skp vx
                    {
                        //uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        //KEYBOARD
                        break;
                    }
                    case 0xA1: //sknp vx
                    {
                        //uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        //KEYBOARD
                        pc += 2;
                        break;
                    }
                    default: break;
                }
                break;
            }
            case 0x0F:
            {
                uint8_t mode = (uint8_t)(opcode & 0x00FF);
                switch(mode)
                {
                    case 0x07: //ld vx, dt
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        v[op1] = chip->getDT();
                        break;
                    }
                    case 0x0A: //ld vx,k
                    {
                        //KEYPRESS WITH STOP
                        uint8_t op1 = (uint8_t)((opcode & 0xF00) >> 8);
                        v[op1] = '0';
                        break;
                    }
                    case 0x15: //ld dt, vx
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        chip->setDT(v[op1]);
                        break;
                    }
                    case 0x18:
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        chip->setST(v[op1]);
                        break;
                    }
                    case 0x1E: //add i, vx
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        i += v[op1];  
                        break;
                    }
                    case 0x29: //ld f, vx
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        uint8_t chara = v[op1];
                        i = ((uint16_t)chara) * 5;
                        break;
                    }
                    case 0x33://ld b, vx
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        ram[i] = v[op1] / 100;
                        ram[i+1] = (v[op1] / 10) % 10;
                        ram[i+2] = v[op1] % 10;
                        break;
                    }
                    case 0x55://ld [i], vx
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        for(int i = 0; i <= op1; ++i)
                        {
                            ram[i + i] = v[i];
                        }
                        break;
                    }
                    case 0x65://ld vx,[i]
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        for(int i = 0; i <= op1; ++i)
                        {
                            v[i] = ram[i + i];
                        }
                        break;
                    }
                }
                break; 
            }
            default: break;   
        }
    }
    pc += 2;
}