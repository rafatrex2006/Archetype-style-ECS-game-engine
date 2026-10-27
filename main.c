#include <SDL3/SDL.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "renderer/renderer.h"
#include "data_structures/World.h"
#include "metadata/metadata.h"

int main(int argc, char *argv[])
{
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

    metadata *meta = createMetadata(64);
    World *World = createWorld(20, 100, meta, window);
    register_system_world(World, &(SystemDesc){.name    = "renderer", .phase   = PHASE_RENDER, .init    = FRX_CreateRenderer, .update  = FRX_UpdateRenderer, .destroy = FRX_DestroyRenderer});

    
    world_init_systems(World);
    
    bool running = true;
    SDL_Event event;

    //SDL_FRect rect = {300, 200, 200, 100};


    //float x, y;
    // Main loop
    while (running) {
        //SDL_GetMouseState(&x, &y);
        UpdateWorld(World, 0); //0 should be changed to time later
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
    }
    //Cleanup
    freeWorld(World);
    freeMetadata(meta);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}