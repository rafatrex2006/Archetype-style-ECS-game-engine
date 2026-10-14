#ifndef metadata_H
#define metadata_H

#include "data_structures/components.h"

typedef struct metadata{
    //Component *id;
    size_t *sizes;
    size_t *alignment;

    void (**constructor)(void *);
    void (**destructor)(void *);
    void (**move)(void *dst, void *src);
} metadata;


void assign_new_component(metadata *M, size_t size, Component id, size_t alignment, void (*constructor)(void *), void (*destructor)(void *), void (*move)(void *, void *));

metadata *createMetadata(int n);

void freeMetadata(metadata *meta);


#endif