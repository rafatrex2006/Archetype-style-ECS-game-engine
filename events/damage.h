#ifndef Damage_H
#define Damage_H

#include "data_structures/components.h"  //entity

//events are plain data: no owning pointers, they are dropped without any cleanup
//so no constructor or destructor like components have

//"target should lose amount hp"
typedef struct Damage{
    entity target;
    int amount;
} Damage;

#endif
