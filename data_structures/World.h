#ifndef World_H
#define World_H

#include "archetype_array.h"
#include "metadata/metadata.h"
#include "archetype_hashtable.h"
#include "System_Handler/system_handler.h"
#include "queries.h"
#include <SDL3/SDL.h>

typedef struct World{
    archetype_array *ArcArray;
    archetype_hasht *ArcTable;
    queries *QueryArr;
    metadata *metadata;
    SystemRegistry *systems;
    SDL_Window *window;
} World;

World *createWorld(int size, int EntSize, metadata *meta, SDL_Window *window);

void freeWorld(World *W);

//returns the system id or -1 on bad input
int register_system_world(World *W, const SystemDesc *desc);

//runs init on every registered system, systems registered after this are initialized right away
//returns false if any system failed to start, those are disabled and the rest still run
bool world_init_systems(World *W);

void UpdateWorld(World *W, float dt);

#endif