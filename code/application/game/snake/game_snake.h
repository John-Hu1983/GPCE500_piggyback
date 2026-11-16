#ifndef _GAME_SNAKE_H__
#define _GAME_SNAKE_H__

// clang-format off
#define SNAKE_COLS      25U
#define SNAKE_ROWS      18U
#define SNAKE_CELL      6U
#define SNAKE_X0        5U
#define SNAKE_Y0        18U
#define SNAKE_MAX_LEN   80U
// clang-format on

typedef struct
{
    u8 x[SNAKE_MAX_LEN];
    u8 y[SNAKE_MAX_LEN];
    u8 len;
    s16 dx;
    s16 dy;
    s16 next_dx;
    s16 next_dy;
    u8 food_x;
    u8 food_y;
    u8 tick_cnt;
    u8 speed_div;
    u16 score;
    bool over;
} Snake_State;

void game_snake_init(Snake_State *s);
void game_snake_step(Snake_State *s);
void game_snake_on_key(Snake_State *s, BTN_Id key);

#endif /* _GAME_SNAKE_H__ */
