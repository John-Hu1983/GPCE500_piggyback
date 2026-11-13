#include "..\..\..\Include\spinmaster.h"

/*
 *@ brief    : Render or erase the player starfighter ship at (x, y).
 *@ type     : private
 *@ argu in  : x, y: ship center coordinates; erase: true to clear with background color.
 *@ argu out : Delta-wing starfighter drawn or erased.
 *@ theme    : Incremental ship sprite rendering.
 */
static void _sh_draw_ship(s16 x, s16 y, bool erase)
{
    u16 wing = erase ? GAME_BG_COLOR : GUI_COLOR_CYAN;
    u16 core = erase ? GAME_BG_COLOR : GUI_COLOR_YELLOW;

    gui_fill_triangle(x, (s16)(y - 5), (s16)(x - 6), (s16)(y + 4), (s16)(x + 6), (s16)(y + 4),
                      wing);
    if (!erase)
    {
        gui_fill_rect((s16)(x - 1), (s16)(y - 3), (s16)(x + 1), (s16)(y + 3), core);
    }
}

/*
 *@ brief    : Render or erase an alien enemy ship at (x, y).
 *@ type     : private
 *@ argu in  : x, y: enemy center coordinates; erase: true to clear with background color.
 *@ argu out : Enemy sprite drawn or erased.
 *@ theme    : Incremental enemy sprite rendering.
 */
static void _sh_draw_enemy(s16 x, s16 y, bool erase)
{
    u16 col = erase ? GAME_BG_COLOR : GUI_COLOR_MAGENTA;

    gui_fill_rect((s16)(x - 4), (s16)(y - 3), (s16)(x + 4), (s16)(y + 3), col);
    if (!erase)
    {
        gui_draw_point((s16)(x - 2), (s16)(y - 1), GUI_COLOR_YELLOW);
        gui_draw_point((s16)(x + 2), (s16)(y - 1), GUI_COLOR_YELLOW);
    }
}

/*
 *@ brief    : Spawn a new upward laser bolt from the nose of the player starfighter.
 *@ type     : private
 *@ argu in  : sh: pointer to active Shooter_State.
 *@ argu out : Free bullet slot activated and laser SFX played.
 *@ theme    : Player weapon projectile spawner.
 */
static void _sh_fire(Shooter_State *sh)
{
    u8 i;

    for (i = 0U; i < SH_MAX_BULLETS; ++i)
    {
        if (!sh->bul_act[i])
        {
            sh->bul_act[i] = true;
            sh->bul_x[i] = sh->ship_x;
            sh->bul_y[i] = (s16)(sh->ship_y - 7);
            gui_game_sfx(GAME_TONE_HIGH);
            break;
        }
    }
}

/*
 *@ brief    : Initialize or reset the Space Shooter game state and draw the initial scene.
 *@ type     : public
 *@ argu in  : sh: pointer to allocated Shooter_State.
 *@ argu out : Player starfighter and initial wave of enemies rendered.
 *@ theme    : Space Shooter initialization.
 */
void game_shooter_init(Shooter_State *sh)
{
    u8 i;

    sh->ship_x = 80;
    sh->ship_y = 116;
    sh->fire_cd = 0U;
    sh->score = 0U;
    sh->over = false;

    for (i = 0U; i < SH_MAX_BULLETS; ++i)
    {
        sh->bul_act[i] = false;
    }
    for (i = 0U; i < SH_MAX_ENEMIES; ++i)
    {
        sh->enm_act[i] = true;
        sh->enm_x[i] = (s16)(20 + i * 36);
        sh->enm_y[i] = (s16)(24 + (i & 1U) * 14);
    }

    lcd_fill_color(GAME_BG_COLOR);
    gui_game_draw_hud(0U);
    _sh_draw_ship(sh->ship_x, sh->ship_y, false);
    for (i = 0U; i < SH_MAX_ENEMIES; ++i)
    {
        _sh_draw_enemy(sh->enm_x[i], sh->enm_y[i], false);
    }
}

/*
 *@ brief    : Advance ship movement, laser bolts, enemy descent, and hit detection by one frame.
 *@ type     : public
 *@ argu in  : sh: pointer to active Shooter_State.
 *@ argu out : Projectiles and enemies updated; score incremented on hits.
 *@ theme    : 32FPS Space Shooter game loop.
 */
