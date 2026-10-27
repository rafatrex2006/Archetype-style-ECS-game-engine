#ifndef FRX_renderer_H
#define FRX_renderer_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include "System_Handler/system_handler.h"
#include "data_structures/World.h"

//one query per drawable component type
typedef enum FRX_RenderQuery {
    QUERY_SHAPE = 0,
    QUERY_SURFACE,

    QUERY_COUNT
} FRX_RenderQuery;

typedef struct FRX_renderer{
    SDL_Window *window;
    SDL_Renderer *renderer;
    archetype_array *query[QUERY_COUNT];
} FRX_renderer;

void *FRX_CreateRenderer(World *W);

void FRX_DestroyRenderer(World *W, void *state);

void FRX_UpdateRenderer(World *W, float dt, void *state);

#endif