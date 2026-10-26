#include "system_handler.h"

#include <stdlib.h>
#include <stdio.h>

SystemRegistry *createSystemRegistry(World *W, int size){
    if(size < 1){
        size = 1;
    }
    SystemRegistry *R = (SystemRegistry*)malloc(sizeof(SystemRegistry));
    R->W = W;
    R->systems = (System*)malloc(size*sizeof(System));
    R->size = size;
    R->n = 0;
    R->initialized = false;
    return R;
}

void freeSystemRegistry(SystemRegistry *R){
    if(R == NULL){
        return;
    }
    //reverse order so later systems can still rely on earlier ones while cleaning up
    for(int i = R->n - 1; i >= 0; i--){
        System *s = &R->systems[i];
        if(s->desc.destroy){
            s->desc.destroy(R->W, s->state);
        }
        else{
            free(s->state);
        }
    }
    free(R->systems);
    free(R);
}

static void DoubleSystemSize(SystemRegistry *R){
    R->size *= 2;
    R->systems = realloc(R->systems, R->size*sizeof(System));
}

static void InitSystem(SystemRegistry *R, System *s){
    s->state = s->desc.init ? s->desc.init(R->W) : NULL;
}

int register_system(SystemRegistry *R, const SystemDesc *desc){
    if(desc == NULL || desc->update == NULL || desc->phase < 0 || desc->phase >= PHASE_COUNT){
        fprintf(stderr, "register_system: invalid system '%s'\n", desc && desc->name ? desc->name : "?");
        return -1;
    }

    if(R->n == R->size){
        DoubleSystemSize(R);
    }
    System *s = &R->systems[R->n];
    s->desc = *desc;
    s->state = NULL;
    s->enabled = true;

    //registered after systems_init already ran, so init it right away
    if(R->initialized){
        InitSystem(R, s);
    }

    return R->n++;
}

void systems_init(SystemRegistry *R){
    if(R->initialized){
        return;
    }
    for(int i = 0; i < R->n; i++){
        InitSystem(R, &R->systems[i]);
    }
    R->initialized = true;
}

void systems_run(SystemRegistry *R, float dt){
    for(int phase = 0; phase < PHASE_COUNT; phase++){
        for(int i = 0; i < R->n; i++){
            System *s = &R->systems[i];
            if((int)s->desc.phase == phase && s->enabled){
                s->desc.update(R->W, dt, s->state);
            }
        }
    }
}

void set_system_enabled(SystemRegistry *R, int id, bool enabled){
    if(id < 0 || id >= R->n){
        return;
    }
    R->systems[id].enabled = enabled;
}
