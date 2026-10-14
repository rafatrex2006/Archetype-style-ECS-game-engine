#include "renderer.h"

FRX_renderer *FRX_CreateRenderer(SDL_Window *window){
    FRX_renderer *renderer = (FRX_renderer*)malloc(sizeof(FRX_renderer));
    renderer->window = window;
    return renderer;
}

void FRX_DestroyRenderer(FRX_renderer *renderer){
    free(renderer);
}