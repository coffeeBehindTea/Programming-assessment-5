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


typedef struct {
    Position *data;   // 数据存储区
    int capacity;     // 队列容量
    int front;        // 头索引
    int rear;         // 尾索引（指向下一个可写位置）
    int size;         // 当前元素数量
} PositionQueue;

void pos_queue_init(PositionQueue *q, int init_cap);

int pos_queue_is_empty(PositionQueue *q);

void pos_queue_expand(PositionQueue *q);

void pos_queue_push(PositionQueue *q, Position p);

Position pos_queue_pop(PositionQueue *q);

void pos_queue_free(PositionQueue *q);

int can_walk(Game *game, int x, int y);

#endif