void game_shooter_step(Shooter_State *sh)
{
    u8 i, j;
    s16 nx, ny;

    if (sh->over)
    {
        return;
    }

    nx = sh->ship_x;
    ny = sh->ship_y;
    if (gpio_read_level(GPIOA, 4U) && nx > 10)
    {
        nx = (s16)(nx - 3);
    }
    if (gpio_read_level(GPIOA, 6U) && nx < 150)
    {
        nx = (s16)(nx + 3);
    }
    if (gpio_read_level(GPIOA, 1U) && ny > 36)
    {
        ny = (s16)(ny - 2);
    }
    if (gpio_read_level(GPIOA, 5U) && ny < 120)
    {
        ny = (s16)(ny + 2);
    }
    if (nx != sh->ship_x || ny != sh->ship_y)
    {
        _sh_draw_ship(sh->ship_x, sh->ship_y, true);
        sh->ship_x = nx;
        sh->ship_y = ny;
        _sh_draw_ship(sh->ship_x, sh->ship_y, false);
    }

    if (++sh->fire_cd >= 10U)
    {
        sh->fire_cd = 0U;
        _sh_fire(sh);
    }

    for (i = 0U; i < SH_MAX_BULLETS; ++i)
    {
        if (sh->bul_act[i])
        {
            gui_draw_vline(sh->bul_x[i], (s16)(sh->bul_y[i] - 2), sh->bul_y[i], GAME_BG_COLOR);
            sh->bul_y[i] = (s16)(sh->bul_y[i] - 5);
            if (sh->bul_y[i] < 18)
            {
                sh->bul_act[i] = false;
            }
            else
            {
                gui_draw_vline(sh->bul_x[i], (s16)(sh->bul_y[i] - 2), sh->bul_y[i],
                               GUI_COLOR_YELLOW);
            }
        }
    }

    for (i = 0U; i < SH_MAX_ENEMIES; ++i)
    {
        if (!sh->enm_act[i])
        {
            sh->enm_act[i] = true;
            sh->enm_x[i] = (s16)(12 + (gui_game_rand() % 136U));
            sh->enm_y[i] = 22;
        }
        else
        {
            _sh_draw_enemy(sh->enm_x[i], sh->enm_y[i], true);
            sh->enm_y[i] = (s16)(sh->enm_y[i] + 1);
            if (sh->enm_y[i] >= 122)
            {
                sh->over = true;
                gui_game_draw_over_banner("GAME OVER");
                return;
            }
        }

        for (j = 0U; j < SH_MAX_BULLETS; ++j)
        {
            if (sh->bul_act[j] && sh->bul_x[j] >= (s16)(sh->enm_x[i] - 5) &&
                sh->bul_x[j] <= (s16)(sh->enm_x[i] + 5) &&
                sh->bul_y[j] >= (s16)(sh->enm_y[i] - 4) &&
                sh->bul_y[j] <= (s16)(sh->enm_y[i] + 4))
            {
                gui_draw_vline(sh->bul_x[j], (s16)(sh->bul_y[j] - 2), sh->bul_y[j], GAME_BG_COLOR);
                sh->bul_act[j] = false;
                sh->enm_act[i] = false;
                sh->score = (u16)(sh->score + 15U);
                gui_game_update_score(sh->score);
                gui_game_sfx(GAME_TONE_SCORE);
                break;
            }
        }

        if (sh->enm_act[i])
        {
            if (sh->enm_x[i] >= (s16)(sh->ship_x - 8) && sh->enm_x[i] <= (s16)(sh->ship_x + 8) &&
                sh->enm_y[i] >= (s16)(sh->ship_y - 6) && sh->enm_y[i] <= (s16)(sh->ship_y + 5))
            {
                sh->over = true;
                gui_game_draw_over_banner("GAME OVER");
                return;
            }
            _sh_draw_enemy(sh->enm_x[i], sh->enm_y[i], false);
        }
    }
}

/*
 *@ brief    : Handle manual fire and restart key events for Space Shooter.
 *@ type     : public
 *@ argu in  : sh: pointer to active Shooter_State; key: pressed BTN_Id.
 *@ argu out : Laser bolt fired or game restarted if over.
 *@ theme    : Space Shooter key input handler.
 */
void game_shooter_on_key(Shooter_State *sh, BTN_Id key)
{
    if (sh->over)
    {
        if (key == Btn_A7_Enter)
        {
            game_shooter_init(sh);
        }
    }
    else if (key == Btn_A7_Enter)
    {
        _sh_fire(sh);
    }
}
