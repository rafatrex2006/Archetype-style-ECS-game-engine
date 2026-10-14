#include "metadata.h"
#include <stdlib.h>
#include <stdalign.h>

void assign_new_component(metadata *M, size_t size, Component id, size_t alignment, void (*constructor)(void *), void (*destructor)(void *), void (*move)(void *, void *)){
    M->sizes[id] = size;
    M->alignment[id] = alignment;
    M->constructor[id] = constructor;
    M->destructor[id] = destructor;
    M->move[id] = move;
}

void freeMetadata(metadata *meta){
    free(meta->sizes);
    free(meta->alignment);
    free(meta->constructor);
    free(meta->destructor);
    free(meta->move);
    free(meta);
}



//include components
#include "components/transform.h"
#include "components/velocity.h"
#include "components/health.h"
#include "components/acceleration.h"


metadata *createMetadata(int n){
    metadata *M = (metadata*)malloc(sizeof(metadata));
    M->sizes = (size_t*)calloc(n, sizeof(size_t));
    M->alignment = (size_t*)calloc(n, sizeof(size_t));

    //M->constructor =  (M->constructor**)calloc(n, sizeof(*M->constructor));
    //M->destructor = (M->destructor**)calloc(n, sizeof(*M->destructor));
    //M->move = (M->move**)calloc(n, sizeof(*M->move));
    M->constructor =  calloc(n, sizeof(*M->constructor));
    M->destructor = calloc(n, sizeof(*M->destructor));
    M->move = calloc(n, sizeof(*M->move));

    //components -- assign_new_component(metadata *M, size_t size, Component id, size_t alignment, constructor, destructor, move)
    assign_new_component(M, sizeof(Transform), TRANSFORM, alignof(Transform), createTransform, NULL, NULL); //transform
    assign_new_component(M, sizeof(Velocity), VELOCITY, alignof(Velocity), createVelocity, NULL, NULL); //velocity
    assign_new_component(M, sizeof(Health), HEALTH, alignof(Health), createHealth, NULL, NULL);
    assign_new_component(M, sizeof(Acceleration), ACCELERATION, alignof(Acceleration), createAcceleration, NULL, NULL);


    





    return M;
}
