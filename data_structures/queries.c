#include "queries.h"

#include "World.h"


queries *CreateQueryArr(int size){
    if(size < 1){
        size = 1;
    }
    queries *q = (queries*)malloc(sizeof(queries));
    q->sigs_in = (signature*)malloc(size*sizeof(signature));
    q->sigs_out = (signature*)malloc(size*sizeof(signature));
    q->list = (archetype_array**)malloc(size*sizeof(archetype_array*));
    q->size = size;
    q->n = 0;
    return q;
}

void freeQueryArr(queries *q){
    if(q == NULL){
        return;
    }
    for(int i = 0; i < q->n; i++){
        freeArcArray(q->list[i]);
    }
    free(q->list);
    free(q->sigs_in);
    free(q->sigs_out);
    free(q);
}

void DoubleQuerySize(queries *q){
    q->size *= 2;
    q->list = realloc(q->list, q->size*sizeof(archetype_array*));
    q->sigs_in = realloc(q->sigs_in, q->size*sizeof(signature));
    q->sigs_out = realloc(q->sigs_out, q->size*sizeof(signature));
}

archetype_array *createQuery(World *W, signature sig){
    queries *q = W->QueryArr;
    //search old
    for(int i = 0; i < q->n; i++){
        if ((q->sigs_in[i] == sig) && (q->sigs_out[i] == 0)){
            return q->list[i];
        }
    }

    //create new
    if(q->n == q->size){
        DoubleQuerySize(q);
    }
    archetype_array *arr = CreateArcArray(W->ArcArray->n > 0 ? W->ArcArray->n : 1);
    for(int i = 0; i < W->ArcArray->n; i++){
        if ((W->ArcArray->arcs[i]->sig & sig) == sig){
            AddArcToArray(arr, W->ArcArray->arcs[i]);
        }
    }
    q->sigs_in[q->n] = sig;
    q->sigs_out[q->n] = 0;
    q->list[q->n++] = arr;
    return arr;
}

archetype_array *createCustomQueue(World *W, Component *in, int SizeOfin, Component *out, int SizeOfout){
    signature sig_in = 0;
    signature sig_out = 0;
    for(int i = 0; i < SizeOfin; i++){
        sig_in = sig_in | (1ULL << in[i]);
    }
    for(int i = 0; i < SizeOfout; i++){
        sig_out = sig_out | (1ULL << out[i]);
    }

    //repeated
    queries *q = W->QueryArr;
    //search old
    for(int i = 0; i < q->n; i++){
        if ((q->sigs_in[i] == sig_in) && (q->sigs_out[i] == sig_out)){
            return q->list[i];
        }
    }

    //create new
    if(q->n == q->size){
        DoubleQuerySize(q);
    }
    archetype_array *arr = CreateArcArray(W->ArcArray->n > 0 ? W->ArcArray->n : 1);
    for(int i = 0; i < W->ArcArray->n; i++){
        if (((W->ArcArray->arcs[i]->sig & sig_in) == sig_in) && !(W->ArcArray->arcs[i]->sig & sig_out)){
            AddArcToArray(arr, W->ArcArray->arcs[i]);
        }
    }
    q->sigs_in[q->n] = sig_in;
    q->sigs_out[q->n] = sig_out;
    q->list[q->n++] = arr;
    return arr;
}
