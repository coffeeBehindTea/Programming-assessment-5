#include <ncurses.h>
#include <string.h>
#include "game_inf.h"
#include "game_tool.h"

// --------- Global Variables --------
int YMAX;
int XMAX;
Player player;
Position trap_list[15]; // stores all traps in the map
int manual_mode = 1;    // 1 for manual, 0 for AI control
int playing = 1;        // set to 0 to break the main game loop

/* -------------------------------------------------------------------------- */
/*                         declar function prototypes                         */
/* -------------------------------------------------------------------------- */
WINDOW *init_game(Game *game);
/* ---------------------------------- utls ---------------------------------- */
int check_around(Game *game, int x, int y, int r, enum ElementType element);
int check_pos(Game *game, int x, int y, enum ElementType element);
int is_near_existing_trap(int x, int y, int r);
/* -------------------------------- game gen -------------------------------- */
void gen_trap(Game *game);
void gen_wall(Game *game);
void gen_people(Game *game);
/* --------------------------- interaction control -------------------------- */
void handle_input(Game *game);
void move_robot(Game *game);
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

    /* ----------------------------- Initialise Game ---------------------------- */
    Game game;
    memset(&game, 0, sizeof(Game));
    init_game(&game);   // sets field size, creates WINDOW, draws box
    draw_border(&game); // draws the border directly on stdscr too

    mvprintw(1, 2, "Draw Arena. Press any key to quit.");

    gen_trap(&game);
    gen_wall(&game);
    gen_people(&game);
    for (int i = 0; i < 3; ++i)
    {
        draw_map(&game);
        draw_player(&game);
        refresh();
        wrefresh(game.field.window);

        // Wait for a key press
        getch();
        player.position.y += 1;
        werase(game.field.window);
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

    game->field.rows = BOARD_ROWS;
    game->field.cols = BOARD_COLS;

    game->field.window = newwin(BOARD_ROWS, BOARD_COLS, start_y, start_x);

    /* -------------------- initialize all blocks into floor. ------------------- */
    for (int y = 0; y < BOARD_ROWS; ++y)
    {
        for (int x = 0; x < BOARD_COLS; ++x)
        {
            game->field.map[y][x].position.x = x;
            game->field.map[y][x].position.y = y;
            game->field.map[y][x].element_type = ELE_FLOOR;
        }
    }

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
             '@'); // draw the body
    mvwaddch(
        game->field.window,
        player.position.y + pos_shift[player.facing_direction][0],
        player.position.x + pos_shift[player.facing_direction][1], HEAD_ARROWS[player.facing_direction]); // draw the head

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
    // the first "trap" is player's initial position. so trap will not be generated on the player or block the way
    trap_list[0].x = player.position.x;
    trap_list[0].y = player.position.y;

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
        trap_list[num].x = x;
        trap_list[num].y = y;
        /* ------------------------- add the trap to the map ------------------------ */
        game->field.map[y][x].element_type = ELE_TRAP;

        // minus one count
        num--;
    }
}

// check if there's a trap near.
int is_near_existing_trap(int x, int y, int r)
{
    for (int i = 0; i < 15; ++i)
    {
        if (trap_list[i].x == 0)
            continue;

        int dx = abs(trap_list[i].x - x);
        int dy = abs(trap_list[i].y - y);

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

    while (check_around(game, x, y, 1, ELE_TRAP) || check_around(game, x, y, 1, ELE_WALL)) // check if there's no other trap close to it
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
    char ch = getch();

    switch (ch)
    {
        /* ----------------------------- game flow part ----------------------------- */
    case 'q':
        playing = 0; // quit the game
        break;
    case 'm':
        manual_mode = manual_mode ^ 1; //
        break;
        /* ------------------------------ movement part ----------------------------- */
    case KEY_UP:
        player.facing_direction = UP;
        break;
    case KEY_RIGHT:
        player.facing_direction = RIGHT;
        break;
    case KEY_DOWN:
        player.facing_direction = DOWN;
        break;
    case KEY_LEFT:
        player.facing_direction = LEFT;
        break;

    default:
        break;
    }
}

// move the player


