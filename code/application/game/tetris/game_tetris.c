#include "..\..\..\Include\spinmaster.h"

/* 7 standard tetrominoes x 4 rotations encoded as 4x4 16-bit bitmasks */
static const u16 tet_shapes[7][4] = {
    {0x0F00U, 0x2222U, 0x00F0U, 0x4444U}, /* I */
    {0x6600U, 0x6600U, 0x6600U, 0x6600U}, /* O */
    {0x0E40U, 0x4C40U, 0x4E00U, 0x4640U}, /* T */
    {0x06C0U, 0x8C40U, 0x6C00U, 0x4620U}, /* S */
    {0x0C60U, 0x4C80U, 0xC600U, 0x2640U}, /* Z */
    {0x0E20U, 0x44C0U, 0x8E00U, 0x6440U}, /* J */
    {0x0E80U, 0xC440U, 0x2E00U, 0x4460U}  /* L */
};

static const u16 tet_colors[8] = {GAME_BG_COLOR,     GUI_COLOR_CYAN,  GUI_COLOR_YELLOW,
                                  GUI_COLOR_MAGENTA, GUI_COLOR_GREEN, GUI_COLOR_RED,
                                  GUI_COLOR_BLUE,    GUI_COLOR_ORANGE};

/*
 *@ brief    : Draw one 6x6 cell on the 10x18 Tetris board.
 *@ type     : private
 *@ argu in  : c, r: column and row indices; color_idx: palette index (0..7).
 *@ argu out : Cell drawn on LCD.
 *@ theme    : Board block renderer.
 */
static void _tet_draw_cell(s16 c, s16 r, u8 color_idx)
{
    s16 px, py;

    if (c < 0 || c >= (s16)TET_COLS || r < 0 || r >= (s16)TET_ROWS)
    {
        return;
    }
    px = (s16)(TET_X0 + c * TET_CELL);
    py = (s16)(TET_Y0 + r * TET_CELL);
    gui_fill_rect(px, py, (s16)(px + TET_CELL - 2), (s16)(py + TET_CELL - 2),
                  tet_colors[color_idx]);
}

/*
 *@ brief    : Render or erase the active falling tetromino piece.
 *@ type     : private
 *@ argu in  : t: pointer to Tetris_State; color_idx: 0 to erase, or 1..7 to draw.
 *@ argu out : 4 blocks of the active piece rendered.
 *@ theme    : Incremental piece rendering.
 */
static void _tet_draw_piece(const Tetris_State *t, u8 color_idx)
{
    u16 mask = tet_shapes[t->cur_type][t->cur_rot];
    s16 r, c;

    for (r = 0; r < 4; ++r)
    {
        for (c = 0; c < 4; ++c)
        {
            if ((mask & (u16)(0x8000U >> (r * 4 + c))) != 0U)
            {
                _tet_draw_cell((s16)(t->cur_x + c), (s16)(t->cur_y + r), color_idx);
            }
        }
    }
}

/*
 *@ brief    : Check if the active piece can occupy (nx, ny) with rotation `nrot`.
 *@ type     : private
 *@ argu in  : t: pointer to Tetris_State; nx, ny: candidate position; nrot: candidate rotation.
 *@ argu out : true if valid and unobstructed, false on wall/floor/block collision.
 *@ theme    : Tetromino collision detection.
 */
