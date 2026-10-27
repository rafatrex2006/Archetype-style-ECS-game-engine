#ifndef components_H
#define components_H


#include <stdlib.h>
#include "metadata/metadata.h"  //Component enum

//forward declarations
typedef unsigned long long int signature;
typedef struct archetype archetype;
typedef struct World World;
typedef struct entity{
    int id;
    archetype *arch;
    int index;
}entity;

typedef struct ComponentColumn
{
    void *data;
    size_t DataSize;
} ComponentColumn;


#endif