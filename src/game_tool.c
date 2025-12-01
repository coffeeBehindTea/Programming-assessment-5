#include "game_tool.h"
// #include <stdlib.h>


int random_range(int left, int right)
{
    return left + rand() % (right - left + 1);
}

void pos_list_init(PosList *arr)
{
    arr->size = 0;
    arr->capacity = 8;   // 初始容量，可以随意改
    arr->data = malloc(arr->capacity * sizeof(Position));

    if (!arr->data)
    {
        perror("malloc failed");
        exit(1);
    }
}

void pos_list_expand(PosList *arr)
{
    arr->capacity *= 2;
    Position *new_data = realloc(arr->data, arr->capacity * sizeof(Position));

    if (!new_data)
    {
        perror("realloc failed");
        exit(1);
    }

    arr->data = new_data;
}

void pos_list_push(PosList *arr, Position p)
{
    if (arr->size == arr->capacity)
    {
        pos_list_expand(arr);
    }

    arr->data[arr->size++] = p;
}

void pos_list_free(PosList *arr)
{
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

