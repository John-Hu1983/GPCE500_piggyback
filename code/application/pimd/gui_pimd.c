#include "..\..\Include\spinmaster.h"

static const char *const pimd_btn_labels[PIMD_BTN_COUNT] = {"Start", "Stop", "Quit"};

static const gpio_pin_s Pimd_En_Pin = PIMD_EN_PIN;
static GUI_Pimd_Page *pimd_page = null;
static HEAP_Mark pimd_heap_mark = 0U;
static SWT_Timer pimd_timer;
static MSG_Sub pimd_btn_sub = MSG_SUB_INIT;

static void _pimd_pulse_timer_cb(void);

/*
 *@ brief    : Draw the upper status card showing PIMD running state and 10ms/90ms pulse diagram.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Status badge and 10ms HIGH / 90ms LOW waveform diagram rendered on LCD.
 *@ theme    : Visual feedback for IOB1 (PIMD_EN) pulse output without top/bottom bars.
 */
static void _pimd_draw_status_panel(void)
{
    u16 state_bg;
    u16 wave_col;

    if (pimd_page == null)
    {
        return;
    }

    gui_fill_round_rect(6, 8, (s16)(LCD_WIDTH - 7U), 90, 6U, PIMD_PANEL_BG);
    gui_draw_round_rect(6, 8, (s16)(LCD_WIDTH - 7U), 90, 6U, PIMD_PANEL_BORDER);

    /* Title and active state badge */
    gui_icon_draw(GUI_Icon_Pimd, 20, 22, PIMD_PANEL_BG, GUI_RGB565(0, 220, 160));
    gui_draw_str_at(32, 18, "PIMD  IOB1", GUI_COLOR_WHITE, GUI_Font_6x8);

    if (pimd_page->running)
    {
        state_bg = GUI_RGB565(0, 168, 96);
        wave_col = GUI_RGB565(0, 240, 160);
        gui_fill_round_rect(104, 14, 146, 28, 3U, state_bg);
        gui_draw_str_at(113, 17, "RUN", GUI_COLOR_WHITE, GUI_Font_6x8);
    }
    else
    {
        state_bg = GUI_RGB565(70, 78, 96);
        wave_col = GUI_RGB565(110, 128, 160);
        gui_fill_round_rect(104, 14, 146, 28, 3U, state_bg);
        gui_draw_str_at(110, 17, "STOP", GUI_COLOR_WHITE, GUI_Font_6x8);
    }

    /* 100ms duty-cycle timing diagram (10ms HIGH, 90ms LOW) */
    gui_draw_str_at(16, 38, "T:100ms (10ms H / 90ms L)", GUI_RGB565(170, 205, 245), GUI_Font_6x8);

    /* Draw two pulse periods across x = 18..142, y = 56..78 */
    gui_draw_hline(18, 22, 76, wave_col);
    gui_draw_vline(22, 56, 76, wave_col);
    gui_draw_hline(22, 30, 56, wave_col);
    gui_draw_vline(30, 56, 76, wave_col);
    gui_draw_hline(30, 82, 76, wave_col);

    gui_draw_vline(82, 56, 76, wave_col);
    gui_draw_hline(82, 90, 56, wave_col);
    gui_draw_vline(90, 56, 76, wave_col);
    gui_draw_hline(90, 142, 76, wave_col);
}

/*
 *@ brief    : Software timer callback alternating IOB1 (PIMD_EN) between 10ms HIGH and 90ms LOW.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : IOB1 toggled and next phase timer armed (20 ticks HIGH / 185 ticks LOW).
 *@ theme    : Asymmetric 10% duty-cycle 100ms pulse generator.
 */
