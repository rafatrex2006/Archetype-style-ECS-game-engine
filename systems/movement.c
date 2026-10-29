#include "movement.h"

#include <stdlib.h>

//column index = number of component bits below this one
#define COLUMN(arc, comp) ((arc)->data[__builtin_popcountll((arc)->sig & ((1ULL << (comp)) - 1))].data)

//the query belongs to the World (freed in freeQueryArr), so it is wrapped in a struct
//and the registry's default free(state) only frees the wrapper
typedef struct QueryState {
    archetype_array *query;
} QueryState;

static bool CreateQueryState(World *W, void **state, Component a, Component b){
    QueryState *s = (QueryState*)malloc(sizeof(QueryState));
    if(!s){
        return false;
    }
    s->query = createCustomQueue(W, (Component[]){a, b}, 2, NULL, 0);
    *state = s;
    return true;
}

bool accelerate_init(World *W, void **state){
    return CreateQueryState(W, state, VELOCITY, ACCELERATION);
}

void accelerate_update(World *W, float dt, void *state){
    (void)W;
    QueryState *s = (QueryState*)state;
    if(!s){
        return;
    }
    for(int i = 0; i < s->query->n; i++){
        archetype *arc = s->query->arcs[i];
        Velocity *vel = (Velocity*)COLUMN(arc, VELOCITY);
        Acceleration *acc = (Acceleration*)COLUMN(arc, ACCELERATION);

        for(int j = 0; j < arc->entityCount; j++){
            vel[j].x += acc[j].x * dt;
            vel[j].y += acc[j].y * dt;
        }
    }
}

bool move_init(World *W, void **state){
    return CreateQueryState(W, state, TRANSFORM, VELOCITY);
}

void move_update(World *W, float dt, void *state){
    (void)W;
    QueryState *s = (QueryState*)state;
    if(!s){
        return;
    }
    for(int i = 0; i < s->query->n; i++){
        archetype *arc = s->query->arcs[i];
        Transform *pos = (Transform*)COLUMN(arc, TRANSFORM);
        Velocity *vel = (Velocity*)COLUMN(arc, VELOCITY);

        for(int j = 0; j < arc->entityCount; j++){
            pos[j].x += vel[j].x * dt;
            pos[j].y += vel[j].y * dt;
        }
    }
}

const SystemDesc ACCELERATE_SYSTEM = {
    .name = "accelerate",
    .phase = PHASE_PRE_UPDATE,
    .init = accelerate_init,
    .update = accelerate_update,
};

const SystemDesc MOVE_SYSTEM = {
    .name = "move",
    .phase = PHASE_UPDATE,
    .init = move_init,
    .update = move_update,
};
