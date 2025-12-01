#include <stdlib.h>
#include <ncurses.h>
#include "game_inf.h"

#ifndef GAME_TOOL_H
#define GAME_TOOL_H

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CLAMP(v,l,h) (MAX((l), MIN((v),(h))))

typedef struct {
    Position *data;
    int size;
    int capacity; 
} PosList;


// function that generate a random integer in [left,right]
int random_range(int left, int right);

// function initialize the variable size list
void pos_list_init(PosList *arr);

// funciton that expand the size of list
void pos_list_expand(PosList *arr);

// function that push element into variable size list
void pos_list_push(PosList *arr, Position p);

// free the memory
void pos_list_free(PosList *arr);

#endif