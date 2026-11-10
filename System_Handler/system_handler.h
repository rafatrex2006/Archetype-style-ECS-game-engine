#ifndef system_handler_H
#define system_handler_H

#include <stdbool.h>
#include "event_bus.h"
//forward declared to avoid a circular include, World.h includes this header
typedef struct World World;

//phases run in this order every frame, systems inside a phase run in registration order
typedef enum SystemPhase {
    PHASE_PRE_UPDATE = 0,
    PHASE_UPDATE,
    PHASE_POST_UPDATE,
    PHASE_RENDER,

    PHASE_COUNT
} SystemPhase;

//init mallocs the system's own state (queries, settings, subscribers...) and writes it to *state, leave it NULL for stateless systems
//returns false if the system could not start, it must clean up after itself then, destroy is not called for it
//config is SystemDesc.config, given as is: the system can read it but does not own it
typedef bool (*SystemInitFn)(World *W, void *config, void **state);
typedef void (*SystemUpdateFn)(World *W, float dt, void *state);
//frees whatever init created, if NULL the registry just calls free(state)
typedef void (*SystemDestroyFn)(World *W, void *state);

typedef struct SystemDesc {
    const char *name;
    SystemPhase phase;
    void *config;               //optional, passed to init (a window, settings...), the system does not own it
    SystemInitFn init;          //optional
    SystemUpdateFn update;
    SystemDestroyFn destroy;    //optional
} SystemDesc;

typedef struct System {
    SystemDesc desc;
    void *state;    //whatever init wrote
    bool enabled;
    bool failed;    //init returned false, the system never runs and is never destroyed
} System;

typedef struct SystemRegistry {
    World *W;
    System *systems;
    int size;
    int n;
    bool initialized;
    EventBus *events;   //this World's events, systems talk to each other through it
} SystemRegistry;


SystemRegistry *createSystemRegistry(World *W, int size);

//calls destroy on every system
void freeSystemRegistry(SystemRegistry *R);

//desc is copied, returns the system id, or -1 on bad input or if it was registered after systems_init and its init failed
int register_system(SystemRegistry *R, const SystemDesc *desc);

//runs init on every system, returns false if any of them failed (the others still run)
bool systems_init(SystemRegistry *R);

void systems_run(SystemRegistry *R, float dt);

//failed systems stay disabled
void set_system_enabled(SystemRegistry *R, int id, bool enabled);

#endif
