#include "renderer.h"

FRX_renderer *FRX_CreateRenderer(){
    if (!FRX_CreateRenderer) {
        return 0;
    }
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 0;
    }

    SDL_Window *window = SDL_CreateWindow("SDL3 Window", 800, 600, SDL_WINDOW_RESIZABLE);

    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer) {
        SDL_Log("Renderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    FRX_renderer *rd = (FRX_renderer*)malloc(sizeof(FRX_renderer));
    rd->window = window;
    rd->renderer = renderer;

    return rd;
}

void FRX_DestroyRenderer(FRX_renderer *rd){
    // Cleanup
    SDL_DestroyRenderer(rd->renderer);
    SDL_DestroyWindow(rd->window);
    SDL_Quit();
    free(rd);
}


void FRX_UpdateRenderer(FRX_renderer *rd){
    SDL_RenderPresent(rd->renderer);
}