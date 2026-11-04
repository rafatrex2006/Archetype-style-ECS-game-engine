#ifndef cmd_array_H
#define cmd_array_H

#include <stddef.h>
#include <stdbool.h>
#include "data_structures/components.h"  //entity, Component, World

typedef struct cmd_command{
    int op; //operation
    entity e;
    Component comp;
    size_t data_offset;
    bool has_data;
} cmd_command;

typedef struct cmd_array{
    cmd_command *commands;
    size_t n_count;
    size_t n_size;
    unsigned char *data;
    size_t dn_count;
    size_t dn_size;
} cmd_array;



cmd_array *CreateCMDArray(int size);

void freeCMDArray(cmd_array *arr);

//data is copied, NULL = add with the constructor's default values
void AddCMDToArray(cmd_array *arr, cmd_command cmd, const void *data, size_t data_size);

void CMDFlush(World *W);

#endif