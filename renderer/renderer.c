#include "renderer.h"
#include "components/transform.h"
#include "components/render_simple.h"
#include "components/render_surface.h"

//column index = number of component bits below this one
#define COLUMN(arc, comp) ((arc)->data[__builtin_popcountll((arc)->sig & ((1ULL << (comp)) - 1))].data)

bool FRX_CreateRenderer(World *W, void **state){

    SDL_Renderer *renderer = SDL_CreateRenderer(W->window, NULL);

    //main owns the window, so only report the error here
    if (!renderer) {
        SDL_Log("Renderer failed: %s", SDL_GetError());
        return false;
    }

    FRX_renderer *rd = (FRX_renderer*)malloc(sizeof(FRX_renderer));
    if (!rd) {
        SDL_DestroyRenderer(renderer);
        return false;
    }
    rd->window = W->window;
    rd->renderer = renderer;
    rd->query[QUERY_SHAPE] = createCustomQueue(W, (Component[]){TRANSFORM, SHAPE}, 2, NULL, 0);
    rd->query[QUERY_SURFACE] = createCustomQueue(W, (Component[]){TRANSFORM, SURFACE}, 2, NULL, 0);

    *state = rd;
    return true;
}

void FRX_DestroyRenderer(World *W, void *state){
    FRX_renderer *rd = (FRX_renderer *)state;
    if (!rd) {
        return;
    }
    // Cleanup, this also destroys every texture made with this renderer
    SDL_DestroyRenderer(rd->renderer);
    free(rd);
}

static void RenderShapes(FRX_renderer *rd){
    archetype_array *query = rd->query[QUERY_SHAPE];

    for(int i = 0; i < query->n; i++){
        archetype *arc = query->arcs[i];
        Transform *transforms = (Transform*)COLUMN(arc, TRANSFORM);
        Shape *shapes = (Shape*)COLUMN(arc, SHAPE);

        for(int j = 0; j < arc->entityCount; j++){
            Transform *t = &transforms[j];
            Shape *s = &shapes[j];
            SDL_SetRenderDrawColorFloat(rd->renderer, s->color.r, s->color.g, s->color.b, s->color.a);

            switch(s->type){
                case SHAPE_RECT: {
                    SDL_FRect rect = {t->x, t->y, s->rect.w, s->rect.h};
                    if(s->filled){
                        SDL_RenderFillRect(rd->renderer, &rect);
                    }
                    else{
                        SDL_RenderRect(rd->renderer, &rect);
                    }
                    break;
                }
                case SHAPE_LINE:
                    SDL_RenderLine(rd->renderer, t->x, t->y, t->x + s->line.end.x, t->y + s->line.end.y);
                    break;
                case POINT:
                    SDL_RenderPoint(rd->renderer, t->x, t->y);
                    break;
            }
        }
    }
}

static void RenderSurfaces(FRX_renderer *rd){
    archetype_array *query = rd->query[QUERY_SURFACE];

    for(int i = 0; i < query->n; i++){
        archetype *arc = query->arcs[i];
        Transform *transforms = (Transform*)COLUMN(arc, TRANSFORM);
        Surface *surfaces = (Surface*)COLUMN(arc, SURFACE);

        for(int j = 0; j < arc->entityCount; j++){
            Transform *t = &transforms[j];
            Surface *s = &surfaces[j];
            if(!s->surface){
                continue;
            }

            //upload to the gpu once, then reuse the texture every frame
            if(!s->texture){
                s->texture = SDL_CreateTextureFromSurface(rd->renderer, s->surface);
                if(!s->texture){
                    SDL_Log("Texture creation failed: %s", SDL_GetError());
                    continue;
                }
            }

            SDL_FRect dst = {t->x, t->y, (float)s->surface->w, (float)s->surface->h};
            SDL_RenderTexture(rd->renderer, s->texture, NULL, &dst);
        }
    }
}

void FRX_UpdateRenderer(World *W, float dt, void *state){
    FRX_renderer *rd = (FRX_renderer *)state;
    if (!rd) {
        return;
    }

    SDL_SetRenderDrawColorFloat(rd->renderer, 0.0f, 0.0f, 0.0f, 1.0f);
    SDL_RenderClear(rd->renderer);

    RenderShapes(rd);
    RenderSurfaces(rd);   //after shapes so sprites are drawn on top

    SDL_RenderPresent(rd->renderer);
}
