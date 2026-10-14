#ifndef archetypes_hastable_H
#define archetypes_hastable_H

#include "archetype_array.h"

typedef struct archetype_hasht{
    archetype **archetypes;
    size_t size;
    int ArcSize;
    int entity_Count;
} archetype_hasht;

archetype_hasht *createArcTable(int size, int ArcSize, archetype_array *arr); //maybe remove arr later

void freeArcTable(World *W);

archetype *HashLookupAdd(World *W, signature sig);

void AddArc(World *W, archetype *arc);

entity *CreateEntity(World *W);

archetype *HashLookupComp(World *W, Component *comps, int n);

#endif