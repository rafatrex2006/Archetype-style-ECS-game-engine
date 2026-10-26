#include <SDL3/SDL.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "renderer/renderer.h"
#include "data_structures/World.h"
#include "metadata/metadata.h"

int main(int argc, char *argv[])
{

    FRX_renderer* renderer= FRX_CreateRenderer();

    if(!renderer){
        return 0;
    }

    metadata *meta = createMetadata(64);
    World *World = createWorld(20, 100, meta);
    
    bool running = true;
    SDL_Event event;

    //SDL_FRect rect = {300, 200, 200, 100};


    //float x, y;
    // Main loop
    while (running) {
        //SDL_GetMouseState(&x, &y);
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        FRX_UpdateRenderer(renderer);
    }
    //Cleanup
    freeWorld(World);
    freeMetadata(meta);
    FRX_DestroyRenderer(renderer);
    return 0;
}