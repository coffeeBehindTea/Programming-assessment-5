#include <ncurses.h>
#include <string.h>
#include "game_inf.h"
#include "game_tool.h"

// --------- Global Variables --------
int YMAX;
int XMAX;
Player player;
PosList trap_list; // stores all traps in the map
PositionQueue path;
int ai_enabled = 0;
int playing = 1;                              // set to 0 to break the main game loop
enum GameStatus game_status = GAME_NOT_START; // set 0 to pause the game(not moving until player entered a key)

/* -------------------------------------------------------------------------- */
/*                         declar function prototypes                         */
/* -------------------------------------------------------------------------- */
WINDOW *init_game(Game *game);
/* ---------------------------------- utls ---------------------------------- */
int check_around(Game *game, int x, int y, int r, enum ElementType element);
int check_pos(Game *game, int x, int y, enum ElementType element);
int is_near_existing_trap(int x, int y, int r);
int bfs_find_path(Game *game, PositionQueue *path);
/* -------------------------------- game gen -------------------------------- */
void init_map(Game *game);
void gen_trap(Game *game);
void gen_wall(Game *game);
void gen_people(Game *game);
/* --------------------------- interaction control -------------------------- */
void respawn_player();
void player_hit_wall(Game *game);
void handle_input(Game *game);
void move_robot(Game *game);
void move_result(Game *game);
enum PlayerDirection direction_from_to(Position from, Position to);
void ai_start(Game *game);
void ai_control(Game *game);
/* ---------------------------- drawing elements ---------------------------- */
void draw_border(Game *game);
void draw_player(Game *game);
void draw_map(Game *game);

int main(void)
{
    /* --------------------------- Initialise ncurses --------------------------- */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    /* ----------------------------- Initialise Game ---------------------------- */
    Game game;
    memset(&game, 0, sizeof(Game));
    init_game(&game); // sets field size, creates WINDOW, draws box
    nodelay(game.field.window, TRUE);
    keypad(game.field.window, TRUE);
    draw_border(&game); // draws the border directly on stdscr too

    mvprintw(1, 2, "Draw Arena. Press any key to quit.");

    init_map(&game);
    while (playing)
    {
        /* ---------------------------- game flow control --------------------------- */
        if (game_status == GAME_NOT_START || game_status == GAME_ROUND_FINISHED) // before each round
        {
            init_map(&game);
        }
        if (game_status == GAME_OVER)
        {
            playing = 0;
        }

        /* ----------------- draw the map and get the player's input ---------------- */
        draw_map(&game);
        draw_player(&game);
        handle_input(&game);

        /* ---------------------------- control the robot --------------------------- */

        if (game_status == GAME_PLAYING)
        {
            if (ai_enabled)
            {
                ai_control(&game);
            }
            move_robot(&game);
            move_result(&game);
        }
        refresh();
        wrefresh(game.field.window);

        // Wait for a key press

        werase(game.field.window);
        usleep(100000); // 100ms
    }

    if (game.field.window != NULL)
    {
        delwin(game.field.window);
    }
    endwin();
    return 0;
}

// Create centered game window and draw a box around it
WINDOW *init_game(Game *game)
{
    /* ----------------------- get the size of the window ----------------------- */
    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);
    YMAX = ymax;
    XMAX = xmax;

    int start_y = (ymax - BOARD_ROWS) / 2;
    int start_x = (xmax - BOARD_COLS) / 2;

    /* --------------------------- initialize the game -------------------------- */
    game->field.rows = BOARD_ROWS;
    game->field.cols = BOARD_COLS;
    game->heart_left = 4;
    game->score = 0;
    game->level = 1;

    game->field.window = newwin(BOARD_ROWS, BOARD_COLS, start_y, start_x);

    /* -------------------- initialize all blocks into floor. ------------------- */
    // for (int y = 0; y < BOARD_ROWS; ++y)
    // {
    //     for (int x = 0; x < BOARD_COLS; ++x)
    //     {
    //         game->field.map[y][x].position.x = x;
    //         game->field.map[y][x].position.y = y;
    //         game->field.map[y][x].element_type = ELE_FLOOR;
    //     }
    // }

    // Simple border inside the window
    // box(game->field.window, 0, 0);

    refresh();
    wrefresh(game->field.window);

    /* --------------------------- define color pairs --------------------------- */
    // init_pair(CP_FLOOR, -1, -1);
    init_pair(CP_HEART, COLOR_GREEN, COLOR_BLACK);
    init_pair(CP_PLAYER, COLOR_WHITE, COLOR_BLUE);
    init_pair(CP_WALL, COLOR_CYAN, COLOR_BLACK);
    init_pair(CP_TRAP, COLOR_RED, COLOR_BLACK);
    init_pair(CP_TARGET, COLOR_WHITE, COLOR_MAGENTA);

    /* ---------------------------- initialize player --------------------------- */
    player.facing_direction = RIGHT;
    player.position.x = 10;
    player.position.y = 10;

    return game->field.window;
}

