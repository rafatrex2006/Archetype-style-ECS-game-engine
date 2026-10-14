#include "archetypes.h"


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include "archetype_hashtable.h"
#include "World.h"


archetype *createArc(World *W, signature sig, int capacity){
    archetype *arc = (archetype*)malloc(sizeof(archetype));
    arc->entities = (entity**)malloc(capacity*sizeof(entity*));
    arc->entityCount = 0;
    arc->capacity = capacity;
    arc->sig = sig;

    //int column = __builtin_popcountll(sig & ((1ULL << component) - 1));
    int count = __builtin_popcountll(sig);
    arc->data = (ComponentColumn*)malloc(count*sizeof(ComponentColumn));
    int index = 0;
    unsigned long long bits = sig;
    while (bits)
    {   
        int component = __builtin_ctzll(bits);

        size_t bit_size = W->metadata->sizes[component];
        arc->data[index].data = (void *)malloc(bit_size*capacity);
        arc->data[index++].DataSize = bit_size;

        bits &= bits - 1;
    }
    
    for(int i = 0; i < W->QueryArr->n; i++){
        if (((W->QueryArr->sigs_in[i] & sig) == W->QueryArr->sigs_in[i]) && !(W->QueryArr->sigs_out[i] & sig)){
            AddArcToArray(W->QueryArr->list[i], arc);
        }
    }

    return arc;
}

void DoubleArchSize(archetype *arch){
    int count = __builtin_popcountll(arch->sig);
    arch->capacity *= 2;

    arch->entities = (entity**)realloc(arch->entities, arch->capacity*sizeof(entity*));
    for(int i = 0; i < count; i++){
        arch->data[i].data = (void*)realloc(arch->data[i].data, arch->capacity*arch->data[i].DataSize);
    }
}

static inline void *ComponentPtr(ComponentColumn* col, size_t row){
    return (char *)col->data + row * col->DataSize;
}

void ArcMoveEnt(World *W, void* src, void* dst, int component){
    if(W->metadata->move[component]){
        W->metadata->move[component](dst, src);
    }
    else{
        memcpy(dst, src, W->metadata->sizes[component]);
    }
}

void add_component(World *W, entity *Ent, Component Comp, void *data){
    signature sig = Ent->arch->sig;
    signature new_sig = sig | (1ULL << Comp);

    if(sig == new_sig){
        //already has the component: just overwrite the value
        if(data){
            int col = __builtin_popcountll(sig & ((1ULL << Comp) - 1));
            memcpy(ComponentPtr(&Ent->arch->data[col], Ent->index), data, Ent->arch->data[col].DataSize);
        }
        return;
    }

    archetype *arc1 = Ent->arch;
    archetype *arc2 = HashLookupAdd(W, new_sig);

    if(arc2->entityCount == arc2->capacity)
        DoubleArchSize(arc2);

    int oldRow = Ent->index;
    int newRow = arc2->entityCount;
    int lastRow = arc1->entityCount-1;

    unsigned long long bits = sig;
    while (bits)
    {   
        int component = __builtin_ctzll(bits);
        int col = __builtin_popcountll(sig & ((1ULL << component) - 1));
        int new_col = __builtin_popcountll(new_sig & ((1ULL << component) - 1));

        ArcMoveEnt(W, ComponentPtr(&arc1->data[col], oldRow), ComponentPtr(&arc2->data[new_col], newRow), component);

        if(oldRow != lastRow){
            ArcMoveEnt(W, ComponentPtr(&arc1->data[col], lastRow), ComponentPtr(&arc1->data[col], oldRow), component);
        }

        bits &= bits - 1;
    }

    arc2->entities[newRow] = arc1->entities[Ent->index];
    if(Ent->index != lastRow){
        arc1->entities[Ent->index] = arc1->entities[lastRow];
        arc1->entities[Ent->index]->index = Ent->index;
    }
    arc1->entities[lastRow] = NULL;
    Ent->index = newRow;
    Ent->arch = arc2;

    int new_col = __builtin_popcountll(new_sig & ((1ULL << Comp) - 1));
    void *dst = ComponentPtr(&arc2->data[new_col], newRow);
    if (data)
        memcpy(dst, data, arc2->data[new_col].DataSize);
    else if (W->metadata->constructor[Comp])
        W->metadata->constructor[Comp](dst);
    
    
    arc2->entityCount++;
    arc1->entityCount--;
}


