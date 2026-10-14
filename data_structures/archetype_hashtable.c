#include "archetype_hashtable.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include "World.h"

int ArcHash(signature sig, size_t size){
    return sig%size;
}

archetype_hasht *createArcTable(int size, int ArcSize, archetype_array *arr){
    archetype_hasht *ht = (archetype_hasht*)malloc(sizeof(archetype_hasht));
    ht->archetypes = (archetype**)calloc(size, sizeof(archetype*));
    
    //create archtype zero
    archetype *temp = (archetype*)malloc(sizeof(archetype));
    temp->entities = (entity**)malloc(ArcSize*sizeof(entity*)*10);
    temp->capacity = ArcSize*10;
    temp->entityCount = 0;
    temp->data = NULL;
    temp->sig = 0;

    ht->archetypes[ArcHash(0,size)] = temp;
    AddArcToArray(arr, temp);

    //
    ht->size = size;
    ht->ArcSize = ArcSize;
    ht->entity_Count = 0;
    return ht;
}

void DoubleHashSize(archetype_hasht *ht){
    size_t oldSize = ht->size;
    ht->size *= 2;
    archetype** new_arctable = (archetype**)calloc(ht->size, sizeof(archetype*));
    for (size_t i = 0; i < oldSize; i++){
        if (ht->archetypes[i] == NULL)
            continue;
        int index = ArcHash(ht->archetypes[i]->sig, ht->size);
        archetype *temp;
        for(size_t j = 0; j < ht->size; j++){
            temp = new_arctable[(index + j) % ht->size];
            if(temp == NULL){
                new_arctable[(index+j)%ht->size] = ht->archetypes[i];
                break;
            }
        }
    }

    free(ht->archetypes);
    ht->archetypes = new_arctable;
}

void freeArcTable(World *W){
    archetype_hasht *ht = W->ArcTable;
    for(size_t i = 0; i < ht->size; i++){
        freeArc(W, ht->archetypes[i]);
    }
    free(ht->archetypes);
    free(ht);
}

archetype *HashLookupAdd(World *W, signature sig){
    archetype_hasht *ht = W->ArcTable;
    int index = ArcHash(sig, ht->size);
    archetype *temp;

    for(size_t i = 0; i < ht->size; i++){
        temp = ht->archetypes[(index+i)%ht->size];
        if(temp == NULL){
            temp = createArc(W, sig, ht->ArcSize);
            ht->archetypes[(index+i)%ht->size] = temp;
            AddArcToArray(W->ArcArray, temp);
            return temp;
        }
        else if(temp->sig == sig){
            return temp;
        }
    }
    DoubleHashSize(ht);
    return HashLookupAdd(W, sig);
}

void AddArc(World *W, archetype *arc){
    archetype_hasht *ht = W->ArcTable;
    int index = ArcHash(arc->sig, ht->size);
    archetype *temp;

    for(size_t i = 0; i < ht->size; i++){
        temp = ht->archetypes[(index+i)%ht->size];
        if(temp == NULL){
            ht->archetypes[(index+i)%ht->size] = arc;
            AddArcToArray(W->ArcArray, arc);
            return;
        }
        else if(temp->sig == arc->sig){
            return;
        }
    }
    DoubleHashSize(ht);
    AddArc(W, arc);
}


entity *CreateEntity(World *W){
    entity *Ent = (entity*)malloc(sizeof(entity));
    Ent->id = W->ArcTable->entity_Count++;

    //add to the hashmap.
    Ent->arch = HashLookupAdd(W, 0);
    if(Ent->arch->entityCount == Ent->arch->capacity)
        DoubleArchSize(Ent->arch);
    Ent->index = Ent->arch->entityCount++;
    Ent->arch->entities[Ent->index] = Ent;
    return Ent;
}

archetype *HashLookupComp(World *W, Component *comps, int n){
    signature sig = 0;
    for(int i = 0; i < n; i++){
        sig |= 1ULL << comps[i];
    }
    return HashLookupAdd(W, sig);
}

//int main(int argc, char *argv[]) {
//
//}