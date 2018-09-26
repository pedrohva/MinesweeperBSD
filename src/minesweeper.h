#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#define FIELD_WIDTH     9
#define FIELD_HEIGHT    9
#define NUM_MINES       10

#define MINE_SPRITE     '*'
#define FLAG_SPRITE     '+'

typedef struct {
    int adjacent_mines;
    int revealed;
    int has_mine;
    int has_flag;
} Tile;

typedef struct {
    Tile field[FIELD_WIDTH][FIELD_HEIGHT];
    int mines_remaining;
} MinesweeperState;

void minesweeper_init(MinesweeperState *state);

/**
 * Sets a tile at the specified coordinates to revealed. 
 **/
void reveal_tile(int x, int y, MinesweeperState *state);

int flag_tile(int x, int y, MinesweeperState *state);

/**
 * Converts a coordinate from the game (such as A1, 1A, B2, etc.) into coordinates 
 * that match to the field array
 * Will return a 1 if the conversion was successful
 **/
int convert_coordinate(char *coord, int *x, int *y);

#endif // MINESWEEPER_H