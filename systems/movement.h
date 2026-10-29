#ifndef Movement_H
#define Movement_H

#include "data_structures/World.h"

//Velocity += Acceleration * dt, runs in PHASE_PRE_UPDATE so velocities are ready before anything moves
bool accelerate_init(World *W, void **state);
void accelerate_update(World *W, float dt, void *state);

//Transform += Velocity * dt, runs in PHASE_UPDATE
bool move_init(World *W, void **state);
void move_update(World *W, float dt, void *state);

//ready to pass to register_system_world
extern const SystemDesc ACCELERATE_SYSTEM;
extern const SystemDesc MOVE_SYSTEM;

#endif
