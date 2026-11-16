#ifndef _GAME_SHOOTER_H__
#define _GAME_SHOOTER_H__

// clang-format off
#define SH_MAX_BULLETS  4U
#define SH_MAX_ENEMIES  4U
// clang-format on

typedef struct
{
    s16 ship_x;
    s16 ship_y;
    s16 bul_x[SH_MAX_BULLETS];
    s16 bul_y[SH_MAX_BULLETS];
    bool bul_act[SH_MAX_BULLETS];
    s16 enm_x[SH_MAX_ENEMIES];
    s16 enm_y[SH_MAX_ENEMIES];
    bool enm_act[SH_MAX_ENEMIES];
    u8 fire_cd;
    u16 score;
    bool over;
} Shooter_State;

void game_shooter_init(Shooter_State *sh);
void game_shooter_step(Shooter_State *sh);
void game_shooter_on_key(Shooter_State *sh, BTN_Id key);

#endif /* _GAME_SHOOTER_H__ */
