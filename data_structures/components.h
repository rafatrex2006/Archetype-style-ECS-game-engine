#ifndef components_H
#define components_H


#include <stdlib.h>
#include <stdint.h>
#include "metadata/metadata.h"  //Component enum

//forward declarations
typedef unsigned long long int signature;
typedef struct archetype archetype;
typedef struct World World;

//handle to an entity
typedef struct entity{
    uint32_t index;     //slot in the World's entity table
    uint32_t gen;       //handle's life
}entity;

//never a valid entity, used for "no entity"
#define NULL_ENTITY ((entity){0, 0})

typedef struct ComponentColumn
{
    void *data;
    size_t DataSize;
} ComponentColumn;


#endif