// set all grid into floor, generate the wall, trap, and target
void init_map(Game *game)
{
    /* ------------------------- set all grid into floor ------------------------ */
    for (int y = 0; y < BOARD_ROWS; ++y)
    {
        for (int x = 0; x < BOARD_COLS; ++x)
        {
            game->field.map[y][x].position.x = x;
            game->field.map[y][x].position.y = y;
            game->field.map[y][x].element_type = ELE_FLOOR;
        }
    }

    player.position.x = 10;
    player.position.y = 10;
    player.facing_direction = RIGHT;

    gen_wall(game);
    gen_trap(game);
    gen_people(game);
    game_status = GAME_ROUND_READY;

    if (ai_enabled) // also init ai when new round started
    {
        ai_start(game);
        game_status = GAME_PLAYING;
    }
}

// respawn the player, do not regenerate map
void respawn_player()
{
    player.position.x = 10;
    player.position.y = 10;
    player.facing_direction = RIGHT;
    game_status = GAME_NOT_START;
}

// handle when player hit wall
void player_hit_wall(Game *game)
{
    game->heart_left -= 1;
    if (game->heart_left)
    {
        respawn_player();
        game_status = GAME_ROUND_READY;
    }
    else
    {
        game_status = GAME_OVER;
    }
}

// Draw border directly on stdscr
void draw_border(Game *game)
{
    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);

    int start_y = (ymax - game->field.rows) / 2 - 1;
    int start_x = (xmax - game->field.cols) / 2 - 1;

    // Horizontal borders
    for (int x = start_x; x <= start_x + game->field.cols; x++)
    {
        mvaddch(start_y, x, ACS_HLINE);
        mvaddch(start_y + game->field.rows + 1, x, ACS_HLINE);
    }

    // Vertical borders
    for (int y = start_y; y <= start_y + game->field.rows; y++)
    {
        mvaddch(y, start_x, ACS_VLINE);
        mvaddch(y, start_x + game->field.cols + 1, ACS_VLINE);
    }

    // Corners
    mvaddch(start_y, start_x, ACS_ULCORNER);
    mvaddch(start_y, start_x + game->field.cols + 1, ACS_URCORNER);
    mvaddch(start_y + game->field.rows + 1, start_x, ACS_LLCORNER);
    mvaddch(start_y + game->field.rows + 1, start_x + game->field.cols + 1, ACS_LRCORNER);
}

// draw the player
void draw_player(Game *game)
{
    wattron(game->field.window, COLOR_PAIR(CP_PLAYER));
    mvwaddch(game->field.window,
             player.position.y,
             player.position.x,
             CHAR_BODY); // draw the body
    mvwaddch(
        game->field.window,
        player.position.y + pos_shift[player.facing_direction][0],
        player.position.x + pos_shift[player.facing_direction][1],
        HEAD_ARROWS[player.facing_direction]); // draw the head

    wattroff(game->field.window, COLOR_PAIR(CP_PLAYER));
}

// draw the cross wall in the center
void draw_map(Game *game)
{
    for (int y = 0; y < game->field.rows; ++y)
    {
        for (int x = 0; x < game->field.cols; ++x) // double loop to draw every grid
        {
            // get the type of current grid
            MapElement grid = game->field.map[y][x];
            // if not floor then draw it
            if (grid.element_type != ELE_FLOOR)
            {
                wattron(game->field.window, COLOR_PAIR(grid.element_type));
                mvwaddch(game->field.window,
                         y,
                         x,
                         MAP_ELEMENTS[grid.element_type]);
                wattroff(game->field.window, COLOR_PAIR(grid.element_type));
            }
        }
    }
}

