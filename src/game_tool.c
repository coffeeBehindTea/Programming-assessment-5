#include "game_tool.h"
// #include <stdlib.h>


int random_range(int left, int right)
{
    return left + rand() % (right - left + 1);
}