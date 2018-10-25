#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#define FIELD_WIDTH     9
#define FIELD_HEIGHT    9
#define NUM_MINES       2

#define MINE_SPRITE     '*'
#define FLAG_SPRITE     '+'

/**
 * A tile is one point in the Minesweeper field. Ie., the field is made of 
 * many tiles.
 **/
typedef struct {
    int adjacent_mines;
    int revealed;
    int has_mine;
    int has_flag;
} Tile;

/**
 * Contains information about a current game of Minesweeper
 **/
typedef struct {
    Tile field[FIELD_WIDTH][FIELD_HEIGHT];
    int mines_remaining;
    int game_won;
    time_t game_start_time;
    time_t game_time_taken;
    char* username;
} MinesweeperState;

/**
 * Prepare a Minesweeper field by randomly placing mines and starting the timer.
 * Will reset the previous board state. 
 **/
void minesweeper_init(MinesweeperState *state);

/**
 * Sets a tile at the specified coordinates to revealed. 
 **/
void reveal_tile(int x, int y, MinesweeperState *state);

/**
 * Attempts to place a flag at a coordinate in the game field.
 * 
 * Return   1 - Flag placed at a location where mine resides
 *          0 - Flag no placed because there is no mine present
 **/
int flag_tile(int x, int y, MinesweeperState *state);

/**
 * Reveals all the mines on the field. Will also hide every tile that is not a mine
 **/
void show_mines(MinesweeperState *state, int show_flags);

/**
 * Converts a coordinate from the game (such as A1, 1A, B2, etc.) into coordinates 
 * that match to the field array
 * Will return a 1 if the conversion was successful
 **/
int convert_coordinate(char *coord, int *x, int *y);

#endif // MINESWEEPER_H