// generate the traps
void gen_trap(Game *game)
{
    int num = random_range(5, 10); // number of traps

    pos_list_init(&trap_list);
    Position current_pos = {player.position.x, player.position.y};

    // the first "trap" is player's initial position. so trap will not be generated on the player or block the way
    pos_list_push(&trap_list, current_pos);

    while (num)
    {
        /* ------------------------ choose a random position ------------------------ */
        int y = random_range(1, game->field.rows - 1);
        int x = random_range(1, game->field.cols - 1);

        while (is_near_existing_trap(x, y, 2)) // check if there's no other trap close to it
        {
            y = random_range(1, game->field.rows - 1);
            x = random_range(1, game->field.cols - 1);
        }
        /* ------------------------- add the trap into list ------------------------- */
        current_pos.x = x;
        current_pos.y = y;
        pos_list_push(&trap_list, current_pos);
        /* ------------------------- add the trap to the map ------------------------ */
        game->field.map[y][x].element_type = ELE_TRAP;

        // minus one count
        num--;
    }
    pos_list_free(&trap_list);
}

// check if there's a trap near.
int is_near_existing_trap(int x, int y, int r)
{
    for (int i = 0; i < trap_list.size; ++i)
    {
        if (trap_list.data[i].x == 0)
            continue;

        int dx = abs(trap_list.data[i].x - x);
        int dy = abs(trap_list.data[i].y - y);

        if (dx <= r && dy <= r)
            return 1;
    }
    return 0;
}

// check if char c appears in square from [x-r,y-r] to [x+r,y+r]. return 1 if found, 0 if not found.
int check_around(Game *game, int x, int y, int r, enum ElementType element)
{
    int row = game->field.rows;
    int col = game->field.cols;
    for (int i = MAX(y - r, 0); i <= MIN(x + r, row - 1); ++i) // double for loop to check every position in the square, also includes border check
    {
        for (int j = MAX(x - r, 0); j <= MIN(y + r, col - 1); ++j)
        {
            if (game->field.map[i][j].element_type == element) // if that char appeared
            {
                return 1;
            }
        }
    }
    return 0;
}

// check the position's element type
int check_pos(Game *game, int x, int y, enum ElementType element)
{
    return game->field.map[y][x].element_type == element ? 1 : 0;
}

// spawn people that robot need to save.
void gen_people(Game *game)
{
    // same logic as generating trap
    int x = random_range(1, game->field.cols - 1);
    int y = random_range(1, game->field.rows - 1);

    while (check_around(game, x, y, 1, ELE_TRAP) || check_around(game, x, y, 1, ELE_WALL) || abs(player.position.x - x) < 5 || abs(player.position.y - y) < 5) // check if there's no other trap close to it
    {
        x = random_range(1, game->field.cols - 1);
        y = random_range(1, game->field.rows - 1);
    }
    game->field.map[y][x].element_type = ELE_TARGET;
}

// generate the cross-shaped wall
void gen_wall(Game *game)
{
    int startx = game->field.cols / 2;
    int starty = game->field.rows / 2;

    // draw the horizontal wall
    for (int x = startx - 4; x <= startx + 5; ++x)
    {
        game->field.map[starty][x].element_type = ELE_WALL;
    }
    // draw the vertical wall
    for (int y = starty - 4; y <= starty + 5; ++y)
    {
        game->field.map[y][startx].element_type = ELE_WALL;
    }
}

// handle user's input
void handle_input(Game *game)
{
    int ch = wgetch(game->field.window);

    switch (ch)
    {
        /* ----------------------------- game flow part ----------------------------- */
    case 'q':
        playing = 0; // quit the game
        break;
    case 'm':
        ai_enabled = ai_enabled ^ 1;
        if (ai_enabled)
        {
            ai_start(game); // find the path and take over the control.
            game_status = GAME_PLAYING;
        }
        break;
        /* ------------------------------ movement part ----------------------------- */
    case KEY_UP:
        if (!ai_enabled)
        {
            player.facing_direction = UP;
            game_status = GAME_PLAYING;
            break;
        }
    case KEY_RIGHT:
        if (!ai_enabled)
        {
            player.facing_direction = RIGHT;
            game_status = GAME_PLAYING;
            break;
        }
    case KEY_DOWN:
        if (!ai_enabled)
        {
            player.facing_direction = DOWN;
            game_status = GAME_PLAYING;
            break;
        }
    case KEY_LEFT:
        if (!ai_enabled)
        {
            player.facing_direction = LEFT;
            game_status = GAME_PLAYING;
            break;
        }

    default:
        break;
    }
}

