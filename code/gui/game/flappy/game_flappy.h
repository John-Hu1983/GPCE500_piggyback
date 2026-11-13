#ifndef _GAME_FLAPPY_H__
#define _GAME_FLAPPY_H__

// clang-format off
#define FLP_BIRD_X      32
#define FLP_PIPE_W      14
#define FLP_GAP_H       36
// clang-format on

typedef struct
{
    s16 bird_y;
    s16 bird_vy;
    s16 pipe_x[2];
    s16 gap_y[2];
    bool passed[2];
    u16 score;
    bool over;
} Flappy_State;

void game_flappy_init(Flappy_State *f);
void game_flappy_step(Flappy_State *f);
void game_flappy_on_key(Flappy_State *f, BTN_Id key);

#endif /* _GAME_FLAPPY_H__ */
