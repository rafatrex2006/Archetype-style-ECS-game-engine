#ifndef entity_table_H
#define entity_table_H

#include <stdbool.h>
#include <stdint.h>
#include "components.h"

//one per entity, says where its components live
//slots are never freed, only reused, so checking an old handle is always safe
typedef struct EntitySlot {
    archetype *arch;    //NULL while the slot is free
    int row;            //row inside arch
    uint32_t gen;       //bumped every time the entity in this slot dies
    int next_free;      //free list link, -1 = end
} EntitySlot;

typedef struct EntityTable {
    EntitySlot *slots;
    int size;
    int n;              //slots used so far, the most entities that were ever alive at once
    int free_head;      //first reusable slot, -1 = none
} EntityTable;

EntityTable *CreateEntityTable(int size);

void FreeEntityTable(EntityTable *T);

entity CreateEntity(World *W);

//NULL if the handle is dead, stale or NULL_ENTITY
EntitySlot *GetSlot(World *W, entity e);

bool entity_alive(World *W, entity e);

//pointer to the entity's component, NULL if the entity is dead or doesn't have it
void *get_component(World *W, entity e, Component Comp);

//frees the slot for reuse and makes every handle to it stale, used by DestroyEnt
void ReleaseSlot(EntityTable *T, uint32_t index);

#endif
