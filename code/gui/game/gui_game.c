#include "..\..\Include\spinmaster.h"

/* 1st-level Game Submenu 2x3 card captions, badge colors, and icons */
static const char *const game_menu_labels[GUI_Game_Count] = {"Snake",  "Tetris", "Brick",
                                                             "Flappy", "Space",  "2048"};

static const char *const game_hud_titles[GUI_Game_Count] = {"SNAKE",  "TETRIS", "BRICK",
                                                            "FLAPPY", "SPACE",  "2048"};

static const u16 game_menu_colors[GUI_Game_Count] = {
    GUI_RGB565(0, 196, 140),  /* Snake  : Emerald Green */
    GUI_RGB565(46, 144, 255), /* Tetris : Dodger Blue */
    GUI_RGB565(235, 64, 122), /* Brick  : Rose Pink */
    GUI_RGB565(255, 168, 0),  /* Flappy : Amber Gold */
    GUI_RGB565(156, 88, 255), /* Space  : Royal Purple */
    GUI_RGB565(0, 184, 212)   /* 2048   : Cyan Teal */
};

static const GUI_Btn_Icon game_menu_icons[GUI_Game_Count] = {GUI_Icon_Snake,   GUI_Icon_Tetris,
                                                             GUI_Icon_Brick,   GUI_Icon_Flappy,
                                                             GUI_Icon_Shooter, GUI_Icon_2048};

/* 1st-level Game Submenu state */
static GUI_Game_Menu_Page *game_menu_page = null;
static HEAP_Mark game_menu_heap_mark = 0U;
static MSG_Sub game_menu_btn_sub = MSG_SUB_INIT;
static u8 game_menu_saved_idx = 0U;

/* Active individual game state */
static HEAP_Mark game_heap_mark = 0U;
static SWT_Timer game_timer;
static MSG_Sub game_btn_sub = MSG_SUB_INIT;
static GUI_Game_Id active_game_id = GUI_Game_Count;
static void *game_ctx = null;
static u16 game_rng_seed = 0xACE1U;
static u8 game_wait_release = 0U;

/*
 *@ brief    : Generate a 16-bit pseudo-random number mixed with the hardware RTC tick counter.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Non-zero 16-bit pseudo-random value.
 *@ theme    : Shared PRNG utility for all game modules.
 */
u16 gui_game_rand(void)
{
    game_rng_seed ^= swt_read_tick();
    game_rng_seed = (u16)((game_rng_seed >> 1U) ^ (-(s16)(game_rng_seed & 1U) & 0xB400U));
    if (game_rng_seed == 0U)
    {
        game_rng_seed = 0x5A3CU;
    }
    return game_rng_seed;
}

/*
 *@ brief    : Play a synthesized chiptune sound effect on SPU_Channel_1.
 *@ type     : public
 *@ argu in  : freq: SPU sample frequency phase-adder value (e.g., GAME_TONE_SCORE).
 *@ argu out : Chiptune pulse wave triggered on SPU_Channel_1.
 *@ theme    : Shared sound-effect helper for all game modules.
 */
void gui_game_sfx(unsigned freq) { spu_play_tone(freq, 0xFFFFU); }

/*
 *@ brief    : Draw the top HUD bar showing the active game's title and initial score.
 *@ type     : public
 *@ argu in  : score: initial score to display.
 *@ argu out : Top 14px HUD bar and bottom divider rendered on LCD.
 *@ theme    : Shared game header bar.
 */
void gui_game_draw_hud(u16 score)
{
    gui_fill_rect(0, 0, (s16)(LCD_WIDTH - 1U), (s16)(GAME_HUD_H - 1U), GAME_HUD_BG);
    gui_draw_hline(0, (s16)(LCD_WIDTH - 1U), (s16)GAME_HUD_H, GAME_BORDER_COLOR);
    if (active_game_id < GUI_Game_Count)
    {
        gui_draw_str_at(4, 3, game_hud_titles[active_game_id], GUI_COLOR_CYAN, GUI_Font_6x8);
    }
    gui_draw_str_at(96, 3, "S:", GUI_COLOR_YELLOW, GUI_Font_6x8);
    gui_draw_uint_at(110, 3, score, 5U, true, GUI_COLOR_WHITE, GUI_Font_6x8);
}

/*
 *@ brief    : Refresh only the numeric score field in the top-right HUD bar.
 *@ type     : public
 *@ argu in  : score: updated score value.
 *@ argu out : 5-digit zero-padded score redrawn without flickering the HUD title.
 *@ theme    : Incremental score readout update.
 */
