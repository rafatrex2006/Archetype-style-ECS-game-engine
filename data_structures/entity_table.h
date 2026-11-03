#ifndef entity_table_H
#define entity_table_H

#include <stdbool.h>
#include <stdint.h>
#include "components.h"


typedef struct EntitySlot {
    archetype *arch; 
    int row;
    uint32_t gen;
    int next_free;
} EntitySlot;

typedef struct EntityTable {
    EntitySlot *slots;
    int size;
    int n;
    int free_head;
} EntityTable;

EntityTable *CreateEntityTable(int size);

void FreeEntityTable(EntityTable *T);

entity CreateEntity(World *W);

EntitySlot *GetSlot(World *W, entity e);

bool entity_alive(World *W, entity e);

void *get_component(World *W, entity e, Component Comp);

void ReleaseSlot(EntityTable *T, uint32_t index);

#endif
