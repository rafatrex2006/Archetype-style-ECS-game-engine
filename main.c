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
    
    SDL_Surface *fade = SDL_LoadPNG("fade.png");
    if (!fade) {
        SDL_Log("Could not load fade.png: %s", SDL_GetError());
    }

    entity *box = CreateEntity(World);
    add_component(World, box, TRANSFORM, &(Transform){50, 50});
    add_component(World, box, SHAPE, &(Shape){.type = SHAPE_RECT, .color = {1, 0, 0, 1}, .filled = true, .rect = {200, 100}});

    entity *sprite = CreateEntity(World);
    add_component(World, sprite, TRANSFORM, &(Transform){100, 100});
    add_component(World, sprite, SURFACE, &(Surface){.surface = fade});

    
    //skip the loop and clean up if a system (like the renderer) failed to start
    bool running = world_init_systems(World);
    if (!running) {
        SDL_Log("A system failed to start, quitting");
    }
    Uint64 last = SDL_GetTicksNS();
    SDL_Event event;

    //SDL_FRect rect = {300, 200, 200, 100};


    //float x, y;
    // Main loop
    while (running) {
        //SDL_GetMouseState(&x, &y);
        Uint64 now = SDL_GetTicksNS();
        UpdateWorld(World, (now - last) / 1e9f); // dt in seconds
        last = now;
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
    SDL_DestroySurface(fade);
    return 0;
}