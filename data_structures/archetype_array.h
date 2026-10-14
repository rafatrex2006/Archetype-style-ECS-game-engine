#ifndef archetype_array_H
#define archetype_array_H

#include "archetypes.h"

//typedef int entity;

typedef struct archetype_array{
    archetype **arcs;

    int size;
    int n;
} archetype_array;

archetype_array *CreateArcArray(int size);

void freeArcArray(archetype_array *arr);

void AddArcToArray(archetype_array *arr, archetype *arc); //maybe change later to world or chang ethe other worlds to metadata plus arr.

void RemoveArcFromArray(World *W, signature sig);

#endif