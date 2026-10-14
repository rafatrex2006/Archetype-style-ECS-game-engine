#include "health.h"

#include <stdlib.h>


void createHealth(void *dst){
    Health *h = (Health*)dst;
    h->hp = 100;
}