static void _pimd_pulse_timer_cb(void)
{
    if (pimd_page == null || !pimd_page->running)
    {
        gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);
        return;
    }

    if (pimd_page->pin_high)
    {
        /* End of 10ms HIGH phase -> enter 90ms LOW phase */
        pimd_page->pin_high = false;
        gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);
        swt_start_timer(&pimd_timer, PIMD_LOW_TICKS, SWT_Once, _pimd_pulse_timer_cb);
    }
    else
    {
        /* End of 90ms LOW phase -> enter 10ms HIGH phase */
        pimd_page->pin_high = true;
        gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, true);
        swt_start_timer(&pimd_timer, PIMD_HIGH_TICKS, SWT_Once, _pimd_pulse_timer_cb);
    }
}

/*
 *@ brief    : Start the 100ms (10ms HIGH / 90ms LOW) pulse train on IOB1 (PIMD_EN).
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : IOB1 driven HIGH immediately and 10ms timer started.
 *@ theme    : PIMD start action.
 */
static void _pimd_start_output(void)
{
    if (pimd_page == null)
    {
        return;
    }

    pimd_page->running = true;
    pimd_page->pin_high = true;
    gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, true);
    swt_start_timer(&pimd_timer, PIMD_HIGH_TICKS, SWT_Once, _pimd_pulse_timer_cb);
    _pimd_draw_status_panel();
}

/*
 *@ brief    : Stop the pulse train on IOB1 (PIMD_EN) and force IOB1 LOW.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Timer stopped and IOB1 driven LOW.
 *@ theme    : Safe PIMD stop action.
 */
static void _pimd_stop_output(void)
{
    swt_stop_timer(&pimd_timer);
    gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);

    if (pimd_page != null)
    {
        pimd_page->running = false;
        pimd_page->pin_high = false;
        _pimd_draw_status_panel();
    }
}

/*
 *@ brief    : Move focus among the 3 bottom buttons (Start, Stop, Quit) with partial redraw.
 *@ type     : private
 *@ argu in  : new_idx: target button index (0..2).
 *@ argu out : Old button redrawn as Normal, new button redrawn as Focused.
 *@ theme    : Flicker-free button focus transition.
 */
static void _pimd_set_focus(u8 new_idx)
{
    u8 old_idx;

    if (pimd_page == null || new_idx >= PIMD_BTN_COUNT)
    {
        return;
    }

    old_idx = pimd_page->selected_idx;
    if (old_idx == new_idx)
    {
        return;
    }

    pimd_page->buttons[old_idx].state = GUI_Btn_Normal;
    gui_button_draw(&pimd_page->buttons[old_idx]);

    pimd_page->selected_idx = new_idx;
    pimd_page->buttons[new_idx].state = GUI_Btn_Focused;
    gui_button_draw(&pimd_page->buttons[new_idx]);
}

/*
 *@ brief    : Handle button messages while the PIMD page is active.
 *@ type     : private
 *@ argu in  : message: MSG_Button message with BTN_Message payload.
 *@ argu out : Left/Right selects Start/Stop/Quit; Enter triggers selected button;
 *             Back returns to Desktop (Click) or powers off (Hold).
 *@ theme    : PIMD key input router.
 */
