#include "World.h"

#include <stdlib.h>
#include <stdio.h>

World *createWorld(int size, int EntSize, metadata *meta){
    World *W = (World*)malloc(sizeof(World));
    W->ArcArray = CreateArcArray(size);
    W->ArcTable = createArcTable(size, EntSize, W->ArcArray); //maybe remove arr later
    W->QueryArr = CreateQueryArr(size);
    W->metadata = meta;
    return W;
}

void freeWorld(World *W){
    freeArcTable(W);
    freeArcArray(W->ArcArray); //maybe change later
    freeQueryArr(W->QueryArr);
    free(W);
}