void gui_game_update_score(u16 score)
{
    gui_fill_rect(110, 2, 155, 11, GAME_HUD_BG);
    gui_draw_uint_at(110, 3, score, 5U, true, GUI_COLOR_WHITE, GUI_Font_6x8);
}

/*
 *@ brief    : Draw a centered Game Over / Victory overlay dialog and play low alert tone.
 *@ type     : public
 *@ argu in  : msg: banner title string (e.g., "GAME OVER" or "YOU WIN!").
 *@ argu out : Rounded overlay dialog with retry/exit instructions drawn.
 *@ theme    : Shared end-of-game modal banner.
 */
void gui_game_draw_over_banner(const char *msg)
{
    gui_fill_round_rect(26, 50, 134, 88, 5U, GUI_RGB565(30, 14, 24));
    gui_draw_round_rect(26, 50, 134, 88, 5U, GUI_COLOR_RED);
    gui_draw_str_at(46, 56, msg, GUI_COLOR_YELLOW, GUI_Font_8x16);
    gui_draw_str_at(34, 75, "A7:Retry A0:Exit", GUI_COLOR_WHITE, GUI_Font_6x8);
    gui_game_sfx(GAME_TONE_LOW);
}

/* ============================================================================
 * 1ST-LEVEL GAME SUBMENU (2x3 Game Center Grid)
 * ============================================================================ */

/*
 *@ brief    : Append an unsigned 16-bit decimal integer into a char buffer.
 *@ type     : private
 *@ argu in  : dst: output buffer; val: unsigned value.
 *@ argu out : Pointer to the null-terminated end of `dst`.
 *@ theme    : Lightweight string builder for submenu heap bar.
 */
static char *_game_menu_append_u16(char *dst, u16 val)
{
    char tmp[6];
    u8 n = 0U;

    do
    {
        tmp[n++] = (char)('0' + (val % 10U));
        val /= 10U;
    } while (val != 0U);

    while (n > 0U)
    {
        *dst++ = tmp[--n];
    }
    *dst = '\0';
    return dst;
}

/*
 *@ brief    : Append a literal C string into a char buffer.
 *@ type     : private
 *@ argu in  : dst: output buffer; src: source literal string.
 *@ argu out : Pointer to the null-terminated end of `dst`.
 *@ theme    : Lightweight string builder helper.
 */
static char *_game_menu_append_str(char *dst, const char *src)
{
    while (*src != '\0')
    {
        *dst++ = *src++;
    }
    *dst = '\0';
    return dst;
}

/*
 *@ brief    : Move focus from the current submenu card to `new_idx` with partial redraw.
 *@ type     : private
 *@ argu in  : new_idx: target game card index (0..GUI_Game_Count-1).
 *@ argu out : Old card redrawn as Normal, new card redrawn as Focused.
 *@ theme    : Flicker-free submenu focus transition.
 */
static void _game_menu_set_focus(u8 new_idx)
{
    u8 old_idx;

    if (game_menu_page == null || new_idx >= GUI_Game_Count)
    {
        return;
    }

    old_idx = game_menu_page->selected_idx;
    if (old_idx == new_idx)
    {
        return;
    }

    game_menu_page->buttons[old_idx].state = GUI_Btn_Normal;
    gui_button_draw(&game_menu_page->buttons[old_idx]);

    game_menu_page->selected_idx = new_idx;
    game_menu_saved_idx = new_idx;
    game_menu_page->buttons[new_idx].state = GUI_Btn_Focused;
    gui_button_draw(&game_menu_page->buttons[new_idx]);
}

/*
 *@ brief    : Handle button messages inside the 1st-level Game Submenu.
 *@ type     : private
 *@ argu in  : message: generic MSG_Button message with BTN_Message payload.
 *@ argu out : Navigates 2x3 game cards, launches selected game on Enter, or returns to Desktop.
 *@ theme    : Submenu key navigation handler.
 */
