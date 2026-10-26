#ifndef Render_Simple_H
#define Render_Simple_H

#include <SDL3/SDL.h>

typedef enum ShapeType {
    SHAPE_RECT,
    SHAPE_LINE,
    POINT

} ShapeType;

typedef struct Shape {
    ShapeType type;
    SDL_FColor color;
    bool filled;
    union {
        struct { float w, h; } rect;
        struct { SDL_FPoint end; } line;   // start = Transform, end is an offset from it
    };
} Shape;

void createShape(void *dst);

#endif
