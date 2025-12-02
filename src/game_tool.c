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


void pos_queue_init(PositionQueue *q, int init_cap)
{
    q->capacity = init_cap;
    q->data = malloc(sizeof(Position) * q->capacity);

    if (!q->data)
    {
        perror("malloc failed");
        exit(1);
    }

    q->front = 0;
    q->rear = 0;
    q->size = 0;
}

int pos_queue_is_empty(PositionQueue *q)
{
    return q->size == 0;
}


void pos_queue_expand(PositionQueue *q)
{
    int new_cap = q->capacity * 2;
    Position *new_data = malloc(sizeof(Position) * new_cap);

    if (!new_data)
    {
        perror("malloc failed");
        exit(1);
    }

    // 重新按顺序复制
    for (int i = 0; i < q->size; ++i)
    {
        new_data[i] = q->data[(q->front + i) % q->capacity];
    }

    free(q->data);
    q->data = new_data;
    q->capacity = new_cap;

    q->front = 0;
    q->rear = q->size;
}

void pos_queue_push(PositionQueue *q, Position p)
{
    // 满了则扩容
    if (q->size == q->capacity)
    {
        pos_queue_expand(q);
    }

    q->data[q->rear] = p;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

Position pos_queue_pop(PositionQueue *q)
{
    if (q->size == 0)
    {
        fprintf(stderr, "Queue is empty\n");
        exit(1);
    }

    Position p = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return p;
}


void pos_queue_free(PositionQueue *q)
{
    free(q->data);
    q->data = NULL;
    q->capacity = 0;
    q->size = 0;
    q->front = 0;
    q->rear = 0;
}

int can_walk(Game *game, int x, int y)
{
    enum ElementType e = game->field.map[y][x].element_type;
    return e == ELE_FLOOR || e == ELE_TARGET;
}





