#ifndef World_H
#define World_H

#include "archetype_array.h"
#include "metadata/metadata.h"
#include "archetype_hashtable.h"
#include "queries.h"

typedef struct World{
    archetype_array *ArcArray;
    archetype_hasht *ArcTable;
    queries *QueryArr;
    metadata *metadata;
} World;

World *createWorld(int size, int EntSize, metadata *meta);

void freeWorld(World *W);

#endif