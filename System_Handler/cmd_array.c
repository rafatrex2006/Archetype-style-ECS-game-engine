#include "cmd_array.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include "data_structures/World.h"

cmd_array *CreateCMDArray(int size){
    if(size < 1){
        size = 1;
    }
    cmd_array *arr = (cmd_array*)malloc(sizeof(cmd_array));
    arr->commands = (cmd_command*)malloc(size*sizeof(cmd_command));
    arr->data = (unsigned char*)malloc(10*size*sizeof(unsigned char));

    arr->n_size = size;
    arr->n_count = 0;
    arr->dn_size = 10*size;
    arr->dn_count = 0;
    return arr;
}

void freeCMDArray(cmd_array *arr){
    free(arr->commands);
    free(arr->data);
    free(arr);
}


void AddCMDToArray(cmd_array *arr, cmd_command cmd, const void *data, size_t data_size){
    if(arr->n_count == arr->n_size){
        arr->n_size *= 2;
        arr->commands = (cmd_command*)realloc(arr->commands, arr->n_size*sizeof(cmd_command));
    }
    if(data){
        cmd.has_data = true;
        while(arr->dn_count+data_size > arr->dn_size){
            arr->dn_size *= 2;
            arr->data = (unsigned char*)realloc(arr->data, arr->dn_size*sizeof(unsigned char));
        }
        memcpy(arr->data + arr->dn_count, data, data_size);
        cmd.data_offset = arr->dn_count;
        arr->dn_count += data_size;
    } else {
        cmd.has_data = false;
    }
    
    arr->commands[arr->n_count++] = cmd;
}

void CMDFlush(World *W){
    cmd_array *arr = W->command_buffer;
    for(size_t i = 0; i < arr->n_count; i++){
        cmd_command *c = &arr->commands[i];
        void *data = c->has_data ? arr->data + c->data_offset : NULL;
        switch(c->op){
            case 0:     add_component(W, c->e, c->comp, data);  break;
            case 1:     remove_component(W, c->e, c->comp);     break;
            case 2:     DestroyEnt(W, c->e);                    break;
        }
    }
    arr->n_count = 0;
    arr->dn_count = 0;
}

/*
void cmd_add_component(World *W, entity e, Component Comp, void *data){
    AddCMDToArray(W->command_buffer, (cmd_command){.op = 0, .e = e, .comp = Comp}, data, data == NULL ? NULL : W->metadata->sizes[Comp]);
}

void cmd_remove_component(World *W, entity e, Component Comp){
    AddCMDToArray(W->command_buffer, (cmd_command){.op = 1, .e = e, .comp = Comp}, NULL, NULL);
}

void cmd_DestroyEnt(World *W, entity e){
    AddCMDToArray(W->command_buffer, (cmd_command){.op = 2, .e = e, .comp = NULL}, NULL, NULL);
}
*/