// move the player
void move_robot(Game *game)
{
    // calculate the new position
    int newy = CLAMP(player.position.y + pos_shift[player.facing_direction][0], 0, game->field.rows - 1);
    int newx = CLAMP(player.position.x + pos_shift[player.facing_direction][1], 0, game->field.cols - 1);

    // just move the player
    player.position.x = newx;
    player.position.y = newy;
}

// check if player still alive or saved a target
void move_result(Game *game)
{
    Position pos = player.position;
    // if still alive
    if (check_pos(game, pos.x, pos.y, ELE_FLOOR))
    {
        return;
    }
    // hit wall
    if (check_pos(game, pos.x, pos.y, ELE_WALL) || check_pos(game, pos.x, pos.y, ELE_TRAP))
    {
        player_hit_wall(game);
    }
    if (check_pos(game, pos.x, pos.y, ELE_TARGET))
    {
        game_status = GAME_ROUND_FINISHED;
    }
}

int bfs_find_path(Game *game, PositionQueue *path)
{
    int rows = game->field.rows;
    int cols = game->field.cols;

    int visited[BOARD_ROWS][BOARD_COLS] = {0};
    Position prev[BOARD_ROWS][BOARD_COLS];

    PositionQueue q;
    pos_queue_init(&q, 64);

    Position start = player.position;
    pos_queue_push(&q, start);
    visited[start.y][start.x] = 1;

    Position target = {-1, -1};

    // 方向偏移
    int dx[4] = {0, 1, 0, -1};
    int dy[4] = {-1, 0, 1, 0};

    while (!pos_queue_is_empty(&q))
    {
        Position cur = pos_queue_pop(&q);

        // 若找到目标
        if (game->field.map[cur.y][cur.x].element_type == ELE_TARGET)
        {
            target = cur;
            break;
        }

        // 扩散
        for (int d = 0; d < 4; ++d)
        {
            int nx = cur.x + dx[d];
            int ny = cur.y + dy[d];

            if (nx < 0 || ny < 0 || nx >= cols || ny >= rows)
                continue;

            if (visited[ny][nx])
                continue;

            if (!can_walk(game, nx, ny))
                continue;

            visited[ny][nx] = 1;
            prev[ny][nx] = cur;

            pos_queue_push(&q, (Position){nx, ny});
        }
    }

    pos_queue_free(&q);

    if (target.x == -1)
        return 0; // 没找到路


    // 正向存入 path
    Position stack[BOARD_ROWS * BOARD_COLS];
    int len = 0;

    Position cur = target;

    // 从 target 回溯到 start
    while (!(cur.x == start.x && cur.y == start.y))
    {
        stack[len++] = cur;
        cur = prev[cur.y][cur.x];
    }

    // 反向入队列：start → target
    pos_queue_init(path, len);

    for (int i = len - 1; i >= 0; --i)
    {
        pos_queue_push(path, stack[i]);
    }

    return 1;
}

enum PlayerDirection direction_from_to(Position from, Position to)
{
    if (to.x == from.x && to.y == from.y - 1)
        return UP;
    if (to.x == from.x && to.y == from.y + 1)
        return DOWN;
    if (to.x == from.x - 1 && to.y == from.y)
        return LEFT;
    if (to.x == from.x + 1 && to.y == from.y)
        return RIGHT;

    return player.facing_direction; // fallback
}

void ai_start(Game *game)
{
    pos_queue_free(&path);
    bfs_find_path(game, &path);
}

void ai_control(Game *game)
{
    if (pos_queue_is_empty(&path))
        return;

    Position next = pos_queue_pop(&path);

    player.facing_direction =
        direction_from_to(player.position, next);
}
