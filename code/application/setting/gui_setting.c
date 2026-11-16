#include "..\..\Include\spinmaster.h"

static GUI_Setting_Page *setting_page = null;
static HEAP_Mark setting_heap_mark = 0U;
static MSG_Sub setting_btn_sub = MSG_SUB_INIT;
static s16 setting_bl_val = (s16)LCD_BL_DEFAULT_PERCENT;
static s16 setting_vol_val = (s16)AUDIO_VOLUME_DEFAULT;
static u8 setting_saved_idx = (u8)GUI_Setting_Brightness;

/*
 *@ brief    : Append an unsigned 16-bit decimal integer into a char buffer.
 *@ type     : private
 *@ argu in  : dst: output buffer; val: unsigned value.
 *@ argu out : Pointer to null-terminated end of `dst`.
 *@ theme    : Compact numeric string formatter without sprintf.
 */
static char *_setting_append_u16(char *dst, u16 val)
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
 *@ argu out : Pointer to null-terminated end of `dst`.
 *@ theme    : Compact string builder helper.
 */
static char *_setting_append_str(char *dst, const char *src)
{
    while (*src != '\0')
    {
        *dst++ = *src++;
    }
    *dst = '\0';
    return dst;
}

/*
 *@ brief    : Draw only the numeric brightness percentage ("xx%") in the Brightness card header.
 *@ type     : private
 *@ argu in  : focused: true if Brightness card is currently focused.
 *@ argu out : Right-aligned percentage text updated on LCD.
 *@ theme    : Flicker-free partial readout update during slider adjustment.
 */
static void _setting_draw_bl_value(bool focused)
{
    u16 card_bg = focused ? SETTING_CARD_BG_FOCUS : SETTING_CARD_BG_NORMAL;
    s16 text_y = (s16)(SETTING_BL_CARD_Y + 10);
    char buf[8];
    char *p = buf;
    u16 w;

    p = _setting_append_u16(p, (u16)setting_bl_val);
    _setting_append_str(p, "%");

    gui_fill_rect(112, (s16)(text_y - 1), 144, (s16)(text_y + 8), card_bg);
    w = gui_font_get_str_width(buf, GUI_Font_6x8);
    gui_draw_str_at((s16)(144 - (s16)w), text_y, buf, GUI_RGB565(255, 200, 60), GUI_Font_6x8);
}

/*
 *@ brief    : Draw only the numeric volume level ("xx/15") in the Volume card header.
 *@ type     : private
 *@ argu in  : focused: true if Volume card is currently focused.
 *@ argu out : Right-aligned volume ratio updated on LCD.
 *@ theme    : Flicker-free partial readout update during slider adjustment.
 */
static void _setting_draw_vol_value(bool focused)
{
    u16 card_bg = focused ? SETTING_CARD_BG_FOCUS : SETTING_CARD_BG_NORMAL;
    s16 text_y = (s16)(SETTING_VOL_CARD_Y + 10);
    char buf[8];
    char *p = buf;
    u16 w;

    p = _setting_append_u16(p, (u16)setting_vol_val);
    _setting_append_str(p, "/15");

    gui_fill_rect(106, (s16)(text_y - 1), 144, (s16)(text_y + 8), card_bg);
    w = gui_font_get_str_width(buf, GUI_Font_6x8);
    gui_draw_str_at((s16)(144 - (s16)w), text_y, buf, GUI_RGB565(80, 240, 180), GUI_Font_6x8);
}

/*
 *@ brief    : Render one complete setting card (background, border, icon, caption, value, slider).
 *@ type     : private
 *@ argu in  : item: GUI_Setting_Brightness or GUI_Setting_Volume.
 *@ argu out : Setting card rendered on LCD.
 *@ theme    : Card-based parameter control without top/bottom bars.
 */
