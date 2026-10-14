#include "velocity.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>


void createVelocity(void *dst){
    Velocity *v = (Velocity*)dst;
    v->x = 0.0f;
    v->y = 0.0f;
}
