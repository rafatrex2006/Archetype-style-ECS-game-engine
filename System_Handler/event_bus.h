#ifndef event_bus_H
#define event_bus_H

#include <stddef.h>
#include <stdint.h>
#include "metadata/metadata.h"  //EventType, event sizes

//forward declared, World.h includes system_handler.h which includes this header
typedef struct World World;

//one channel per event type, double buffered:
//buf[cur] gets this frame's events, the other buffer holds last frame's
//every event gets a number that only goes up, readers remember the next number they haven't read
typedef struct EventChannel{
    size_t elem_size;
    unsigned char *buf[2];
    int count[2];
    int cap[2];
    uint64_t start[2];  //number of the first event in each buffer
    int cur;            //buffer being written this frame
    uint64_t total;     //events ever emitted, also the number of the next one
} EventChannel;

typedef struct EventBus{
    EventChannel channels[EVENT_COUNT];
} EventBus;

//keep one per event type a system reads, in the system's state
typedef struct EventReader{
    uint64_t next;
} EventReader;


EventBus *createEventBus(const metadata *M);

void freeEventBus(EventBus *B);

//called once at the start of every frame by systems_run
//drops events older than last frame, so an event lives for the frame it was sent in and the next one
void event_bus_swap(EventBus *B);

//event is copied, NULL sends a zeroed event
void emit_event(World *W, EventType type, const void *event);

//a reader that only sees events sent from now on, create it in the system's init
EventReader event_reader(World *W, EventType type);

//next unread event, or NULL when there is none
//the pointer is only valid until the next emit of the same type, copy the event if you emit that type while reading it
const void *read_event(World *W, EventType type, EventReader *r);

#endif
