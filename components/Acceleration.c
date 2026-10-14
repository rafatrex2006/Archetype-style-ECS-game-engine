#include "acceleration.h"

#include <stdlib.h>


void createAcceleration(void *dst){
    Acceleration *a = (Acceleration*)dst;
    a->x = 0.0f;
    a->y = 0.0f;
}
