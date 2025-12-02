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



void ensure_save_folder()
{
    struct stat st = {0};

    if (stat("saves", &st) == -1)
    {
        mkdir("saves", 0755); // create folder with full admin
    }
}

void get_save_path(const char *player, char *out)
{
    sprintf(out, "saves/%s.txt", player);
}

int load_player_record(const char *player)
{
    char path[256];
    get_save_path(player, path);

    FILE *f = fopen(path, "r");
    if (!f) return 0;

    int level = 0;
    fscanf(f, "%d", &level);
    fclose(f);

    return level;
}

void save_player_record(const char *player, int new_level)
{
    char path[256];
    get_save_path(player, path);

    int old = load_player_record(player);

    if (new_level <= old) return;

    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "%d", new_level);
    fclose(f);
}

int load_all_records(Record *list, int max)
{
    DIR *dir = opendir("saves");
    if (!dir) return 0;

    struct dirent *ent; // saves
    int count = 0;
    char path[512];

    while ((ent = readdir(dir)) != NULL && count < max)
    {
        // skip "." and ".."
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        // build full path "saves/filename"
        snprintf(path, sizeof(path), "saves/%s", ent->d_name);

        // use stat to check it's a regular file
        struct stat st;
        if (stat(path, &st) == -1)
            continue;
        if (!S_ISREG(st.st_mode))
            continue;

        // only accept .txt files (case-sensitive). skip others.
        size_t len = strlen(ent->d_name);
        if (len < 5) // at least "a.txt"
            continue;
        if (strcmp(ent->d_name + len - 4, ".txt") != 0)
            continue;

        // read the level integer from the file
        FILE *f = fopen(path, "r");
        if (!f) continue;

        int level = 0;
        if (fscanf(f, "%d", &level) != 1)
        {
            fclose(f);
            continue;
        }
        fclose(f);

        // copy name without .txt extension into list[count].name
        strncpy(list[count].name, ent->d_name, MAX_NAME - 1);
        list[count].name[MAX_NAME - 1] = '\0';
        char *dot = strrchr(list[count].name, '.');
        if (dot) *dot = '\0';

        list[count].level = level;
        count++;
    }

    closedir(dir);
    return count;
}

int cmp_record(const void *a, const void *b)
{
    Record *ra = (Record *)a;
    Record *rb = (Record *)b;
    return rb->level - ra->level;
}