static void _pimd_on_button_msg(const MSG_Message *message)
{
    const BTN_Message *btn;

    if (pimd_page == null || message == null || message->type != MSG_Button)
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
            _pimd_stop_output();
            bsp_power_off();
        }
        else if (btn->event == BTN_Click)
        {
            if (gui_setting_get_volume() > 0U)
            {
                spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
            }
            gui_pimd_exit();
            gui_desktop_enter();
        }
        return;
    }

    if (btn->event != BTN_Click && btn->event != BTN_Long_Press)
    {
        return;
    }

    if (btn->key == Btn_A4_Left || btn->key == Btn_A1_Up)
    {
        u8 next = (pimd_page->selected_idx == 0U) ? (u8)(PIMD_BTN_COUNT - 1U)
                                                  : (u8)(pimd_page->selected_idx - 1U);
        if (gui_setting_get_volume() > 0U)
        {
            spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
        }
        _pimd_set_focus(next);
    }
    else if (btn->key == Btn_A6_Right || btn->key == Btn_A5_Down)
    {
        u8 next = (u8)((pimd_page->selected_idx + 1U) % PIMD_BTN_COUNT);
        if (gui_setting_get_volume() > 0U)
        {
            spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
        }
        _pimd_set_focus(next);
    }
    else if (btn->key == Btn_A7_Enter && btn->event == BTN_Click)
    {
        if (pimd_page->selected_idx == (u8)PIMD_Btn_Start)
        {
            if (gui_setting_get_volume() > 0U)
            {
                spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_SCORE, TONE_VOL_MAX);
            }
            _pimd_start_output();
        }
        else if (pimd_page->selected_idx == (u8)PIMD_Btn_Stop)
        {
            if (gui_setting_get_volume() > 0U)
            {
                spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_HIT, TONE_VOL_HIGH);
            }
            _pimd_stop_output();
        }
        else if (pimd_page->selected_idx == (u8)PIMD_Btn_Quit)
        {
            if (gui_setting_get_volume() > 0U)
            {
                spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
            }
            gui_pimd_exit();
            gui_desktop_enter();
        }
    }
}

/*
 *@ brief    : Enter the PIMD application page, configure IOB1 output low, and render UI.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Full-screen PIMD panel and 3 evenly-spaced bottom buttons rendered.
 *@ theme    : Dynamic widget allocation from ram_heap with watermark tracking.
 */
void gui_pimd_enter(void)
{
    u8 i;

    if (pimd_page != null)
    {
        return;
    }

    gui_desktop_exit();

    /* Configure IOB1 (PIMD_EN) as push-pull output initially LOW */
    gpio_init_pattern(Pimd_En_Pin.port, Pimd_En_Pin.pin, GPIO_Out_Low);

    pimd_heap_mark = heap_get_mark();
    pimd_page = HEAP_NEW(GUI_Pimd_Page);
    if (pimd_page == null)
    {
        bsp_halt_fault(BSP_Fault_HEAP_State);
    }

    pimd_page->selected_idx = (u8)PIMD_Btn_Start;
    pimd_page->running = false;
    pimd_page->pin_high = false;

    for (i = 0U; i < PIMD_BTN_COUNT; ++i)
    {
        s16 bx = (s16)(PIMD_BTN_X0 + i * (PIMD_BTN_W + PIMD_BTN_GAP_X));
        gui_button_init(&pimd_page->buttons[i], bx, PIMD_BTN_Y, PIMD_BTN_W, PIMD_BTN_H,
                        pimd_btn_labels[i], GUI_Font_6x8);
        pimd_page->buttons[i].radius = 5U;
        pimd_page->buttons[i].icon = GUI_Icon_None;
        pimd_page->buttons[i].state =
            (i == pimd_page->selected_idx) ? GUI_Btn_Focused : GUI_Btn_Normal;
    }

    lcd_fill_color(PIMD_BG_COLOR);
    _pimd_draw_status_panel();
    for (i = 0U; i < PIMD_BTN_COUNT; ++i)
    {
        gui_button_draw(&pimd_page->buttons[i]);
    }

    msg_subscribe_type(&pimd_btn_sub, MSG_Button, null, _pimd_on_button_msg);
}

/*
 *@ brief    : Exit the PIMD page, stop pulse output on IOB1, and release ram_heap memory.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : IOB1 driven low, timer stopped, and widget memory reclaimed.
 *@ theme    : Zero-fragmentation page exit via heap_release_to_mark().
 */
void gui_pimd_exit(void)
{
    if (pimd_page == null)
    {
        return;
    }

    swt_stop_timer(&pimd_timer);
    gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);
    msg_unsubscribe_node(&pimd_btn_sub);
    pimd_page = null;
    heap_release_to_mark(pimd_heap_mark);
}

/*
 *@ brief    : Check whether the PIMD page is currently active.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if PIMD page is active.
 *@ theme    : State query helper.
 */
bool gui_pimd_is_active(void) { return pimd_page != null; }
