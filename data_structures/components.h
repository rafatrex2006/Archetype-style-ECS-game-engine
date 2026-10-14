#ifndef components_H
#define components_H


#include <stdlib.h>

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


typedef enum Component {
    TRANSFORM = 0,
    VELOCITY,
    HEALTH,
    ACCELERATION,

    COMPONENT_COUNT
} Component;



#endif