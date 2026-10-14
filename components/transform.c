#include "transform.h"

#include <stdlib.h>


void createTransform(void *dst){
    Transform *t = (Transform*)dst;
    t->x = 0.0f;
    t->y = 0.0f;
}