static void _game_menu_on_button_msg(const MSG_Message *message)
{
    const BTN_Message *btn;

    if (game_menu_page == null || message == null || message->type != MSG_Button)
    {
        return;
    }
    if (message->data == null)
    {
        bsp_halt_fault(BSP_Fault_MSG_Message);
    }

    btn = (const BTN_Message *)message->data;
    if (btn->key == Btn_A0_Back)
    {
        if (btn->event == BTN_Hold)
        {
            bsp_power_off();
        }
        else if (btn->event == BTN_Click)
        {
            spu_play_tone(GAME_TONE_MOVE, 0x5FFFU);
            gui_game_menu_exit();
            gui_desktop_enter();
        }
        return;
    }

    if (btn->event != BTN_Click)
    {
        return;
    }

    if (btn->key == Btn_A4_Left)
    {
        u8 next = (game_menu_page->selected_idx == 0U)
                      ? (u8)(GUI_Game_Count - 1U)
                      : (u8)(game_menu_page->selected_idx - 1U);
        spu_play_tone(GAME_TONE_MOVE, 0x5FFFU);
        _game_menu_set_focus(next);
    }
    else if (btn->key == Btn_A6_Right)
    {
        u8 next = (u8)((game_menu_page->selected_idx + 1U) % GUI_Game_Count);
        spu_play_tone(GAME_TONE_MOVE, 0x5FFFU);
        _game_menu_set_focus(next);
    }
    else if (btn->key == Btn_A1_Up || btn->key == Btn_A5_Down)
    {
        u8 next = (u8)((game_menu_page->selected_idx + DESKTOP_BTN_COLS) % GUI_Game_Count);
        spu_play_tone(GAME_TONE_MOVE, 0x5FFFU);
        _game_menu_set_focus(next);
    }
    else if (btn->key == Btn_A7_Enter)
    {
        u8 launch_id = game_menu_page->selected_idx;
        game_menu_saved_idx = launch_id;
        gui_game_enter((GUI_Game_Id)launch_id);
    }
}

/*
 *@ brief    : Enter the 1st-level Game Submenu: allocate submenu widgets from ram_heap and render.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : 2x3 Game Center submenu rendered on LCD.
 *@ theme    : First-level submenu managed via ram_heap watermark.
 */
void gui_game_menu_enter(void)
{
    u8 r, c, idx;
    HEAP_Stats st;
    u16 pct = 0U;
    char *p;
    s16 cy = (s16)(DESKTOP_TOP_BAR_H / 2U);

    if (game_menu_page != null)
    {
        return;
    }

    gui_desktop_exit();

    game_menu_heap_mark = heap_get_mark();
    game_menu_page = HEAP_NEW(GUI_Game_Menu_Page);
    if (game_menu_page == null)
    {
        bsp_halt_fault(BSP_Fault_HEAP_State);
    }

    if (game_menu_saved_idx >= GUI_Game_Count)
    {
        game_menu_saved_idx = 0U;
    }
    game_menu_page->selected_idx = game_menu_saved_idx;

    gui_label_init(&game_menu_page->top_bar, 0, 0, LCD_WIDTH, DESKTOP_TOP_BAR_H, "Game Center",
                   GUI_Font_6x8);
    game_menu_page->top_bar.align = GUI_Align_Center;
    game_menu_page->top_bar.bg_color = DESKTOP_TOPBAR_BG;
    game_menu_page->top_bar.text_color = GUI_COLOR_YELLOW;
    game_menu_page->top_bar.transparent = false;

    gui_label_init(&game_menu_page->bottom_bar, 0, (s16)(LCD_HEIGHT - DESKTOP_BOTTOM_BAR_H),
                   LCD_WIDTH, DESKTOP_BOTTOM_BAR_H, game_menu_page->heap_info_str, GUI_Font_6x8);
    game_menu_page->bottom_bar.align = GUI_Align_Left;
    game_menu_page->bottom_bar.bg_color = DESKTOP_BOTBAR_BG;
    game_menu_page->bottom_bar.text_color = GUI_RGB565(120, 220, 255);
    game_menu_page->bottom_bar.transparent = false;

    for (r = 0U; r < DESKTOP_BTN_ROWS; ++r)
    {
        for (c = 0U; c < DESKTOP_BTN_COLS; ++c)
        {
            s16 bx = (s16)(DESKTOP_GRID_X0 + c * (DESKTOP_BTN_W + DESKTOP_GAP_X));
            s16 by = (s16)(DESKTOP_GRID_Y0 + r * (DESKTOP_BTN_H + DESKTOP_GAP_Y));
            idx = (u8)(r * DESKTOP_BTN_COLS + c);

            gui_button_init(&game_menu_page->buttons[idx], bx, by, DESKTOP_BTN_W, DESKTOP_BTN_H,
                            game_menu_labels[idx], GUI_Font_6x8);
            game_menu_page->buttons[idx].radius = 6U;
            game_menu_page->buttons[idx].icon = game_menu_icons[idx];
            game_menu_page->buttons[idx].icon_bg_color = game_menu_colors[idx];
            game_menu_page->buttons[idx].state =
                (idx == game_menu_page->selected_idx) ? GUI_Btn_Focused : GUI_Btn_Normal;
        }
    }

    lcd_fill_color(DESKTOP_BG_COLOR);
    gui_label_draw(&game_menu_page->top_bar);
    gui_fill_circle(8, cy, 3U, GUI_COLOR_YELLOW);
    gui_draw_hline(0, (s16)(LCD_WIDTH - 1U), (s16)(DESKTOP_TOP_BAR_H - 1U),
                   GUI_RGB565(52, 72, 110));

    for (idx = 0U; idx < GUI_Game_Count; ++idx)
    {
        gui_button_draw(&game_menu_page->buttons[idx]);
    }

    heap_get_stats(&st);
    if (st.total_words > 0U)
    {
        pct =
            (u16)(((u32)st.free_words * 100UL + (u32)(st.total_words / 2U)) / (u32)st.total_words);
    }
    p = game_menu_page->heap_info_str;
    p = _game_menu_append_str(p, "M:");
    p = _game_menu_append_u16(p, pct);
    _game_menu_append_str(p, "%  A7:Play A0:Back");

    gui_label_set_text(&game_menu_page->bottom_bar, game_menu_page->heap_info_str);
    gui_draw_hline(0, (s16)(LCD_WIDTH - 1U), (s16)(LCD_HEIGHT - DESKTOP_BOTTOM_BAR_H),
                   GUI_RGB565(52, 72, 110));

    msg_subscribe_type(&game_menu_btn_sub, MSG_Button, null, _game_menu_on_button_msg);
}

