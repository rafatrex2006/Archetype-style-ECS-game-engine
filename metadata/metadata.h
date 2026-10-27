#ifndef metadata_H
#define metadata_H

#include <stddef.h>

//one id per component type, register each one in createMetadata
#include "components/transform.h"
#include "components/velocity.h"
#include "components/health.h"
#include "components/acceleration.h"
#include "components/render_simple.h"
#include "components/render_surface.h"


//metadata.h includes nothing from the ECS so anything can include it without loops
typedef enum Component {
    TRANSFORM = 0,
    VELOCITY,
    HEALTH,
    ACCELERATION,
    SHAPE,
    SURFACE,

    COMPONENT_COUNT
} Component;

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