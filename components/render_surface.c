#include "render_surface.h"
#include <stdlib.h>

void createSurface(void *dst){
    Surface *S = (Surface*)dst;

    *S = (Surface){
    .surface = NULL,
    .texture = NULL
    };
}
