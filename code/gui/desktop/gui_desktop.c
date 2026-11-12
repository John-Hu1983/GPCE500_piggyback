#include "..\..\Include\spinmaster.h"

/* Desktop 2x3 (2 rows x 3 cols) button captions stored in Flash/ROM */
static const char *const desktop_btn_labels[DESKTOP_BTN_COUNT] = {"Music", "Voice", "Vol",
                                                                  "Light", "Timer", "Setup"};

/* PyQt-style circular badge accent colors for each app card */
static const u16 desktop_icon_colors[DESKTOP_BTN_COUNT] = {
    GUI_RGB565(235, 64, 122), /* Music : Rose Pink */
    GUI_RGB565(46, 144, 255), /* Voice : Dodger Blue */
    GUI_RGB565(0, 196, 140),  /* Vol   : Emerald Green */
    GUI_RGB565(255, 168, 0),  /* Light : Amber Gold */
    GUI_RGB565(156, 88, 255), /* Timer : Royal Purple */
    GUI_RGB565(0, 184, 212)   /* Setup : Cyan Teal */
};

/* Vector icon types for each of the 6 desktop cards */
static const GUI_Btn_Icon desktop_btn_icons[DESKTOP_BTN_COUNT] = {GUI_Icon_Music,  GUI_Icon_Voice,
                                                                  GUI_Icon_Volume, GUI_Icon_Light,
                                                                  GUI_Icon_Timer,  GUI_Icon_Setup};

static GUI_Desktop_Page *desktop_page = null;
static HEAP_Mark desktop_heap_mark = 0U;
static MSG_Sub desktop_btn_sub = MSG_SUB_INIT;

/*
 *@ brief    : Append an unsigned 16-bit decimal integer into a char buffer.
 *@ type     : private
 *@ argu in  : dst: output buffer; val: unsigned value.
 *@ argu out : Pointer to the null-terminated end of `dst`.
 *@ theme    : Lightweight string builder for heap status formatting.
 */
static char *_desktop_append_u16(char *dst, u16 val)
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
static char *_desktop_append_str(char *dst, const char *src)
{
    while (*src != '\0')
    {
        *dst++ = *src++;
    }
    *dst = '\0';
    return dst;
}

/*
 *@ brief    : Query ram_heap statistics and refresh the bottom bar GUI_Label with free percentage.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Bottom bar displays "m: **%" and top divider line.
 *@ theme    : Compact RAM pool free-percentage indicator via GUI_Label widget.
 */
void gui_desktop_refresh_heap_bar(void)
{
    HEAP_Stats st;
    u16 pct = 0U;
    char *p;

    if (desktop_page == null)
    {
        return;
    }

    heap_get_stats(&st);
    if (st.total_words > 0U)
    {
        pct =
            (u16)(((u32)st.free_words * 100UL + (u32)(st.total_words / 2U)) / (u32)st.total_words);
    }

    p = desktop_page->heap_info_str;
    p = _desktop_append_str(p, "M:");
    p = _desktop_append_u16(p, pct);
    _desktop_append_str(p, "%");

    gui_label_set_text(&desktop_page->bottom_bar, desktop_page->heap_info_str);
    gui_draw_hline(0, (s16)(LCD_WIDTH - 1U), (s16)(LCD_HEIGHT - DESKTOP_BOTTOM_BAR_H),
                   GUI_RGB565(52, 72, 110));
}

/*
 *@ brief    : Handle button messages while the desktop page is active.
 *@ type     : private
 *@ argu in  : message: generic MSG_Button message with BTN_Message payload.
 *@ argu out : Btn_A4_Left/Btn_A5_Down move focus prev; Btn_A6_Right/Btn_A1_Up move focus next.
 *@ theme    : Message-bus-driven desktop navigation.
 */
static void _desktop_on_button_msg(const MSG_Message *message)
{
    const BTN_Message *btn;

    if (desktop_page == null || message == null || message->type != MSG_Button)
    {
        return;
    }
    if (message->data == null)
    {
        bsp_halt_fault(BSP_Fault_MSG_Message);
    }

    btn = (const BTN_Message *)message->data;
    if (btn->event != BTN_Click && btn->event != BTN_Hold)
    {
        return;
    }

    if (btn->key == Btn_A4_Left && btn->event == BTN_Click)
    {
        gui_desktop_select_prev();
    }
    else if (btn->key == Btn_A6_Right && btn->event == BTN_Click)
    {
        gui_desktop_select_next();
    }
    else if (btn->key == Btn_A0_Back && btn->event == BTN_Hold)
    {
        bsp_power_off();
    }
}

