#ifndef VIDEO_H
#define VIDEO_H

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 400

#include <SDL2/SDL.h>


class Video
{
public:
    Video(class Chip8 *back);
    void init();
    uint8_t drawSprite(uint8_t x, uint8_t y, uint8_t *sprite, uint8_t rows);
    void update();
    void cls();
    void closeSDL();

    inline static int indexof(int row, int col, int nCols)
    {
        return row * nCols + col;
    }
private:
    uint8_t screenBuffer[64*32];
    SDL_Window *window;
    SDL_Surface *screenSurface;
    SDL_Renderer *gRenderer;
    class Chip8 *chip;
};

#endif