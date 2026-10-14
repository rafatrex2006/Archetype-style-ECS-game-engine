#ifndef FRX_renderer_H
#define FRX_renderer_H


#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct FRX_renderer{
    SDL_Window *window;
} FRX_renderer;

FRX_renderer *FRX_CreateRenderer(SDL_Window *window);

void FRX_DestroyRenderer(FRX_renderer *renderer);

#endif