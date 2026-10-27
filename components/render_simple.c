#include "render_simple.h"
#include <stdbool.h>
#include <stdlib.h>

void createShape(void *dst){
    Shape *S = (Shape*)dst;
    *S = (Shape){
    .type = SHAPE_RECT,
    .color = { 0.5f, 0.5f, 0.5f, 1.0f },   // braces are fine here: this part is an initializer
    .filled = false,
    .rect = { .w = 0.0f, .h = 0.0f },
    };
}
