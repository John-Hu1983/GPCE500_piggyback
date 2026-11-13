#include "..\..\..\Include\spinmaster.h"

static const u16 g2048_colors[12] = {
    GUI_RGB565(32, 40, 58),   GUI_RGB565(68, 92, 136), GUI_RGB565(46, 134, 193),
    GUI_RGB565(230, 126, 34), GUI_RGB565(231, 76, 60), GUI_RGB565(155, 89, 182),
    GUI_RGB565(39, 174, 96),  GUI_RGB565(241, 196, 15), GUI_RGB565(211, 84, 0),
    GUI_RGB565(192, 57, 43),  GUI_RGB565(142, 68, 173), GUI_RGB565(255, 215, 0)};

/*
 *@ brief    : Render all 16 tiles of the 4x4 2048 puzzle grid.
 *@ type     : private
 *@ argu in  : g: pointer to active G2048_State.
 *@ argu out : Rounded colored number tiles drawn on LCD.
 *@ theme    : 2048 board renderer.
 */
static void _g2048_draw_board(const G2048_State *g)
{
    u8 r, c;

    for (r = 0U; r < 4U; ++r)
    {
        for (c = 0U; c < 4U; ++c)
        {
            u8 p = g->grid[r][c];
            u16 bg = (p < 12U) ? g2048_colors[p] : GUI_COLOR_YELLOW;
            s16 x = (s16)(18 + c * 32);
            s16 y = (s16)(20 + r * 26);

            gui_fill_round_rect(x, y, (s16)(x + 29), (s16)(y + 23), 3U, bg);
            if (p > 0U)
            {
                u16 val = (u16)(1U << p);
                s16 tx = (val >= 1000U) ? (s16)(x + 3)
                                        : ((val >= 100U) ? (s16)(x + 6)
                                                         : ((val >= 10U) ? (s16)(x + 9)
                                                                         : (s16)(x + 12)));
                gui_draw_uint_at(tx, (s16)(y + 8), val, 0U, false, GUI_COLOR_WHITE, GUI_Font_6x8);
            }
        }
    }
}

/*
 *@ brief    : Spawn a new tile (2 or 4) on a randomly chosen empty cell of the 4x4 board.
 *@ type     : private
 *@ argu in  : g: pointer to active G2048_State.
 *@ argu out : One empty cell populated with log2 value 1 (2) or 2 (4).
 *@ theme    : Random tile spawner for 2048.
 */
static void _g2048_spawn(G2048_State *g)
{
    u8 empty_r[16], empty_c[16];
    u8 cnt = 0U, r, c;

    for (r = 0U; r < 4U; ++r)
    {
        for (c = 0U; c < 4U; ++c)
        {
            if (g->grid[r][c] == 0U)
            {
                empty_r[cnt] = r;
                empty_c[cnt] = c;
                cnt++;
            }
        }
    }
    if (cnt > 0U)
    {
        u8 pick = (u8)(gui_game_rand() % cnt);
        g->grid[empty_r[pick]][empty_c[pick]] = ((gui_game_rand() & 7U) == 0U) ? 2U : 1U;
    }
}

/*
 *@ brief    : Slide and merge all 4 rows/columns in direction (dx, dy).
 *@ type     : private
 *@ argu in  : g: pointer to active G2048_State; dx, dy: slide unit vector.
 *@ argu out : Tiles compacted/merged, new tile spawned if moved, board and score redrawn.
 *@ theme    : Core 2048 slide-and-merge algorithm.
 */
static void _g2048_slide(G2048_State *g, s16 dx, s16 dy)
{
    bool moved = false;
    bool merged_sfx = false;
    u8 i, j;

    if (g->over)
    {
        return;
    }

    for (i = 0U; i < 4U; ++i)
    {
        u8 line[4] = {0U, 0U, 0U, 0U};
        u8 out[4] = {0U, 0U, 0U, 0U};
        u8 pos = 0U;

        for (j = 0U; j < 4U; ++j)
        {
            u8 r = (dy != 0) ? ((dy > 0) ? (u8)(3U - j) : j) : i;
            u8 c = (dx != 0) ? ((dx > 0) ? (u8)(3U - j) : j) : i;
            if (g->grid[r][c] != 0U)
            {
                line[pos++] = g->grid[r][c];
            }
        }

        pos = 0U;
        j = 0U;
        while (j < 4U && line[j] != 0U)
        {
            if (j + 1U < 4U && line[j] == line[j + 1U])
            {
                out[pos++] = (u8)(line[j] + 1U);
                g->score = (u16)(g->score + (u16)(1U << (line[j] + 1U)));
                merged_sfx = true;
                j += 2U;
            }
            else
            {
                out[pos++] = line[j];
                j++;
            }
        }

        for (j = 0U; j < 4U; ++j)
        {
            u8 r = (dy != 0) ? ((dy > 0) ? (u8)(3U - j) : j) : i;
            u8 c = (dx != 0) ? ((dx > 0) ? (u8)(3U - j) : j) : i;
            if (g->grid[r][c] != out[j])
            {
                g->grid[r][c] = out[j];
                moved = true;
            }
        }
    }

    if (moved)
    {
        _g2048_spawn(g);
        _g2048_draw_board(g);
        gui_game_update_score(g->score);
        gui_game_sfx(merged_sfx ? GAME_TONE_SCORE : GAME_TONE_MOVE);
    }
}

/*
 *@ brief    : Initialize or reset the 2048 game state and draw the initial 4x4 board.
 *@ type     : public
 *@ argu in  : g: pointer to allocated G2048_State.
 *@ argu out : Board cleared, 2 initial tiles spawned, and grid rendered.
 *@ theme    : 2048 puzzle initialization.
 */
void game_2048_init(G2048_State *g)
{
    u8 r, c;

    for (r = 0U; r < 4U; ++r)
    {
        for (c = 0U; c < 4U; ++c)
        {
            g->grid[r][c] = 0U;
        }
    }
    g->score = 0U;
    g->over = false;
    _g2048_spawn(g);
    _g2048_spawn(g);

    lcd_fill_color(GAME_BG_COLOR);
    gui_game_draw_hud(0U);
    gui_draw_round_rect(14, 17, 147, 125, 4U, GAME_BORDER_COLOR);
    _g2048_draw_board(g);
}

/*
 *@ brief    : Handle 4-way slide and restart key events for 2048.
 *@ type     : public
 *@ argu in  : g: pointer to active G2048_State; key: pressed BTN_Id.
 *@ argu out : Board slid/merged in chosen direction or reset on Enter.
 *@ theme    : 2048 key input handler.
 */
void game_2048_on_key(G2048_State *g, BTN_Id key)
{
    if (key == Btn_A7_Enter)
    {
        game_2048_init(g);
    }
    else if (key == Btn_A4_Left)
    {
        _g2048_slide(g, -1, 0);
    }
    else if (key == Btn_A6_Right)
    {
        _g2048_slide(g, 1, 0);
    }
    else if (key == Btn_A1_Up)
    {
        _g2048_slide(g, 0, -1);
    }
    else if (key == Btn_A5_Down)
    {
        _g2048_slide(g, 0, 1);
    }
}
