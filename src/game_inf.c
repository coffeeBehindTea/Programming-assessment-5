#include "game_inf.h"

char HEAD_ARROWS[4] = {CHAR_UP, CHAR_RIGHT, CHAR_DOWN, CHAR_LEFT}; // head of the robot
int pos_shift[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};          // shift of y and x coordinate
char MAP_ELEMENTS[6] = {' ',' ',CHAR_WALL, CHAR_TARGET, CHAR_TRAP};