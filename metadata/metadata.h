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

//one id per event type, register each one in createMetadata
//the event structs are included in metadata.c, not here: they use entity, and components.h includes this header
typedef enum EventType {
    EVT_DAMAGE = 0,

    EVENT_COUNT
} EventType;

typedef struct metadata{
    //Component *id;
    size_t *sizes;
    size_t *alignment;

    void (**constructor)(void *);
    void (**destructor)(void *);
    void (**move)(void *dst, void *src);

    size_t *event_sizes;
} metadata;


void assign_new_component(metadata *M, size_t size, Component id, size_t alignment, void (*constructor)(void *), void (*destructor)(void *), void (*move)(void *, void *));

void assign_new_event(metadata *M, size_t size, EventType id);

metadata *createMetadata(int n);

void freeMetadata(metadata *meta);


#endif