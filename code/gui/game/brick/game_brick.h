#ifndef _GAME_BRICK_H__
#define _GAME_BRICK_H__

// clang-format off
#define BRK_ROWS    5U
#define BRK_COLS    10U
#define BRK_W       14U
#define BRK_H       5U
#define BRK_X0      5U
#define BRK_Y0      22U
// clang-format on

typedef struct
{
    u8 bricks[BRK_ROWS][BRK_COLS];
    s16 pad_x;
    s16 ball_x;
    s16 ball_y;
    s16 vx;
    s16 vy;
    u8 left_cnt;
    u16 score;
    bool over;
} Brick_State;

void game_brick_init(Brick_State *b);
void game_brick_step(Brick_State *b);
void game_brick_on_key(Brick_State *b, BTN_Id key);

#endif /* _GAME_BRICK_H__ */
