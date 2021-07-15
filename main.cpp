/*
    0x000-0x1FF emulator only
    0xF00-0xFFF display
    0xEFF-0XEA0 call stack, internal use other v
*/
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#define RAM_SIZE 4096
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 400
#define FPS 60

typedef struct Video
{
    uint8_t screenBuffer[64*32];
    SDL_Window *window;
    SDL_Surface *screenSurface;
    SDL_Renderer *gRenderer;
    
    Video();
    void init();
    uint8_t drawSprite(uint8_t x, uint8_t y, uint8_t *sprite, uint8_t rows);
    void update();
    void clear();
    void closeSDL();
}Video;

typedef struct Cpu
{
    uint8_t v[16];      //v[0x0] to v[0xF] 
    uint16_t pc;         //program counter
    uint16_t i;
    
    uint8_t dt;         //delay timer
    uint8_t st;         //sound timer
    
    uint8_t sp;         //stack pointer
    uint16_t stack[16];  //stack, used only for storing the pc before a call
    uint8_t ram[RAM_SIZE];
    
    Video *video;
    
    Cpu();
    void init();
    int loadRam(char *pathname);
    void execute();
    void attachVideo(Video *video);
    void printRam();
}Cpu;

/*
      012
      ___
    0|012
    1|345
    2|678
*/

int indexof(int row, int col, int nCols)
{
    return row * nCols + col;
}

Cpu::Cpu(){;}
void Cpu::init()
{
    this->pc = 0x200;
    this->dt = 0;
    this->st = 0;
    this->sp = 0;
}

//return 1 if error occurs, else 0
int Cpu::loadRam(char *pathname)
{
    FILE *fr;
    int fsize;
    if(!(fr = fopen(pathname,"r")))
    {
        return 1;
    }
    fseek(fr, 0, SEEK_END);
    fsize = ftell(fr);
    fseek(fr, 0, SEEK_SET);
    fread(this->ram+0x200, 1, fsize, fr);   
    fclose(fr);
    return 0;
}
void Cpu::attachVideo(Video *video)
{
    this->video = video;
}

Video::Video(){;}
void Video::closeSDL()
{
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void Video::init()
{
    for (int i = 0; i < 64*32; ++i)
    {
        screenBuffer[i] = 0;
    }
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    else
    {
        //Create window
        this->window = SDL_CreateWindow( "Chip-8", SDL_WINDOWPOS_UNDEFINED, 
                SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 
                SDL_WINDOW_SHOWN );
        if( this->window == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", 
                    SDL_GetError() );
        }
        else
        {
            //Get window surface
            this->screenSurface = SDL_GetWindowSurface( this->window );

            //Fill the surface black
            SDL_FillRect( this->screenSurface, NULL, 
                    SDL_MapRGB(this->screenSurface->format, 0x00, 0x00, 0x00 ));
            
            //Update the surface
            SDL_UpdateWindowSurface( this->window );
            this->gRenderer = SDL_CreateRenderer( this->window, -1, 
                                                  SDL_RENDERER_ACCELERATED );
			if( this->gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", 
				        SDL_GetError() );
			}
        }
    }
}
void Video::clear()
{
    for(int i = 0; i < 64*32; ++i)
    {
        this->screenBuffer[i] = 0;
    }
}
/*
    x,y = screen position of the sprite
    sprite is an array of #rows 8bit numbers.
    each bit represent a pixel insithe the sprite
    00000
    01110
    00000
    each row is then xored in the screenBuffer
    
    if a bit is resetted in screenBuffer then return 1 else 0
*/
uint8_t Video::drawSprite(uint8_t x, uint8_t y, uint8_t *sprite, uint8_t rows)
{
    uint8_t temp;
    uint8_t pix;
    uint8_t modified = 0;
    uint8_t currentPix;
    int index;
    
    for(int i = 0; i < rows; ++i)
    {
        temp = sprite[i];
        for(int j = 0; j < 8; ++j)
        {
            pix = (temp & 0x80) >> 7; //takes the most significant bit
            index = indexof((i + y) % 32, (j + x) % 64, 64);
            currentPix = screenBuffer[index];
            screenBuffer[index] ^= pix;
            if(currentPix == 1 &&  screenBuffer[index] == 0)
            {
                modified = 1;
            }
            temp = temp << 1;
        }
    }
    return modified;
}
void Video::update()
{
    SDL_Rect pixel;
    int size2 = (SCREEN_WIDTH / 64);
    int size1 = (SCREEN_HEIGHT / 32);
    for(int i = 0; i < 32; ++i)
    {
        for(int j = 0; j < 64; ++j)
        {
            if(this->screenBuffer[indexof(i, j, 64)])
            {
                SDL_SetRenderDrawColor(this->gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            else
            {
                SDL_SetRenderDrawColor(this->gRenderer, 0x00, 0x00, 0x00, 0x00);
            }
            pixel = {j * size1, i * size2, size1, size2};
            SDL_RenderFillRect(this->gRenderer, &pixel);       
        }
    }
    SDL_RenderPresent( this->gRenderer ); 
}


int main(int argc, char** argv)
{
    Cpu cpu;
    Video video;
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
        cpu.ram[i] = fonts[i];
    }
    if(argc != 2)
    {
        printf("ERROR Usage: %s <rom path>\n", argv[0]);
        return 1;
    }
    if(cpu.loadRam(argv[1]))
    {
        printf("ERROR: File %s not found\n", argv[1]);
        return 1;
    }
    cpu.init();
    video.init();
    video.clear();
    cpu.attachVideo(&video);
    
    srand(time(0));
    
    int end = 0;
    SDL_Event e;
    
    int frameDelay = 1000 / FPS;
    uint32_t frameStart;
    int frameTime;
    
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
        cpu.execute();
        video.update();
        
        frameTime = SDL_GetTicks() - frameStart;
        if(frameDelay > frameTime)
        {
            SDL_Delay(frameDelay - frameTime);
        }
    }
    video.closeSDL();
    return 0;
}

