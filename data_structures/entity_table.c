#include "entity_table.h"

#include <stdlib.h>
#include "World.h"

EntityTable *CreateEntityTable(int size){
    if(size < 1){
        size = 1;
    }
    EntityTable *T = (EntityTable*)malloc(sizeof(EntityTable));
    T->slots = (EntitySlot*)malloc(size*sizeof(EntitySlot));
    T->size = size;
    T->n = 0;
    T->free_head = -1;
    return T;
}

void FreeEntityTable(EntityTable *T){
    if(T == NULL){
        return;
    }
    free(T->slots);
    free(T);
}

//handles are indices, not pointers, so moving the table doesn't break them
static void DoubleEntityTable(EntityTable *T){
    T->size *= 2;
    T->slots = (EntitySlot*)realloc(T->slots, T->size*sizeof(EntitySlot));
}

entity CreateEntity(World *W){
    EntityTable *T = W->entities;
    int i;
    if(T->free_head != -1){
        //reuse a dead slot first, so the table only grows to the most entities alive at once
        i = T->free_head;
        T->free_head = T->slots[i].next_free;
    }
    else{
        if(T->n == T->size){
            DoubleEntityTable(T);
        }
        i = T->n++;
        T->slots[i].gen = 1;    //gen 0 is never used, so a zeroed entity (NULL_ENTITY) is never valid
    }

    archetype *arch = HashLookupAdd(W, 0);
    if(arch->entityCount == arch->capacity)
        DoubleArchSize(arch);

    EntitySlot *s = &T->slots[i];
    s->arch = arch;
    s->row = arch->entityCount++;
    s->next_free = -1;
    arch->entities[s->row] = (uint32_t)i;

    return (entity){ .index = (uint32_t)i, .gen = s->gen };
}

EntitySlot *GetSlot(World *W, entity e){
    EntityTable *T = W->entities;
    if(e.index >= (uint32_t)T->n){
        return NULL;
    }
    EntitySlot *s = &T->slots[e.index];
    if(s->arch == NULL || s->gen != e.gen){
        return NULL;
    }
    return s;
}

bool entity_alive(World *W, entity e){
    return GetSlot(W, e) != NULL;
}

void *get_component(World *W, entity e, Component Comp){
    EntitySlot *s = GetSlot(W, e);
    if(s == NULL || !(s->arch->sig & (1ULL << Comp))){
        return NULL;
    }
    int col = __builtin_popcountll(s->arch->sig & ((1ULL << Comp) - 1));
    return (char *)s->arch->data[col].data + s->row * s->arch->data[col].DataSize;
}

void ReleaseSlot(EntityTable *T, uint32_t index){
    EntitySlot *s = &T->slots[index];
    s->arch = NULL;
    s->gen++;
    if(s->gen == 0){
        s->gen = 1;
    }
    s->next_free = T->free_head;
    T->free_head = (int)index;
}
