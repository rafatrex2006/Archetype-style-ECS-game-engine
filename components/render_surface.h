#ifndef Render_Surface_H
#define Render_Surface_H

#include <SDL3/SDL.h>

typedef struct Surface {
    SDL_Surface *surface;   //not owned, load once, share between entities and free it yourself, NULL = draw nothing
    SDL_Texture *texture;   //made by the renderer on first draw, leave it NULL
} Surface;

void createSurface(void *dst);

#endif