static void _setting_draw_card(GUI_Setting_Item item)
{
    bool focused;
    u16 card_bg;
    u16 border_col;
    s16 y0, y1, x1;

    if (setting_page == null)
    {
        return;
    }

    focused = (setting_page->selected_idx == (u8)item);
    card_bg = focused ? SETTING_CARD_BG_FOCUS : SETTING_CARD_BG_NORMAL;
    border_col = focused ? SETTING_CARD_BORDER_FOCUS : SETTING_CARD_BORDER_NORMAL;
    x1 = (s16)(SETTING_CARD_X + (s16)SETTING_CARD_W - 1);

    if (item == GUI_Setting_Brightness)
    {
        y0 = SETTING_BL_CARD_Y;
        y1 = (s16)(y0 + (s16)SETTING_CARD_H - 1);

        gui_fill_round_rect(SETTING_CARD_X, y0, x1, y1, SETTING_CARD_RADIUS, card_bg);
        gui_draw_round_rect(SETTING_CARD_X, y0, x1, y1, SETTING_CARD_RADIUS, border_col);
        if (focused)
        {
            gui_draw_round_rect((s16)(SETTING_CARD_X + 1), (s16)(y0 + 1), (s16)(x1 - 1),
                                (s16)(y1 - 1), (u16)(SETTING_CARD_RADIUS - 1U), border_col);
        }

        gui_icon_draw(GUI_Icon_Light, 22, (s16)(y0 + 14), card_bg, GUI_RGB565(255, 184, 0));
        gui_draw_str_at(34, (s16)(y0 + 10), "Brightness",
                        focused ? GUI_COLOR_WHITE : GUI_RGB565(190, 205, 230), GUI_Font_6x8);
        _setting_draw_bl_value(focused);

        setting_page->bl_slider.focused = focused;
        setting_page->bl_slider.page_bg_color = card_bg;
        gui_slider_draw(&setting_page->bl_slider);
    }
    else
    {
        y0 = SETTING_VOL_CARD_Y;
        y1 = (s16)(y0 + (s16)SETTING_CARD_H - 1);

        gui_fill_round_rect(SETTING_CARD_X, y0, x1, y1, SETTING_CARD_RADIUS, card_bg);
        gui_draw_round_rect(SETTING_CARD_X, y0, x1, y1, SETTING_CARD_RADIUS, border_col);
        if (focused)
        {
            gui_draw_round_rect((s16)(SETTING_CARD_X + 1), (s16)(y0 + 1), (s16)(x1 - 1),
                                (s16)(y1 - 1), (u16)(SETTING_CARD_RADIUS - 1U), border_col);
        }

        gui_icon_draw(GUI_Icon_Volume, 22, (s16)(y0 + 14), card_bg, GUI_RGB565(0, 216, 150));
        gui_draw_str_at(34, (s16)(y0 + 10), "Volume",
                        focused ? GUI_COLOR_WHITE : GUI_RGB565(190, 205, 230), GUI_Font_6x8);
        _setting_draw_vol_value(focused);

        setting_page->vol_slider.focused = focused;
        setting_page->vol_slider.page_bg_color = card_bg;
        gui_slider_draw(&setting_page->vol_slider);
    }
}

/*
 *@ brief    : Switch focus between Brightness and Volume items and redraw both cards.
 *@ type     : private
 *@ argu in  : new_idx: target GUI_Setting_Item index.
 *@ argu out : Focus updated and cards redrawn.
 *@ theme    : Up/Down item selection handler.
 */
static void _setting_set_focus(u8 new_idx)
{
    u8 old_idx;

    if (setting_page == null || new_idx >= (u8)GUI_Setting_Count)
    {
        return;
    }

    old_idx = setting_page->selected_idx;
    if (old_idx == new_idx)
    {
        return;
    }

    setting_page->selected_idx = new_idx;
    setting_saved_idx = new_idx;
    _setting_draw_card((GUI_Setting_Item)old_idx);
    _setting_draw_card((GUI_Setting_Item)new_idx);
}

/*
 *@ brief    : Adjust the currently focused setting item by `dir` (-1 or +1).
 *@ type     : private
 *@ argu in  : dir: -1 for Left (decrease), +1 for Right (increase).
 *@ argu out : Slider stepped; backlight updated in real time or volume updated with prompt tone.
 *@ theme    : Responds to both BTN_Click and periodic BTN_Long_Press events.
 */
static void _setting_adjust_active(s16 dir)
{
    if (setting_page == null)
    {
        return;
    }

    if (setting_page->selected_idx == (u8)GUI_Setting_Brightness)
    {
        s16 old_val = setting_page->bl_slider.value;
        gui_slider_step(&setting_page->bl_slider, dir);
        setting_bl_val = setting_page->bl_slider.value;
        if (setting_bl_val != old_val)
        {
            lcd_set_backlight((u16)setting_bl_val);
            _setting_draw_bl_value(true);
        }
    }
    else
    {
        gui_slider_step(&setting_page->vol_slider, dir);
        setting_vol_val = setting_page->vol_slider.value;
        _setting_draw_vol_value(true);

        if (setting_vol_val <= 0)
        {
            spu_stop_channel(SPU_Channel_1);
            spu_set_volume(0U);
        }
        else
        {
            spu_set_volume((unsigned)setting_vol_val);
            spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_SCORE, TONE_VOL_MAX);
        }
    }
}

/*
 *@ brief    : Handle button messages while the Settings page is active.
 *@ type     : private
 *@ argu in  : message: MSG_Button message with BTN_Message payload.
 *@ argu out : Up/Down selects item; Left/Right (Click or Long_Press) adjusts parameter;
 *             Back returns to Desktop (Click) or powers off (Hold).
 *@ theme    : Settings page input router.
 */
