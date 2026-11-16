#include "..\..\..\Include\spinmaster.h"

/*
 *@ brief    : Draw one 6x6 grid cell on the Snake playfield.
 *@ type     : private
 *@ argu in  : gx, gy: grid cell coordinates; color: RGB565 fill color.
 *@ argu out : Cell rectangle rendered on LCD.
 *@ theme    : Dirty-cell rendering without clearing the full screen.
 */
static void _snake_draw_cell(u8 gx, u8 gy, u16 color)
{
    s16 px = (s16)(SNAKE_X0 + gx * SNAKE_CELL);
    s16 py = (s16)(SNAKE_Y0 + gy * SNAKE_CELL);
    gui_fill_rect(px, py, (s16)(px + SNAKE_CELL - 2), (s16)(py + SNAKE_CELL - 2), color);
}

/*
 *@ brief    : Spawn a new apple on an unoccupied grid cell and draw it.
 *@ type     : private
 *@ argu in  : s: pointer to active Snake_State.
 *@ argu out : food_x, food_y updated and drawn in red.
 *@ theme    : Collision-free random food placement.
 */
static void _snake_spawn_food(Snake_State *s)
{
    u8 i, fx, fy;
    bool hit;

    do
    {
        hit = false;
        fx = (u8)(gui_game_rand() % SNAKE_COLS);
        fy = (u8)(gui_game_rand() % SNAKE_ROWS);
        for (i = 0U; i < s->len; ++i)
        {
            if (s->x[i] == fx && s->y[i] == fy)
            {
                hit = true;
                break;
            }
        }
    } while (hit);

    s->food_x = fx;
    s->food_y = fy;
    _snake_draw_cell(fx, fy, GUI_COLOR_RED);
}

/*
 *@ brief    : Initialize or reset the Snake game state and draw the initial playfield.
 *@ type     : public
 *@ argu in  : s: pointer to allocated Snake_State.
 *@ argu out : Playfield border, initial 4-segment snake, and first apple rendered.
 *@ theme    : Snake game initialization.
 */
void game_snake_init(Snake_State *s)
{
    u8 i;

    s->len = 4U;
    s->dx = 1;
    s->dy = 0;
    s->next_dx = 1;
    s->next_dy = 0;
    s->tick_cnt = 0U;
    s->speed_div = 4U;
    s->score = 0U;
    s->over = false;

    lcd_fill_color(GAME_BG_COLOR);
    gui_game_draw_hud(0U);
    gui_draw_rect((s16)(SNAKE_X0 - 2), (s16)(SNAKE_Y0 - 2),
                  (s16)(SNAKE_X0 + SNAKE_COLS * SNAKE_CELL),
                  (s16)(SNAKE_Y0 + SNAKE_ROWS * SNAKE_CELL), GAME_BORDER_COLOR);

    for (i = 0U; i < s->len; ++i)
    {
        s->x[i] = (u8)(10U - i);
        s->y[i] = 9U;
        _snake_draw_cell(s->x[i], s->y[i], (i == 0U) ? GUI_COLOR_YELLOW : GUI_COLOR_GREEN);
    }
    _snake_spawn_food(s);
}

/*
 *@ brief    : Advance the Snake game by one timer frame.
 *@ type     : public
 *@ argu in  : s: pointer to active Snake_State.
 *@ argu out : Snake moved, collisions and apple pickup processed.
 *@ theme    : Incremental head/tail cell redraw at 32Hz base timer.
 */
void game_snake_step(Snake_State *s)
{
    s16 nx, ny;
    u8 i;

    if (s->over)
    {
        return;
    }
    if (++s->tick_cnt < s->speed_div)
    {
        return;
    }
    s->tick_cnt = 0U;

    s->dx = s->next_dx;
    s->dy = s->next_dy;
    nx = (s16)s->x[0] + s->dx;
    ny = (s16)s->y[0] + s->dy;

    if (nx < 0 || nx >= (s16)SNAKE_COLS || ny < 0 || ny >= (s16)SNAKE_ROWS)
    {
        s->over = true;
        gui_game_draw_over_banner("GAME OVER");
        return;
    }
    for (i = 0U; i < s->len; ++i)
    {
        if (s->x[i] == (u8)nx && s->y[i] == (u8)ny)
        {
            s->over = true;
            gui_game_draw_over_banner("GAME OVER");
            return;
        }
    }

    if ((u8)nx == s->food_x && (u8)ny == s->food_y)
    {
        if (s->len < SNAKE_MAX_LEN)
        {
            s->len++;
        }
        for (i = (u8)(s->len - 1U); i > 0U; --i)
        {
            s->x[i] = s->x[i - 1U];
            s->y[i] = s->y[i - 1U];
        }
        _snake_draw_cell(s->x[1], s->y[1], GUI_COLOR_GREEN);
        s->x[0] = (u8)nx;
        s->y[0] = (u8)ny;
        _snake_draw_cell(s->x[0], s->y[0], GUI_COLOR_YELLOW);

        s->score = (u16)(s->score + 10U);
        if (s->speed_div > 2U && (s->score % 50U) == 0U)
        {
            s->speed_div--;
        }
        gui_game_update_score(s->score);
        gui_game_sfx(GAME_TONE_SCORE);
        _snake_spawn_food(s);
    }
    else
    {
        _snake_draw_cell(s->x[s->len - 1U], s->y[s->len - 1U], GAME_BG_COLOR);
        for (i = (u8)(s->len - 1U); i > 0U; --i)
        {
            s->x[i] = s->x[i - 1U];
            s->y[i] = s->y[i - 1U];
        }
        _snake_draw_cell(s->x[1], s->y[1], GUI_COLOR_GREEN);
        s->x[0] = (u8)nx;
        s->y[0] = (u8)ny;
        _snake_draw_cell(s->x[0], s->y[0], GUI_COLOR_YELLOW);
    }
}

/*
 *@ brief    : Handle directional and restart key events for the Snake game.
 *@ type     : public
 *@ argu in  : s: pointer to active Snake_State; key: pressed BTN_Id.
 *@ argu out : Next heading direction updated or game restarted if over.
 *@ theme    : Prevent 180-degree self-reversal.
 */
void game_snake_on_key(Snake_State *s, BTN_Id key)
{
    if (s->over)
    {
        if (key == Btn_A7_Enter)
        {
            game_snake_init(s);
        }
        return;
    }

    if (key == Btn_A1_Up && s->dy == 0)
    {
        s->next_dx = 0;
        s->next_dy = -1;
    }
    else if (key == Btn_A5_Down && s->dy == 0)
    {
        s->next_dx = 0;
        s->next_dy = 1;
    }
    else if (key == Btn_A4_Left && s->dx == 0)
    {
        s->next_dx = -1;
        s->next_dy = 0;
    }
    else if (key == Btn_A6_Right && s->dx == 0)
    {
        s->next_dx = 1;
        s->next_dy = 0;
    }
}
