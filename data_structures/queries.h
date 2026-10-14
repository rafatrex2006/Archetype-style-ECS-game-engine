#ifndef queries_H
#define queries_H

#include <stdlib.h>
#include <stdbool.h>
#include "archetype_array.h"

typedef struct queries{
    signature *sigs_in;
    signature *sigs_out; //gotta update everywhere for this
    archetype_array **list;
    int size;
    int n;
} queries;


queries *CreateQueryArr(int size);

void freeQueryArr(queries *q);

archetype_array *createQuery(World *W, signature sig);

archetype_array *createCustomQueue(World *W, Component *in, int SizeOfin, Component *out, int SizeOfout);



#endif