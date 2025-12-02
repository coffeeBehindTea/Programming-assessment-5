#include <stdlib.h>
#include <ncurses.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>
#include "game_inf.h"

#ifndef GAME_TOOL_H
#define GAME_TOOL_H

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CLAMP(v,l,h) (MAX((l), MIN((v),(h))))


typedef struct {
    char name[MAX_NAME];
    int level;
} Record;

// ensure the folder exist
void ensure_save_folder();

void get_save_path(const char *player, char *out);

int load_player_record(const char *player);

void save_player_record(const char *player, int new_level);

int load_all_records(Record *list, int max);

int cmp_record(const void *a, const void *b);



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