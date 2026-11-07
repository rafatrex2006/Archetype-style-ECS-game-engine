#include "event_bus.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "data_structures/World.h"

EventBus *createEventBus(const metadata *M){
    EventBus *B = (EventBus*)calloc(1, sizeof(EventBus));   //all buffers NULL, counts and numbers 0
    for(int i = 0; i < EVENT_COUNT; i++){
        B->channels[i].elem_size = M->event_sizes[i];
        if(M->event_sizes[i] == 0){
            fprintf(stderr, "createEventBus: event type %d was not registered in createMetadata, it can't be sent\n", i);
        }
    }
    return B;
}

void freeEventBus(EventBus *B){
    if(B == NULL){
        return;
    }
    for(int i = 0; i < EVENT_COUNT; i++){
        free(B->channels[i].buf[0]);
        free(B->channels[i].buf[1]);
    }
    free(B);
}

void event_bus_swap(EventBus *B){
    for(int i = 0; i < EVENT_COUNT; i++){
        EventChannel *ch = &B->channels[i];
        //last frame's buffer is emptied and becomes this frame's
        int old = 1 - ch->cur;
        ch->count[old] = 0;
        ch->start[old] = ch->total;
        ch->cur = old;
    }
}

static EventChannel *GetChannel(World *W, EventType type, const char *who){
    if(type < 0 || type >= EVENT_COUNT || W->systems->events->channels[type].elem_size == 0){
        fprintf(stderr, "%s: invalid event type %d\n", who, (int)type);
        return NULL;
    }
    return &W->systems->events->channels[type];
}

void emit_event(World *W, EventType type, const void *event){
    EventChannel *ch = GetChannel(W, type, "emit_event");
    if(ch == NULL){
        return;
    }
    int b = ch->cur;
    if(ch->count[b] == ch->cap[b]){
        //buffers are only allocated once their type is actually sent
        ch->cap[b] = ch->cap[b] ? ch->cap[b] * 2 : 16;
        ch->buf[b] = realloc(ch->buf[b], ch->cap[b] * ch->elem_size);
    }
    unsigned char *dst = ch->buf[b] + ch->count[b] * ch->elem_size;
    if(event){
        memcpy(dst, event, ch->elem_size);
    }
    else{
        memset(dst, 0, ch->elem_size);
    }
    ch->count[b]++;
    ch->total++;
}

EventReader event_reader(World *W, EventType type){
    EventChannel *ch = GetChannel(W, type, "event_reader");
    return (EventReader){ ch ? ch->total : 0 };
}

const void *read_event(World *W, EventType type, EventReader *r){
    EventChannel *ch = GetChannel(W, type, "read_event");
    if(ch == NULL){
        return NULL;
    }
    int last = 1 - ch->cur;

    //the reader didn't read for 2+ frames, the events it missed are gone
    if(r->next < ch->start[last]){
        r->next = ch->start[last];
    }
    if(r->next >= ch->total){
        return NULL;
    }

    //start[cur] == start[last] + count[last], so anything before start[cur] is in last frame's buffer
    int b = (r->next < ch->start[ch->cur]) ? last : ch->cur;
    void *e = ch->buf[b] + (size_t)(r->next - ch->start[b]) * ch->elem_size;
    r->next++;
    return e;
}
