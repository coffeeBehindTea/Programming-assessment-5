#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#ifndef GAME_INF_H
#define GAME_INF_H
/* --------------------------- basic informations --------------------------- */
#define BOARD_ROWS 20
#define BOARD_COLS 75
#define MAX_NAME 20
/* --------------------------- char of map element -------------------------- */
#define CHAR_HEART '♥'
#define CHAR_FLOOR ' '
#define CHAR_WALL '#'
#define CHAR_TRAP 'X'
/* ------------------------ char of target and player ----------------------- */
#define CHAR_TARGET 'O'
#define CHAR_BODY '@'
#define CHAR_UP '^'
#define CHAR_RIGHT '>'
#define CHAR_DOWN 'v'
#define CHAR_LEFT '<'
/* ------------------------- collections of elements ------------------------ */
extern char HEAD_ARROWS[4]; // head of the robot
extern int pos_shift[4][2]; // shift of y and x coordinate
extern char MAP_ELEMENTS[6]; // char to show of each element type



enum ElementType {
    ELE_FLOOR=1, // this value is only used to check the type of grid, not for any drawing.
    ELE_WALL,
    ELE_TARGET,
    ELE_TRAP,
};

enum PlayerDirection {
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

enum ColorPairID {
    // CP_FLOOR = 1,
    CP_PLAYER=1, // id 0 is reserved id so start from 1
    CP_WALL,
    CP_TARGET,
    CP_TRAP,
    CP_HEART,

    COLOR_PAIR_COUNT
};

typedef struct
{
    int x;
    int y;
} Position;

typedef struct {
    Position position;
    enum ElementType element_type;
} MapElement;

typedef struct
{
    int rows;
    int cols;
    MapElement map[BOARD_ROWS][BOARD_COLS];  // the map list, store walls, floors, traps, and target.
    WINDOW *window;
} Field;

typedef struct
{
    enum PlayerDirection facing_direction;
    Position position;
    int hp;
} Player;

typedef struct
{
    Field field;
    char name[MAX_NAME];
    int score;
    int heart_left;
    int level;
} Game;

enum GameStatus{
    GAME_NOT_START, // map is initialized, waiting for player to enter key
    GAME_PLAYING, // player playing
    GAME_ROUND_FINISHED, // player saved a target, init map needed
    GAME_FINISHED, // player hit wall or trap, init player's position, do not re generate map
    GAME_OVER // player run out of hearts.
};



#endif