/*
 *@ brief    : Exit the 1st-level Game Submenu and release its ram_heap memory.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Submenu unsubscribed and widget RAM reclaimed.
 *@ theme    : Zero-fragmentation submenu exit.
 */
void gui_game_menu_exit(void)
{
    if (game_menu_page == null)
    {
        return;
    }

    msg_unsubscribe_node(&game_menu_btn_sub);
    game_menu_page = null;
    heap_release_to_mark(game_menu_heap_mark);
}

/*
 *@ brief    : Check whether the 1st-level Game Submenu is currently active.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if Game Submenu is active.
 *@ theme    : Submenu state query helper.
 */
bool gui_game_menu_is_active(void) { return game_menu_page != null; }

/* ============================================================================
 * INDIVIDUAL GAME DISPATCHER (Timer & Button Message Bus)
 * ============================================================================ */

/*
 *@ brief    : Initialize the currently selected game module.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Active game's init function invoked.
 *@ theme    : Polymorphic game initializer.
 */
static void _game_reset_active(void)
{
    switch (active_game_id)
    {
    case GUI_Game_Snake:
        game_snake_init((Snake_State *)game_ctx);
        break;
    case GUI_Game_Tetris:
        game_tetris_init((Tetris_State *)game_ctx);
        break;
    case GUI_Game_Brick:
        game_brick_init((Brick_State *)game_ctx);
        break;
    case GUI_Game_Flappy:
        game_flappy_init((Flappy_State *)game_ctx);
        break;
    case GUI_Game_Shooter:
        game_shooter_init((Shooter_State *)game_ctx);
        break;
    case GUI_Game_2048:
        game_2048_init((G2048_State *)game_ctx);
        break;
    default:
        break;
    }
}

/*
 *@ brief    : 32FPS software timer callback dispatching frame updates to the active game.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Active game stepped by one frame.
 *@ theme    : Periodic game loop tick.
 */
static void _game_on_timer(void)
{
    if (game_ctx == null)
    {
        return;
    }
    if (game_wait_release > 0U)
    {
        game_wait_release--;
        return;
    }

    switch (active_game_id)
    {
    case GUI_Game_Snake:
        game_snake_step((Snake_State *)game_ctx);
        break;
    case GUI_Game_Tetris:
        game_tetris_step((Tetris_State *)game_ctx);
        break;
    case GUI_Game_Brick:
        game_brick_step((Brick_State *)game_ctx);
        break;
    case GUI_Game_Flappy:
        game_flappy_step((Flappy_State *)game_ctx);
        break;
    case GUI_Game_Shooter:
        game_shooter_step((Shooter_State *)game_ctx);
        break;
    case GUI_Game_2048:
    default:
        break;
    }
}