void remove_component(World *W, entity *Ent, Component Comp){
    signature sig = Ent->arch->sig;
    signature new_sig = sig & (~(1ULL << Comp));

    if(sig == new_sig){
        return; //component not present
    }

    archetype *arc1 = Ent->arch;
    archetype *arc2 = HashLookupAdd(W, new_sig);

    if(arc2->entityCount == arc2->capacity)
        DoubleArchSize(arc2);

    int oldRow = Ent->index;
    int newRow = arc2->entityCount;
    int lastRow = arc1->entityCount-1;

    unsigned long long bits = new_sig;
    while (bits)
    {   
        int component = __builtin_ctzll(bits);
        int col = __builtin_popcountll(sig & ((1ULL << component) - 1));
        int new_col = __builtin_popcountll(new_sig & ((1ULL << component) - 1));

        ArcMoveEnt(W, ComponentPtr(&arc1->data[col], oldRow), ComponentPtr(&arc2->data[new_col], newRow), component);

        if(oldRow != lastRow){
            ArcMoveEnt(W, ComponentPtr(&arc1->data[col], lastRow), ComponentPtr(&arc1->data[col], oldRow), component);
        }

        bits &= bits - 1;
    }

    int old_col = __builtin_popcountll(sig & ((1ULL << Comp) - 1));
    if (W->metadata->destructor[Comp]){
        void *src = ComponentPtr(&arc1->data[old_col], oldRow);
        W->metadata->destructor[Comp](src);
    }

    if(oldRow != lastRow){
        ArcMoveEnt(W, ComponentPtr(&arc1->data[old_col], lastRow), ComponentPtr(&arc1->data[old_col], oldRow), Comp);
    }


    arc2->entities[newRow] = arc1->entities[Ent->index];
    if(Ent->index != lastRow){
        arc1->entities[Ent->index] = arc1->entities[lastRow];
        arc1->entities[Ent->index]->index = Ent->index;
    }
    arc1->entities[lastRow] = NULL;
    Ent->index = newRow;
    Ent->arch = arc2;

    arc2->entityCount++;
    arc1->entityCount--;

}


/*
 * move(dst, src):
 *  - Transfers ownership from src to dst.
 *  - After return, src must be left in a valid destructible state.
 *    (typically by setting owned pointers to NULL).
 */


void DestroyEnt(World *W, entity *Ent){
    archetype *arc1 = Ent->arch;
    int oldRow = Ent->index;
    int lastRow = arc1->entityCount-1;

    unsigned long long bits = arc1->sig;
    while (bits)
    {
        int Comp = __builtin_ctzll(bits);
        int col = __builtin_popcountll(arc1->sig & ((1ULL << Comp) - 1));

        //1. destroy the component of the entity that is dying
        if (W->metadata->destructor[Comp]){
            W->metadata->destructor[Comp](ComponentPtr(&arc1->data[col], oldRow));
        }

        //2. fill the hole with the last row (swap-remove)
        if(oldRow != lastRow){
            ArcMoveEnt(W, ComponentPtr(&arc1->data[col], lastRow), ComponentPtr(&arc1->data[col], oldRow), Comp);
        }

        bits &= bits - 1;
    }
    if (oldRow != lastRow) {
        arc1->entities[oldRow] = arc1->entities[lastRow];
        arc1->entities[oldRow]->index = oldRow;
    }

    arc1->entities[lastRow] = NULL;
    arc1->entityCount--;
    free(Ent);
}


void freeArc(World *W, archetype *arc){
    if(arc == NULL){
        return;
    }
    unsigned long long bits = arc->sig;
    while (bits)
    {   
        int Comp = __builtin_ctzll(bits);
        int col = __builtin_popcountll(arc->sig & ((1ULL << Comp) - 1));

        if (W->metadata->destructor[Comp]){
            for(int i = 0; i < arc->entityCount; i++){
                void *src = ComponentPtr(&arc->data[col], i);
                W->metadata->destructor[Comp](src);
            }
        }
        free(arc->data[col].data);

        bits &= bits - 1;
    }
    
    for(int i = 0; i < arc->entityCount; i++){
        free(arc->entities[i]);
    }
    free(arc->entities);
    free(arc->data);
    free(arc);
}