void Cpu::execute()
{   
    //fetch opcode
    uint16_t opcode = (uint16_t)((this->ram[this->pc] << 8) | (this->ram[this->pc + 1]));
    if(opcode == 0x00E0)    //cls
    {
        this->video->clear();
    }
    else if(opcode == 0x00EE)   //ret
    {
        this->pc = this->stack[this->sp - 1];
        this->sp--;
    }
    else
    {
        uint8_t first = (opcode & 0xF000) >> 12;
        switch(first)
        {
            case 1: //jp addr
            {
                this->pc = (opcode & 0x0FFF) - 2; 
                break; 
            }
            case 2: //call addr
            {
                this->stack[this->sp] = this->pc;
                ++(this->sp);
                this->pc = (opcode & 0x0FFF)-2;  
                break;
            }
            case 3://se vx, byte
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) opcode & 0x00FF;
                if(this->v[op1] == op2)
                    pc += 2;
                break;
            }
            case 4://sne vx, byte
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) opcode & 0x00FF;
                if(this->v[op1] != op2)
                    pc += 2;
                break;
            }
            case 5://se vx, vy
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00F0) >> 4);
                if(this->v[op1] == this->v[op2])
                {
                    pc += 2;
                }
                break;
            }
            case 6://ld vx, byte
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00FF));
                this->v[op1] = op2;
                break;
            }
            case 7://add vx, byte
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00FF));
                this->v[op1] += op2;
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
                        this->v[op1] = this->v[op2];
                        break;
                    case 1:
                        this->v[op1] |= this->v[op2];
                        break;
                    case 2:
                        this->v[op1] &= this->v[op2];
                        break;
                    case 3:
                        this->v[op1] ^= this->v[op2];
                        break;
                    case 4:
                        this->v[op1] += this->v[op2]; break;
                    case 5:
                        this->v[op1] -= this->v[op2]; break;
                    case 6:
                        this->v[0x0F] = this->v[op1] & 0x01;
                        this->v[op1] = this->v[op1] >> 1;
                        break;
                    case 7:
                    {
                        if(this->v[op2] > this->v[op1])
                            this->v[0x0F] = 1;
                        else
                            this->v[0x0F] = 0;
                        this->v[op1] = this->v[op2] - this->v[op1];
                        break;
                    }
                    case 0x0E:
                        this->v[0x0F] = (this->v[op1] & 0x80)>>7; //1000 0000
                        this->v[op1] = this->v[op1] << 1;
                        break;
                    default: break;
                }
                break;
            }
            case 9://sne vx, vy
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00F0) >> 4);
                if(this->v[op1] != this->v[op2])
                    pc += 2;
                break;
            }
            case 0x0A://ld I, addr
            {
                this->i = opcode & 0x0FFF;
                break;
            }
            case 0x0B://jp v0, addr
            {
                pc = (opcode & 0x0FFF) + this->v[0] - 2;
                break;
            }
            case 0x0C://rnd vx, byte
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00FF));
                this->v[op1] = ((uint8_t)(rand() % 256)) & op2;
                break;
            }
            case 0x0D://drw vx,vy, nibble
            {
                uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                uint8_t op2 = (uint8_t) ((opcode & 0x00F0) >> 4);
                uint8_t op3 = (uint8_t) ((opcode & 0x000F));
                uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * op3);
                //uint8_t buffer[] = {0x3c,0x3c,0x3c,0x3c};
                for(int i = 0; i < op3; ++i)
                {
                    buffer[i] = this->ram[this->i + i];
                }
                this->v[0x0F] = video->drawSprite(this->v[op1], 
                        this->v[op2], buffer, op3);
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
                        this->pc += 2;
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
                        this->v[op1] = this->dt;
                        break;
                    }
                    case 0x0A: //ld vx,k
                    {
                        //KEYPRESS WITH STOP
                        uint8_t op1 = (uint8_t)((opcode & 0xF00) >> 8);
                        this->v[op1] = '0';
                        break;
                    }
                    case 0x15: //ld dt, vx
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        this->dt = this->v[op1];
                        break;
                    }
                    case 0x1E: //add i, vx
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        this->i += this->v[op1];  
                        break;
                    }
                    case 0x29: //ld f, vx
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        uint8_t chara = this->v[op1];
                        this->i = ((uint16_t)chara) * 5;
                        break;
                    }
                    case 0x33://ld b, vx
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        this->ram[i] = this->v[op1] / 100;
                        this->ram[i+1] = (this->v[op1] / 10) % 10;
                        this->ram[i+2] = this->v[op1] % 10;
                        break;
                    }
                    case 0x55://ld [i], vx
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        for(int i = 0; i <= op1; ++i)
                        {
                            this->ram[this->i + i] = this->v[i];
                        }
                        break;
                    }
                    case 0x65://ld vx,[i]
                    {
                        uint8_t op1 = (uint8_t) ((opcode & 0x0F00) >> 8);
                        for(int i = 0; i <= op1; ++i)
                        {
                            this->v[i] = this->ram[this->i + i];
                        }
                        break;
                    }
                }
                break; 
            }
            default: break;   
        }
    }
    this->pc += 2;
    if(this->dt > 0)
        this->dt--;
    if(this->st > 0)
        this->st--;
}

void Cpu::printRam()
{
    for(int i = 0; i < RAM_SIZE; ++i)
    {
        printf("0x%02x | ", this->ram[i]);
    }
}














