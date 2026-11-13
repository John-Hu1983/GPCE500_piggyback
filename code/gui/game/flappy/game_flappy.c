#include "..\..\..\Include\spinmaster.h"

/*
 *@ brief    : Render or erase the Flappy Bird sprite at vertical coordinate `y`.
 *@ type     : private
 *@ argu in  : y: vertical center of bird; erase: true to clear with background color.
 *@ argu out : Bird body, eye, and beak drawn or erased.
 *@ theme    : Incremental sprite rendering.
 */
static void _flp_draw_bird(s16 y, bool erase)
{
    u16 body = erase ? GAME_BG_COLOR : GUI_COLOR_YELLOW;
    u16 beak = erase ? GAME_BG_COLOR : GUI_COLOR_ORANGE;

    if (y < 19)
    {
        y = 19;
    }
    if (y > 121)
    {
        y = 121;
    }
    gui_fill_rect((s16)(FLP_BIRD_X - 3), (s16)(y - 3), (s16)(FLP_BIRD_X + 4), (s16)(y + 3), body);
    if (!erase)
    {
        gui_draw_point((s16)(FLP_BIRD_X + 1), (s16)(y - 1), GUI_COLOR_BLACK);
        gui_fill_rect((s16)(FLP_BIRD_X + 3), y, (s16)(FLP_BIRD_X + 5), (s16)(y + 1), beak);
    }
}

/*
 *@ brief    : Initialize or reset the Flappy Bird game state and draw the initial scene.
 *@ type     : public
 *@ argu in  : f: pointer to allocated Flappy_State.
 *@ argu out : HUD, ground line, and bird initialized.
 *@ theme    : Flappy Bird initialization.
 */
void game_flappy_init(Flappy_State *f)
{
    f->bird_y = 64;
    f->bird_vy = 0;
    f->pipe_x[0] = 110;
    f->gap_y[0] = 52;
    f->passed[0] = false;
    f->pipe_x[1] = 190;
    f->gap_y[1] = 72;
    f->passed[1] = false;
    f->score = 0U;
    f->over = false;

    lcd_fill_color(GAME_BG_COLOR);
    gui_game_draw_hud(0U);
    gui_draw_hline(0, (s16)(LCD_WIDTH - 1U), 126, GUI_COLOR_GREEN);
    _flp_draw_bird(f->bird_y, false);
}

/*
 *@ brief    : Advance Flappy Bird gravity, pipe scrolling, and collision checks by one frame.
 *@ type     : public
 *@ argu in  : f: pointer to active Flappy_State.
 *@ argu out : Pipes scrolled by 2px using edge strip updates; bird position updated.
 *@ theme    : Flicker-free 2px strip pipe scrolling.
 */
void game_flappy_step(Flappy_State *f)
{
    u8 i;
    s16 ny;

    if (f->over)
    {
        return;
    }

    _flp_draw_bird(f->bird_y, true);
    f->bird_vy = (s16)(f->bird_vy + 1);
    if (f->bird_vy > 5)
    {
        f->bird_vy = 5;
    }
    ny = (s16)(f->bird_y + f->bird_vy);
    if (ny < 20)
    {
        ny = 20;
        f->bird_vy = 0;
    }
    if (ny >= 122)
    {
        f->over = true;
        gui_game_draw_over_banner("GAME OVER");
        return;
    }
    f->bird_y = ny;

    for (i = 0U; i < 2U; ++i)
    {
        s16 ox = f->pipe_x[i];
        s16 nx_p = (s16)(ox - 2);
        s16 gy = f->gap_y[i];

        if (ox + FLP_PIPE_W >= 0 && ox + FLP_PIPE_W < (s16)LCD_WIDTH)
        {
            s16 ex0 = (s16)(ox + FLP_PIPE_W - 2);
            s16 ex1 = (s16)(ox + FLP_PIPE_W - 1);
            if (ex0 < 0)
            {
                ex0 = 0;
            }
            gui_fill_rect(ex0, 16, ex1, (s16)(gy - 1), GAME_BG_COLOR);
            gui_fill_rect(ex0, (s16)(gy + FLP_GAP_H), ex1, 125, GAME_BG_COLOR);
        }

        if (nx_p >= 0 && nx_p < (s16)LCD_WIDTH)
        {
            s16 dx1 = (s16)(nx_p + 1);
            if (dx1 >= (s16)LCD_WIDTH)
            {
                dx1 = (s16)(LCD_WIDTH - 1U);
            }
            gui_fill_rect(nx_p, 16, dx1, (s16)(gy - 1), GUI_COLOR_GREEN);
            gui_fill_rect(nx_p, (s16)(gy + FLP_GAP_H), dx1, 125, GUI_COLOR_GREEN);
        }

        f->pipe_x[i] = nx_p;
        if (nx_p + FLP_PIPE_W < 0)
        {
            f->pipe_x[i] = (s16)LCD_WIDTH;
            f->gap_y[i] = (s16)(28 + (gui_game_rand() % 52U));
            f->passed[i] = false;
        }

        if (!f->passed[i] && (nx_p + FLP_PIPE_W) < FLP_BIRD_X)
        {
            f->passed[i] = true;
            f->score = (u16)(f->score + 1U);
            gui_game_update_score(f->score);
            gui_game_sfx(GAME_TONE_SCORE);
        }

        if ((FLP_BIRD_X + 3) >= nx_p && (FLP_BIRD_X - 3) < (nx_p + FLP_PIPE_W))
        {
            if ((f->bird_y - 3) < gy || (f->bird_y + 3) >= (gy + FLP_GAP_H))
            {
                _flp_draw_bird(f->bird_y, false);
                f->over = true;
                gui_game_draw_over_banner("GAME OVER");
                return;
            }
        }
    }

    _flp_draw_bird(f->bird_y, false);
}

/*
 *@ brief    : Handle flap (Up/Enter) and restart key events for Flappy Bird.
 *@ type     : public
 *@ argu in  : f: pointer to active Flappy_State; key: pressed BTN_Id.
 *@ argu out : Bird upward velocity applied or game restarted if over.
 *@ theme    : Flappy Bird key input handler.
 */
void game_flappy_on_key(Flappy_State *f, BTN_Id key)
{
    if (f->over)
    {
        if (key == Btn_A7_Enter)
        {
            game_flappy_init(f);
        }
    }
    else if (key == Btn_A7_Enter || key == Btn_A1_Up)
    {
        f->bird_vy = -5;
        gui_game_sfx(GAME_TONE_HIT);
    }
}
