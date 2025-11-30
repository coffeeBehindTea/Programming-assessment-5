#include <stdlib.h>
#include <ncurses.h>
#include "game_inf.h"

#ifndef GAME_TOOL_H
#define GAME_TOOL_H

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CLAMP(v,l,h) (MAX((l), MIN((v),(h))))


// function that generate a random integer in [left,right]
int random_range(int left, int right);




#endif