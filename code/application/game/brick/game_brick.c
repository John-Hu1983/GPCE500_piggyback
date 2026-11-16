#include "..\..\..\Include\spinmaster.h"

static const u16 brk_row_colors[BRK_ROWS] = {GUI_COLOR_RED, GUI_COLOR_ORANGE, GUI_COLOR_YELLOW,
                                             GUI_COLOR_GREEN, GUI_COLOR_CYAN};

/*
 *@ brief    : Render or erase a single brick at (r, c).
 *@ type     : private
 *@ argu in  : r, c: brick row and column; alive: true to draw row color, false to erase.
 *@ argu out : Brick rectangle updated on LCD.
 *@ theme    : Dirty-brick rendering.
 */
static void _brk_draw_brick(u8 r, u8 c, bool alive)
{
    s16 px = (s16)(BRK_X0 + c * (BRK_W + 1U));
    s16 py = (s16)(BRK_Y0 + r * (BRK_H + 2U));
    gui_fill_rect(px, py, (s16)(px + BRK_W - 1U), (s16)(py + BRK_H - 1U),
                  alive ? brk_row_colors[r] : GAME_BG_COLOR);
}

/*
 *@ brief    : Initialize or reset the Brick Breaker game state and draw all bricks, paddle, and ball.
 *@ type     : public
 *@ argu in  : b: pointer to allocated Brick_State.
 *@ argu out : 5x10 rainbow brick wall, paddle, and ball rendered.
 *@ theme    : Brick Breaker initialization.
 */
void game_brick_init(Brick_State *b)
{
    u8 r, c;

    b->pad_x = 66;
    b->ball_x = 78;
    b->ball_y = 112;
    b->vx = 2;
    b->vy = -2;
    b->left_cnt = (u8)(BRK_ROWS * BRK_COLS);
    b->score = 0U;
    b->over = false;

    lcd_fill_color(GAME_BG_COLOR);
    gui_game_draw_hud(0U);

    for (r = 0U; r < BRK_ROWS; ++r)
    {
        for (c = 0U; c < BRK_COLS; ++c)
        {
            b->bricks[r][c] = 1U;
            _brk_draw_brick(r, c, true);
        }
    }
    gui_fill_rect(b->pad_x, 120, (s16)(b->pad_x + 28), 123, GUI_COLOR_WHITE);
    gui_fill_circle(b->ball_x, b->ball_y, 2U, GUI_COLOR_YELLOW);
}

/*
 *@ brief    : Advance paddle movement, ball physics, and brick collisions by one frame.
 *@ type     : public
 *@ argu in  : b: pointer to active Brick_State.
 *@ argu out : Paddle and ball updated; destroyed bricks erased and scored.
 *@ theme    : 32FPS Breakout physics step.
 */
void game_brick_step(Brick_State *b)
{
    s16 nx, ny;
    u8 r, c;

    if (b->over)
    {
        return;
    }

    if (gpio_read_level(GPIOA, 4U) && b->pad_x > 4)
    {
        gui_fill_rect((s16)(b->pad_x + 25), 120, (s16)(b->pad_x + 28), 123, GAME_BG_COLOR);
        b->pad_x = (s16)(b->pad_x - 4);
        gui_fill_rect(b->pad_x, 120, (s16)(b->pad_x + 28), 123, GUI_COLOR_WHITE);
    }
    else if (gpio_read_level(GPIOA, 6U) && b->pad_x < 126)
    {
        gui_fill_rect(b->pad_x, 120, (s16)(b->pad_x + 3), 123, GAME_BG_COLOR);
        b->pad_x = (s16)(b->pad_x + 4);
        gui_fill_rect(b->pad_x, 120, (s16)(b->pad_x + 28), 123, GUI_COLOR_WHITE);
    }

    gui_fill_circle(b->ball_x, b->ball_y, 2U, GAME_BG_COLOR);
    nx = (s16)(b->ball_x + b->vx);
    ny = (s16)(b->ball_y + b->vy);

    if (nx <= 4 || nx >= 155)
    {
        b->vx = (s16)(-b->vx);
        nx = (s16)(b->ball_x + b->vx);
        gui_game_sfx(GAME_TONE_MOVE);
    }
    if (ny <= 18)
    {
        b->vy = (s16)(-b->vy);
        ny = (s16)(b->ball_y + b->vy);
        gui_game_sfx(GAME_TONE_MOVE);
    }

    if (ny >= 117 && ny <= 122 && nx >= (s16)(b->pad_x - 2) && nx <= (s16)(b->pad_x + 30))
    {
        b->vy = -2;
        b->vx = (nx < (s16)(b->pad_x + 10)) ? -2 : ((nx > (s16)(b->pad_x + 18)) ? 2 : b->vx);
        ny = 116;
        gui_fill_rect(b->pad_x, 120, (s16)(b->pad_x + 28), 123, GUI_COLOR_WHITE);
        gui_game_sfx(GAME_TONE_HIT);
    }

    if (ny >= (s16)BRK_Y0 && ny < (s16)(BRK_Y0 + BRK_ROWS * (BRK_H + 2U)))
    {
        r = (u8)((ny - (s16)BRK_Y0) / (s16)(BRK_H + 2U));
        if (nx >= (s16)BRK_X0 && nx < (s16)(BRK_X0 + BRK_COLS * (BRK_W + 1U)))
        {
            c = (u8)((nx - (s16)BRK_X0) / (s16)(BRK_W + 1U));
            if (r < BRK_ROWS && c < BRK_COLS && b->bricks[r][c] != 0U)
            {
                b->bricks[r][c] = 0U;
                _brk_draw_brick(r, c, false);
                b->vy = (s16)(-b->vy);
                b->score = (u16)(b->score + 10U);
                b->left_cnt--;
                gui_game_update_score(b->score);
                gui_game_sfx(GAME_TONE_SCORE);
                if (b->left_cnt == 0U)
                {
                    b->over = true;
                    gui_game_draw_over_banner("YOU WIN!");
                    return;
                }
            }
        }
    }

    if (ny >= 125)
    {
        b->over = true;
        gui_game_draw_over_banner("GAME OVER");
        return;
    }

    b->ball_x = nx;
    b->ball_y = ny;
    gui_fill_circle(b->ball_x, b->ball_y, 2U, GUI_COLOR_YELLOW);
}

/*
 *@ brief    : Handle key events for Brick Breaker (restart on Enter when game over).
 *@ type     : public
 *@ argu in  : b: pointer to active Brick_State; key: pressed BTN_Id.
 *@ argu out : Game restarted if over and Enter pressed.
 *@ theme    : Brick Breaker key input handler.
 */
void game_brick_on_key(Brick_State *b, BTN_Id key)
{
    if (b->over && key == Btn_A7_Enter)
    {
        game_brick_init(b);
    }
}