/*
 *@ brief    : Move focus from the current button to `new_idx` and redraw only the changed widgets.
 *@ type     : private
 *@ argu in  : new_idx: target button index (0..DESKTOP_BTN_COUNT-1).
 *@ argu out : Old button redrawn as Normal, new button as Focused, bottom bar updated.
 *@ theme    : Partial dirty-widget redraw to avoid full-screen flicker.
 */
static void _desktop_set_focus(u8 new_idx)
{
    u8 old_idx;

    if (desktop_page == null || new_idx >= DESKTOP_BTN_COUNT)
    {
        return;
    }

    old_idx = desktop_page->selected_idx;
    if (old_idx == new_idx)
    {
        return;
    }

    desktop_page->buttons[old_idx].state = GUI_Btn_Normal;
    gui_button_draw(&desktop_page->buttons[old_idx]);

    desktop_page->selected_idx = new_idx;
    desktop_page->buttons[new_idx].state = GUI_Btn_Focused;
    gui_button_draw(&desktop_page->buttons[new_idx]);
}

/*
 *@ brief    : Move selection to the left / previous button in the 3x2 grid (wrap around 0 -> 5).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Focus moved left and screen updated.
 *@ theme    : Bound to BTN_Key_5.
 */
void gui_desktop_select_prev(void)
{
    u8 next;

    if (desktop_page == null)
    {
        return;
    }
    if (desktop_page->selected_idx == 0U)
    {
        next = (u8)(DESKTOP_BTN_COUNT - 1U);
    }
    else
    {
        next = (u8)(desktop_page->selected_idx - 1U);
    }
    _desktop_set_focus(next);
}

/*
 *@ brief    : Move selection to the right / next button in the 3x2 grid (wrap around 5 -> 0).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Focus moved right and screen updated.
 *@ theme    : Bound to BTN_Key_6.
 */
void gui_desktop_select_next(void)
{
    u8 next;

    if (desktop_page == null)
    {
        return;
    }
    next = (u8)(desktop_page->selected_idx + 1U);
    if (next >= DESKTOP_BTN_COUNT)
    {
        next = 0U;
    }
    _desktop_set_focus(next);
}

/*
 *@ brief    : Draw the PyQt-style top status bar with title and battery indicator icon.
 *@ type     : private
 *@ argu in  : top_bar: pointer to top bar label.
 *@ argu out : Top bar, battery icon, and bottom accent line drawn.
 *@ theme    : Modern PyQt status header dynamically positioned for any LCD_WIDTH.
 */
static void _desktop_draw_top_bar(const GUI_Label *top_bar)
{
    s16 bat_x = (s16)(LCD_WIDTH - 22U);
    s16 cy = (s16)(DESKTOP_TOP_BAR_H / 2U);

    gui_label_draw(top_bar);

    /* Small green status LED dot on the left */
    gui_fill_circle(8, cy, 3U, GUI_RGB565(0, 230, 118));

    /* Battery icon anchored to the right edge of the top bar */
    gui_draw_rect(bat_x, (s16)(cy - 4), (s16)(bat_x + 14), (s16)(cy + 3), GUI_COLOR_WHITE);
    gui_fill_rect((s16)(bat_x + 15), (s16)(cy - 2), (s16)(bat_x + 16), (s16)(cy + 1),
                  GUI_COLOR_WHITE);
    gui_fill_rect((s16)(bat_x + 2), (s16)(cy - 2), (s16)(bat_x + 11), (s16)(cy + 1),
                  GUI_RGB565(0, 230, 118));

    /* Subtle bottom divider line */
    gui_draw_hline(0, (s16)(LCD_WIDTH - 1U), (s16)(DESKTOP_TOP_BAR_H - 1U),
                   GUI_RGB565(52, 72, 110));
}

/*
 *@ brief    : Enter the desktop UI: allocate widgets from ram_heap, draw layout, and subscribe
 * keys.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Top bar, 2x3 PyQt card grid, and bottom heap bar rendered on LCD.
 *@ theme    : Dynamic widget allocation from ram_heap with watermark tracking.
 */
