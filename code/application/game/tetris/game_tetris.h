#ifndef _GAME_TETRIS_H__
#define _GAME_TETRIS_H__

// clang-format off
#define TET_COLS    10U
#define TET_ROWS    18U
#define TET_CELL    6U
#define TET_X0      20U
#define TET_Y0      18U
// clang-format on

typedef struct
{
    u8 board[TET_ROWS][TET_COLS];
    u8 cur_type;
    u8 cur_rot;
    s16 cur_x;
    s16 cur_y;
    u8 next_type;
    u8 tick_cnt;
    u16 score;
    u16 lines;
    bool over;
} Tetris_State;

void game_tetris_init(Tetris_State *t);
void game_tetris_step(Tetris_State *t);
void game_tetris_on_key(Tetris_State *t, BTN_Id key);

#endif /* _GAME_TETRIS_H__ */
