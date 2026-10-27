#include "World.h"

#include <stdlib.h>
#include <stdio.h>

World *createWorld(int size, int EntSize, metadata *meta, SDL_Window *window){
    World *W = (World*)malloc(sizeof(World));
    W->ArcArray = CreateArcArray(size);
    W->ArcTable = createArcTable(size, EntSize, W->ArcArray); //maybe remove arr later
    W->QueryArr = CreateQueryArr(size);
    W->systems = createSystemRegistry(W, size);
    W->metadata = meta;
    W->window = window;
    return W;
}

void freeWorld(World *W){
    freeSystemRegistry(W->systems); //first, systems may still use the world while cleaning up
    freeArcTable(W);
    freeArcArray(W->ArcArray); //maybe change later
    freeQueryArr(W->QueryArr);
    free(W);
}

int register_system_world(World *W, const SystemDesc *desc){
    return register_system(W->systems, desc);
}

void world_init_systems(World *W){
    systems_init(W->systems);
}

void UpdateWorld(World *W, float dt){
    systems_run(W->systems, dt);
}