void gui_desktop_enter(void)
{
    u8 r, c, idx;

    if (desktop_page != null)
    {
        return;
    }

    /* 1. Save heap watermark and allocate the entire desktop widget tree from ram_heap */
    desktop_heap_mark = heap_get_mark();
    desktop_page = HEAP_NEW(GUI_Desktop_Page);
    if (desktop_page == null)
    {
        bsp_halt_fault(BSP_Fault_HEAP_State);
    }

    desktop_page->selected_idx = 0U;

    /* 2. Configure Top Bar using GUI_Label with smallest font (GUI_Font_6x8) */
    gui_label_init(&desktop_page->top_bar, 0, 0, LCD_WIDTH, DESKTOP_TOP_BAR_H, "Spinmaster OS",
                   GUI_Font_6x8);
    desktop_page->top_bar.align = GUI_Align_Center;
    desktop_page->top_bar.bg_color = DESKTOP_TOPBAR_BG;
    desktop_page->top_bar.text_color = GUI_COLOR_WHITE;
    desktop_page->top_bar.transparent = false;

    /* 3. Configure Bottom Bar using GUI_Label to display ram_heap free percentage on the left */
    gui_label_init(&desktop_page->bottom_bar, 0, (s16)(LCD_HEIGHT - DESKTOP_BOTTOM_BAR_H),
                   LCD_WIDTH, DESKTOP_BOTTOM_BAR_H, desktop_page->heap_info_str, GUI_Font_6x8);
    desktop_page->bottom_bar.align = GUI_Align_Left;
    desktop_page->bottom_bar.bg_color = DESKTOP_BOTBAR_BG;
    desktop_page->bottom_bar.text_color = GUI_RGB565(120, 220, 255);
    desktop_page->bottom_bar.transparent = false;

    /* 4. Configure 2 rows x 3 columns of PyQt-style icon card buttons in the middle area */
    for (r = 0U; r < DESKTOP_BTN_ROWS; ++r)
    {
        for (c = 0U; c < DESKTOP_BTN_COLS; ++c)
        {
            s16 bx = (s16)(DESKTOP_GRID_X0 + c * (DESKTOP_BTN_W + DESKTOP_GAP_X));
            s16 by = (s16)(DESKTOP_GRID_Y0 + r * (DESKTOP_BTN_H + DESKTOP_GAP_Y));
            idx = (u8)(r * DESKTOP_BTN_COLS + c);

            gui_button_init(&desktop_page->buttons[idx], bx, by, DESKTOP_BTN_W, DESKTOP_BTN_H,
                            desktop_btn_labels[idx], GUI_Font_6x8);
            desktop_page->buttons[idx].radius = 6U;
            desktop_page->buttons[idx].icon = desktop_btn_icons[idx];
            desktop_page->buttons[idx].icon_bg_color = desktop_icon_colors[idx];
            desktop_page->buttons[idx].state = (idx == 0U) ? GUI_Btn_Focused : GUI_Btn_Normal;
        }
    }

    /* 5. Render full PyQt dark-slate desktop layout */
    lcd_fill_color(DESKTOP_BG_COLOR);
    _desktop_draw_top_bar(&desktop_page->top_bar);

    for (idx = 0U; idx < DESKTOP_BTN_COUNT; ++idx)
    {
        gui_button_draw(&desktop_page->buttons[idx]);
    }

    gui_desktop_refresh_heap_bar();

    /* 6. Subscribe to button messages for left/right selection */
    msg_subscribe_type(&desktop_btn_sub, MSG_Button, null, _desktop_on_button_msg);
}

/*
 *@ brief    : Exit the desktop UI: unsubscribe button events and release all widget RAM to heap.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : All desktop widget resources reclaimed back to ram_heap.
 *@ theme    : Zero-fragmentation page exit via heap_release_to_mark().
 */
void gui_desktop_exit(void)
{
    if (desktop_page == null)
    {
        return;
    }

    msg_unsubscribe_node(&desktop_btn_sub);
    desktop_page = null;
    heap_release_to_mark(desktop_heap_mark);
}

/*
 *@ brief    : Check whether the desktop UI is currently active and allocated.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if desktop is active.
 *@ theme    : State query helper.
 */
bool gui_desktop_is_active(void) { return desktop_page != null; }
