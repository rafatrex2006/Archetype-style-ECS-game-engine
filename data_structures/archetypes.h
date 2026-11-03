#ifndef archetypes_H
#define archetypes_H

#include "components.h"

//forward declarations:

typedef struct archetype {
    signature sig;

    ComponentColumn *data;

    uint32_t *entities;     //entity table slot of each row

    //metadata *meta; //you can add this later to remove all these World dependencies.

    int entityCount;

    int capacity;

} archetype;

archetype *createArc(World *W, signature sig, int capacity);

//add, remove and destroy do nothing if the entity is dead or the handle is stale
void add_component(World *W, entity e, Component Comp, void *data);

void remove_component(World *W, entity e, Component Comp);

void DestroyEnt(World *W, entity e);

void DoubleArchSize(archetype *arch);

void freeArc(World *W, archetype *arc);

#endif