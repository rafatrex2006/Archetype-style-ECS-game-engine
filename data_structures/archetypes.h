#ifndef archetypes_H
#define archetypes_H

#include "components.h"

//forward declarations:

typedef struct archetype {
    signature sig;

    ComponentColumn *data;

    entity **entities;

    //metadata *meta; //you can add this later to remove all these World dependencies.

    int entityCount;

    int capacity;

} archetype;

archetype *createArc(World *W, signature sig, int capacity);

void add_component(World *W, entity *Ent, Component Comp, void *data);

void remove_component(World *W, entity *Ent, Component Comp);

void DestroyEnt(World *W, entity *Ent);

void DoubleArchSize(archetype *arch);

void freeArc(World *W, archetype *arc);

#endif