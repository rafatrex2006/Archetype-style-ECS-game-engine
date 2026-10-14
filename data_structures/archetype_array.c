#include "archetype_array.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include "World.h"

archetype_array *CreateArcArray(int size){
    if(size < 1){
        size = 1;
    }
    archetype_array *arr = (archetype_array*)malloc(sizeof(archetype_array));
    arr->arcs = (archetype**)malloc(size*sizeof(archetype*));

    arr->n = 0;
    arr->size = size;
    return arr;
}

void freeArcArray(archetype_array *arr){
    free(arr->arcs);
    free(arr);
}

void AddArcToArray(archetype_array *arr, archetype *arc){
    if(arr->n == arr->size){
        arr->size *= 2;
        arr->arcs = (archetype**)realloc(arr->arcs, arr->size*sizeof(archetype*));
    }
    arr->arcs[arr->n++] = arc;
}

void RemoveArcFromArray(World *W, signature sig){
    archetype_array *arr = W->ArcArray;
    for(int i = 0; i < arr->n; i++){
        if(arr->arcs[i]->sig == sig){
            freeArc(W, arr->arcs[i]);
            arr->arcs[i] = arr->arcs[--arr->n];
            break;
        }
    }
}