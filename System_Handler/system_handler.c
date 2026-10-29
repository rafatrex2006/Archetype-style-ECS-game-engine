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
        if(s->failed){
            continue;   //init already cleaned up after itself
        }
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

//works by index, init may register more systems and realloc R->systems under us
static bool InitSystem(SystemRegistry *R, int id){
    SystemInitFn init = R->systems[id].desc.init;
    void *state = NULL;
    bool ok = init ? init(R->W, &state) : true;

    System *s = &R->systems[id];
    if(!ok){
        fprintf(stderr, "systems_init: system '%s' failed to start, it is disabled\n", s->desc.name ? s->desc.name : "?");
        s->state = NULL;
        s->enabled = false;
        s->failed = true;
        return false;
    }
    s->state = state;
    return true;
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
    s->failed = false;
    int id = R->n++;

    //registered after systems_init already ran, so init it right away
    if(R->initialized && !InitSystem(R, id)){
        return -1;
    }

    return id;
}

bool systems_init(SystemRegistry *R){
    if(R->initialized){
        return true;
    }
    //set first, so systems registered by an init below are initialized by register_system
    R->initialized = true;
    int n = R->n;
    bool ok = true;
    for(int i = 0; i < n; i++){
        if(!InitSystem(R, i)){
            ok = false;
        }
    }
    return ok;
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
    if(id < 0 || id >= R->n || R->systems[id].failed){
        return;
    }
    R->systems[id].enabled = enabled;
}
