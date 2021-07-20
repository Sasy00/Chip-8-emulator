#include "Video.h"
#include "Chip8.h"

Video::Video(Chip8 *a) : chip(a)
{
    
}

void Video::deinit()
{
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Video::init()
{
    cls();
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    else
    {
        //Create window
        window = SDL_CreateWindow( "Chip-8", SDL_WINDOWPOS_UNDEFINED, 
                SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 
                SDL_WINDOW_SHOWN );
        if( window == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", 
                    SDL_GetError() );
        }
        else
        {
            //Get window surface
            screenSurface = SDL_GetWindowSurface( window );

            //Fill the surface black
            SDL_FillRect( screenSurface, NULL, 
                    SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00 ));
            
            //Update the surface
            SDL_UpdateWindowSurface( window );
            gRenderer = SDL_CreateRenderer( window, -1, 
                                                  SDL_RENDERER_ACCELERATED );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", 
				        SDL_GetError() );
			}
        }
    }
}

void Video::cls()
{
    for(int i = 0; i < 64*32; ++i)
    {
        screenBuffer[i] = 0;
    }
    update();
}

/*
    x,y = screen position of the sprite
    sprite is an array of #rows 8bit numbers.
    each bit represent a pixel inside the the sprite
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
    update();
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
            if(screenBuffer[indexof(i, j, 64)])
            {
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            else
            {
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
            }
            pixel = {j * size1, i * size2, size1, size2};
            SDL_RenderFillRect(gRenderer, &pixel);       
        }
    }
    SDL_RenderPresent(gRenderer); 
}