static void _setting_on_button_msg(const MSG_Message *message)
{
    const BTN_Message *btn;

    if (setting_page == null || message == null || message->type != MSG_Button)
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
            if (setting_vol_val > 0)
            {
                spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
            }
            gui_setting_exit();
            gui_desktop_enter();
        }
        return;
    }

    if (btn->event != BTN_Click && btn->event != BTN_Long_Press)
    {
        return;
    }

    if (btn->key == Btn_A1_Up || btn->key == Btn_A5_Down)
    {
        u8 next = (u8)((setting_page->selected_idx + 1U) % (u8)GUI_Setting_Count);
        if (setting_vol_val > 0)
        {
            spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
        }
        _setting_set_focus(next);
    }
    else if (btn->key == Btn_A4_Left)
    {
        _setting_adjust_active(-1);
    }
    else if (btn->key == Btn_A6_Right)
    {
        _setting_adjust_active(1);
    }
}

/*
 *@ brief    : Enter the Settings UI with a specific item pre-selected.
 *@ type     : public
 *@ argu in  : item: GUI_Setting_Brightness or GUI_Setting_Volume.
 *@ argu out : Full-screen Settings cards and sliders rendered without top/bottom bars.
 *@ theme    : Dynamic widget allocation from ram_heap with watermark tracking.
 */
void gui_setting_enter_item(GUI_Setting_Item item)
{
    if (setting_page != null)
    {
        return;
    }

    gui_desktop_exit();

    setting_heap_mark = heap_get_mark();
    setting_page = HEAP_NEW(GUI_Setting_Page);
    if (setting_page == null)
    {
        bsp_halt_fault(BSP_Fault_HEAP_State);
    }

    if ((u8)item < (u8)GUI_Setting_Count)
    {
        setting_saved_idx = (u8)item;
    }
    else if (setting_saved_idx >= (u8)GUI_Setting_Count)
    {
        setting_saved_idx = (u8)GUI_Setting_Brightness;
    }
    setting_page->selected_idx = setting_saved_idx;

    /* 1. Initialize Brightness Slider (5% .. 100%, step 5%) */
    gui_slider_init(&setting_page->bl_slider, SETTING_SLIDER_X, (s16)(SETTING_BL_CARD_Y + 28),
                    SETTING_SLIDER_W, SETTING_SLIDER_H, SETTING_BL_MIN, SETTING_BL_MAX,
                    setting_bl_val);
    setting_page->bl_slider.step = SETTING_BL_STEP;
    setting_page->bl_slider.track_fill_color = GUI_RGB565(255, 184, 0);
    setting_page->bl_slider.track_bg_color = GUI_RGB565(18, 24, 38);

    /* 2. Initialize Volume Slider (0 .. 15, step 1) */
    gui_slider_init(&setting_page->vol_slider, SETTING_SLIDER_X, (s16)(SETTING_VOL_CARD_Y + 28),
                    SETTING_SLIDER_W, SETTING_SLIDER_H, SETTING_VOL_MIN, SETTING_VOL_MAX,
                    setting_vol_val);
    setting_page->vol_slider.step = SETTING_VOL_STEP;
    setting_page->vol_slider.track_fill_color = GUI_RGB565(0, 216, 150);
    setting_page->vol_slider.track_bg_color = GUI_RGB565(18, 24, 38);

    /* 3. Render full-screen Settings layout (no top bar, no bottom bar) */
    lcd_fill_color(SETTING_BG_COLOR);
    _setting_draw_card(GUI_Setting_Brightness);
    _setting_draw_card(GUI_Setting_Volume);

    /* 4. Subscribe to button messages */
    msg_subscribe_type(&setting_btn_sub, MSG_Button, null, _setting_on_button_msg);
}

/*
 *@ brief    : Enter the Settings UI keeping the last focused item.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Settings page rendered on LCD.
 *@ theme    : Default entry point for Setup card.
 */
void gui_setting_enter(void) { gui_setting_enter_item((GUI_Setting_Item)setting_saved_idx); }

/*
 *@ brief    : Exit the Settings UI and release all widget RAM back to ram_heap.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Button subscription detached and heap memory reclaimed.
 *@ theme    : Zero-fragmentation page exit via heap_release_to_mark().
 */
void gui_setting_exit(void)
{
    if (setting_page == null)
    {
        return;
    }

    msg_unsubscribe_node(&setting_btn_sub);
    setting_page = null;
    heap_release_to_mark(setting_heap_mark);
}

/*
 *@ brief    : Check whether the Settings UI is currently active.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if Settings page is active.
 *@ theme    : State query helper.
 */
bool gui_setting_is_active(void) { return setting_page != null; }

/*
 *@ brief    : Get the current system backlight brightness percentage (5..100%).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Current brightness percentage.
 *@ theme    : System parameter query.
 */
u16 gui_setting_get_brightness(void) { return (u16)setting_bl_val; }

/*
 *@ brief    : Get the current system audio volume level (0..15).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Current volume level.
 *@ theme    : System parameter query.
 */
u8 gui_setting_get_volume(void) { return (u8)setting_vol_val; }