static bool _tet_check(const Tetris_State *t, s16 nx, s16 ny, u8 nrot)
{
    u16 mask = tet_shapes[t->cur_type][nrot];
    s16 r, c;

    for (r = 0; r < 4; ++r)
    {
        for (c = 0; c < 4; ++c)
        {
            if ((mask & (u16)(0x8000U >> (r * 4 + c))) != 0U)
            {
                s16 gx = (s16)(nx + c);
                s16 gy = (s16)(ny + r);
                if (gx < 0 || gx >= (s16)TET_COLS || gy >= (s16)TET_ROWS)
                {
                    return false;
                }
                if (gy >= 0 && t->board[(u8)gy][(u8)gx] != 0U)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

/*
 *@ brief    : Draw the next tetromino preview in the right-hand side panel.
 *@ type     : private
 *@ argu in  : t: pointer to Tetris_State.
 *@ argu out : Preview box cleared and next piece drawn.
 *@ theme    : Next piece HUD preview.
 */
static void _tet_draw_next(const Tetris_State *t)
{
    u16 mask = tet_shapes[t->next_type][0];
    s16 r, c;

    gui_fill_rect(98, 36, 130, 66, GAME_BG_COLOR);
    for (r = 0; r < 4; ++r)
    {
        for (c = 0; c < 4; ++c)
        {
            if ((mask & (u16)(0x8000U >> (r * 4 + c))) != 0U)
            {
                s16 px = (s16)(102 + c * TET_CELL);
                s16 py = (s16)(40 + r * TET_CELL);
                gui_fill_rect(px, py, (s16)(px + TET_CELL - 2), (s16)(py + TET_CELL - 2),
                              tet_colors[t->next_type + 1U]);
            }
        }
    }
}

/*
 *@ brief    : Spawn the next tetromino at the top of the well and roll a new next piece.
 *@ type     : private
 *@ argu in  : t: pointer to Tetris_State.
 *@ argu out : New piece active or Game Over banner shown if blocked.
 *@ theme    : Tetromino spawn lifecycle.
 */
static void _tet_spawn(Tetris_State *t)
{
    t->cur_type = t->next_type;
    t->next_type = (u8)(gui_game_rand() % 7U);
    t->cur_rot = 0U;
    t->cur_x = 3;
    t->cur_y = 0;
    _tet_draw_next(t);

    if (!_tet_check(t, t->cur_x, t->cur_y, t->cur_rot))
    {
        t->over = true;
        gui_game_draw_over_banner("GAME OVER");
    }
    else
    {
        _tet_draw_piece(t, (u8)(t->cur_type + 1U));
    }
}

/*
 *@ brief    : Lock the landed tetromino into the board array and clear any completed rows.
 *@ type     : private
 *@ argu in  : t: pointer to Tetris_State.
 *@ argu out : Full rows removed, score/lines updated, and next piece spawned.
 *@ theme    : Line-clear evaluation and score update.
 */
static void _tet_lock_and_clear(Tetris_State *t)
{
    u16 mask = tet_shapes[t->cur_type][t->cur_rot];
    s16 r, c;
    u8 cleared = 0U;

    for (r = 0; r < 4; ++r)
    {
        for (c = 0; c < 4; ++c)
        {
            if ((mask & (u16)(0x8000U >> (r * 4 + c))) != 0U)
            {
                s16 gx = (s16)(t->cur_x + c);
                s16 gy = (s16)(t->cur_y + r);
                if (gx >= 0 && gx < (s16)TET_COLS && gy >= 0 && gy < (s16)TET_ROWS)
                {
                    t->board[(u8)gy][(u8)gx] = (u8)(t->cur_type + 1U);
                }
            }
        }
    }

    for (r = (s16)(TET_ROWS - 1U); r >= 0; --r)
    {
        bool full = true;
        for (c = 0; c < (s16)TET_COLS; ++c)
        {
            if (t->board[(u8)r][(u8)c] == 0U)
            {
                full = false;
                break;
            }
        }
        if (full)
        {
            s16 rr;
            cleared++;
            for (rr = r; rr > 0; --rr)
            {
                for (c = 0; c < (s16)TET_COLS; ++c)
                {
                    t->board[(u8)rr][(u8)c] = t->board[(u8)(rr - 1)][(u8)c];
                }
            }
            for (c = 0; c < (s16)TET_COLS; ++c)
            {
                t->board[0][(u8)c] = 0U;
            }
            r++;
        }
    }

    if (cleared > 0U)
    {
        for (r = 0; r < (s16)TET_ROWS; ++r)
        {
            for (c = 0; c < (s16)TET_COLS; ++c)
            {
                _tet_draw_cell(c, r, t->board[(u8)r][(u8)c]);
            }
        }
        t->lines = (u16)(t->lines + cleared);
        t->score = (u16)(t->score + (u16)cleared * 25U);
        gui_game_update_score(t->score);
        gui_fill_rect(96, 88, 136, 98, GAME_BG_COLOR);
        gui_draw_uint_at(96, 88, t->lines, 4U, true, GUI_COLOR_WHITE, GUI_Font_6x8);
        gui_game_sfx(GAME_TONE_HIGH);
    }
    else
    {
        gui_game_sfx(GAME_TONE_MOVE);
    }

    _tet_spawn(t);
}

/*
 *@ brief    : Attempt to translate or rotate the active tetromino.
 *@ type     : private
 *@ argu in  : t: pointer to Tetris_State; dx, dy: translation; drot: rotation step (0 or 1).
 *@ argu out : Piece moved/rotated on screen, or locked if blocked while moving down.
 *@ theme    : Tetromino movement controller.
 */
static void _tet_move(Tetris_State *t, s16 dx, s16 dy, u8 drot)
{
    u8 nrot;

    if (t->over)
    {
        return;
    }
    nrot = (u8)((t->cur_rot + drot) & 3U);
    if (_tet_check(t, (s16)(t->cur_x + dx), (s16)(t->cur_y + dy), nrot))
    {
        _tet_draw_piece(t, 0U);
        t->cur_x = (s16)(t->cur_x + dx);
        t->cur_y = (s16)(t->cur_y + dy);
        t->cur_rot = nrot;
        _tet_draw_piece(t, (u8)(t->cur_type + 1U));
        if (drot != 0U)
        {
            gui_game_sfx(GAME_TONE_HIT);
        }
    }
    else if (dy > 0)
    {
        _tet_lock_and_clear(t);
    }
}

/*
 *@ brief    : Initialize or reset the Tetris game state and draw the board and side panel.
 *@ type     : public
 *@ argu in  : t: pointer to allocated Tetris_State.
 *@ argu out : Board cleared, HUD drawn, and first tetromino spawned.
 *@ theme    : Tetris game initialization.
 */
void game_tetris_init(Tetris_State *t)
{
    u8 r, c;

    for (r = 0U; r < TET_ROWS; ++r)
    {
        for (c = 0U; c < TET_COLS; ++c)
        {
            t->board[r][c] = 0U;
        }
    }
    t->score = 0U;
    t->lines = 0U;
    t->tick_cnt = 0U;
    t->over = false;
    t->next_type = (u8)(gui_game_rand() % 7U);

    lcd_fill_color(GAME_BG_COLOR);
    gui_game_draw_hud(0U);
    gui_draw_rect((s16)(TET_X0 - 2), (s16)(TET_Y0 - 1),
                  (s16)(TET_X0 + TET_COLS * TET_CELL),
                  (s16)(TET_Y0 + TET_ROWS * TET_CELL), GAME_BORDER_COLOR);
    gui_draw_str_at(96, 24, "NEXT:", GUI_COLOR_CYAN, GUI_Font_6x8);
    gui_draw_str_at(96, 76, "LINES:", GUI_COLOR_CYAN, GUI_Font_6x8);
    gui_draw_uint_at(96, 88, 0U, 4U, true, GUI_COLOR_WHITE, GUI_Font_6x8);
    _tet_spawn(t);
}

/*
 *@ brief    : Advance the Tetris gravity timer by one frame (with fast soft-drop when A5 held).
 *@ type     : public
 *@ argu in  : t: pointer to active Tetris_State.
 *@ argu out : Active piece drops one row when gravity counter expires.
 *@ theme    : Periodic gravity tick handler.
 */
void game_tetris_step(Tetris_State *t)
{
    u8 limit = gpio_read_level(GPIOA, 5U) ? 2U : 10U;

    if (++t->tick_cnt >= limit)
    {
        t->tick_cnt = 0U;
        _tet_move(t, 0, 1, 0U);
    }
}

/*
 *@ brief    : Handle button actions (left, right, rotate, soft drop, hard drop) for Tetris.
 *@ type     : public
 *@ argu in  : t: pointer to active Tetris_State; key: pressed BTN_Id.
 *@ argu out : Tetromino moved, rotated, or hard-dropped.
 *@ theme    : Tetris key input handler.
 */
void game_tetris_on_key(Tetris_State *t, BTN_Id key)
{
    if (t->over)
    {
        if (key == Btn_A7_Enter)
        {
            game_tetris_init(t);
        }
        return;
    }

    if (key == Btn_A4_Left)
    {
        _tet_move(t, -1, 0, 0U);
    }
    else if (key == Btn_A6_Right)
    {
        _tet_move(t, 1, 0, 0U);
    }
    else if (key == Btn_A1_Up)
    {
        _tet_move(t, 0, 0, 1U);
    }
    else if (key == Btn_A5_Down)
    {
        _tet_move(t, 0, 1, 0U);
    }
    else if (key == Btn_A7_Enter)
    {
        while (!t->over && _tet_check(t, t->cur_x, (s16)(t->cur_y + 1), t->cur_rot))
        {
            _tet_move(t, 0, 1, 0U);
        }
        _tet_lock_and_clear(t);
    }
}