/*
 *@ brief    : Button message callback while an individual game is running.
 *@ type     : private
 *@ argu in  : message: generic MSG_Button message with BTN_Message payload.
 *@ argu out : Routes key presses to the active game, exits to Game Submenu on A0 click,
 *             or powers off on A0 hold.
 *@ theme    : Game input router.
 */
static void _game_on_button_msg(const MSG_Message *message)
{
    const BTN_Message *btn;

    if (game_ctx == null || message == null || message->type != MSG_Button)
    {
        return;
    }
    btn = (const BTN_Message *)message->data;
    if (btn == null)
    {
        return;
    }

    if (btn->key == Btn_A0_Back)
    {
        if (btn->event == BTN_Hold)
        {
            bsp_power_off();
        }
        else if (btn->event == BTN_Click)
        {
            gui_game_exit();
        }
        return;
    }

    if (btn->event != BTN_Click && btn->event != BTN_Hold)
    {
        return;
    }

    switch (active_game_id)
    {
    case GUI_Game_Snake:
        game_snake_on_key((Snake_State *)game_ctx, btn->key);
        break;
    case GUI_Game_Tetris:
        game_tetris_on_key((Tetris_State *)game_ctx, btn->key);
        break;
    case GUI_Game_Brick:
        game_brick_on_key((Brick_State *)game_ctx, btn->key);
        break;
    case GUI_Game_Flappy:
        game_flappy_on_key((Flappy_State *)game_ctx, btn->key);
        break;
    case GUI_Game_Shooter:
        game_shooter_on_key((Shooter_State *)game_ctx, btn->key);
        break;
    case GUI_Game_2048:
        game_2048_on_key((G2048_State *)game_ctx, btn->key);
        break;
    default:
        break;
    }
}

/*
 *@ brief    : Exit Game Submenu, allocate game state from ram_heap, and start the selected game.
 *@ type     : public
 *@ argu in  : id: GUI_Game_Id (0..5).
 *@ argu out : Selected game running on LCD.
 *@ theme    : Dynamic RAM sharing between submenu and individual games via ram_heap.
 */
void gui_game_enter(GUI_Game_Id id)
{
    u16 ctx_words = 0U;

    if (id >= GUI_Game_Count || game_ctx != null)
    {
        return;
    }

    gui_game_menu_exit();
    gui_desktop_exit();

    game_heap_mark = heap_get_mark();
    switch (id)
    {
    case GUI_Game_Snake:
        ctx_words = (u16)sizeof(Snake_State);
        break;
    case GUI_Game_Tetris:
        ctx_words = (u16)sizeof(Tetris_State);
        break;
    case GUI_Game_Brick:
        ctx_words = (u16)sizeof(Brick_State);
        break;
    case GUI_Game_Flappy:
        ctx_words = (u16)sizeof(Flappy_State);
        break;
    case GUI_Game_Shooter:
        ctx_words = (u16)sizeof(Shooter_State);
        break;
    case GUI_Game_2048:
        ctx_words = (u16)sizeof(G2048_State);
        break;
    default:
        break;
    }

    game_ctx = heap_calloc(ctx_words);
    if (game_ctx == null)
    {
        bsp_halt_fault(BSP_Fault_HEAP_State);
    }

    active_game_id = id;
    game_wait_release = 4U;
    gui_game_sfx(GAME_TONE_SCORE);
    _game_reset_active();

    msg_subscribe_type(&game_btn_sub, MSG_Button, null, _game_on_button_msg);
    swt_start_timer(&game_timer, GAME_TICK_PERIOD, SWT_Repeat, _game_on_timer);
}

/*
 *@ brief    : Stop the active game, release its ram_heap memory, and return to the Game Submenu.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Game memory freed and 1st-level Game Submenu restored.
 *@ theme    : Hierarchical navigation back to the Game Center submenu.
 */
void gui_game_exit(void)
{
    if (game_ctx == null)
    {
        return;
    }

    swt_stop_timer(&game_timer);
    msg_unsubscribe_node(&game_btn_sub);
    spu_stop_channel(SPU_Channel_1);

    game_ctx = null;
    active_game_id = GUI_Game_Count;
    heap_release_to_mark(game_heap_mark);

    gui_game_menu_enter();
}

/*
 *@ brief    : Check whether an individual game is currently running.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if a game is active.
 *@ theme    : Game state query helper.
 */
bool gui_game_is_active(void) { return game_ctx != null; }
