#include "..\..\..\Include\spinmaster.h"

static const char *const pimd_btn_labels[PIMD_BTN_COUNT] = {"Start", "Stop", "Quit"};

static GUI_Pimd_Page *pimd_page = null;
static HEAP_Mark pimd_heap_mark = 0U;
static SWT_Timer pimd_timer;
static MSG_Sub pimd_btn_sub = MSG_SUB_INIT;

static void _pimd_cycle_timer_cb(void);

/*
 *@ brief    : Append a null-terminated string into a character buffer at index *pos.
 *@ type     : private
 *@ argu in  : dst: target buffer; pos: write cursor; max_len: buffer size; str: source text.
 *@ argu out : Characters copied up to max_len - 1 and null-terminated.
 *@ theme    : Lightweight string formatting without printf overhead.
 */
static void _pimd_append_str(char *dst, u8 *pos, u8 max_len, const char *str)
{
    u8 p = *pos;

    if (dst == null || str == null || max_len == 0U)
    {
        return;
    }

    while (*str != '\0' && (u8)(p + 1U) < max_len)
    {
        dst[p++] = *str++;
    }
    dst[p] = '\0';
    *pos = p;
}

/*
 *@ brief    : Append an unsigned 16-bit integer to a character buffer.
 *@ type     : private
 *@ argu in  : dst: target buffer; pos: write cursor; max_len: buffer size; val: unsigned value.
 *@ argu out : Decimal digits appended and null-terminated.
 *@ theme    : Compact integer formatter for GUI_Textbox diagnostic lines.
 */
static void _pimd_append_u16(char *dst, u8 *pos, u8 max_len, u16 val)
{
    char tmp[6];
    u8 idx = 0U;
    u8 p = *pos;

    if (dst == null || max_len == 0U)
    {
        return;
    }

    do
    {
        tmp[idx++] = (char)('0' + (val % 10U));
        val /= 10U;
    } while (val != 0U && idx < sizeof(tmp));

    while (idx > 0U && (u8)(p + 1U) < max_len)
    {
        dst[p++] = tmp[--idx];
    }
    dst[p] = '\0';
    *pos = p;
}

/*
 *@ brief    : Append a signed 16-bit delta (with explicit '+' or '-') to a character buffer.
 *@ type     : private
 *@ argu in  : dst: target buffer; pos: write cursor; max_len: buffer size; val: signed delta.
 *@ argu out : Signed decimal string appended and null-terminated.
 *@ theme    : Format baseline deviation (+dSlope / +dPeak) for textbox readout.
 */
static void _pimd_append_s16_delta(char *dst, u8 *pos, u8 max_len, s16 val)
{
    u16 mag;

    if (val < 0)
    {
        _pimd_append_str(dst, pos, max_len, "-");
        mag = (u16)(-val);
    }
    else
    {
        _pimd_append_str(dst, pos, max_len, "+");
        mag = (u16)val;
    }
    _pimd_append_u16(dst, pos, max_len, mag);
}

/*
 *@ brief    : Refresh the status badge textbox and 4 diagnostic GUI_Textbox rows.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Status and dSlp/dPk/T0/Sig textboxes formatted and redrawn.
 *@ theme    : Flicker-free GUI_Textbox telemetry panel above the button bar.
 */
static void _pimd_update_textboxes(void)
{
    const PIMD_Phy_State *phy;
    u8 pos;

    if (pimd_page == null)
    {
        return;
    }

    phy = &pimd_page->phy;

    /* Update top-right state badge textbox */
    if (!phy->running)
    {
        pimd_page->status_box.bg_color = GUI_RGB565(64, 74, 92);
        pimd_page->status_box.border_color = GUI_RGB565(96, 112, 138);
        pimd_page->status_box.text_color = GUI_COLOR_WHITE;
        gui_textbox_set_text(&pimd_page->status_box, "STOP");
    }
    else if (!phy->calibrated)
    {
        pimd_page->status_box.bg_color = GUI_RGB565(180, 120, 0);
        pimd_page->status_box.border_color = GUI_RGB565(240, 180, 40);
        pimd_page->status_box.text_color = GUI_COLOR_WHITE;
        gui_textbox_set_text(&pimd_page->status_box, "CAL...");
    }
    else if (phy->metal_detected)
    {
        pimd_page->status_box.bg_color = GUI_RGB565(210, 36, 36);
        pimd_page->status_box.border_color = GUI_RGB565(255, 120, 120);
        pimd_page->status_box.text_color = GUI_COLOR_WHITE;
        gui_textbox_set_text(&pimd_page->status_box, "METAL !");
    }
    else
    {
        pimd_page->status_box.bg_color = GUI_RGB565(0, 148, 86);
        pimd_page->status_box.border_color = GUI_RGB565(60, 220, 150);
        pimd_page->status_box.text_color = GUI_COLOR_WHITE;
        gui_textbox_set_text(&pimd_page->status_box, "NO METAL");
    }

    /* Row 0: Net slope change dSlp & (current Slp / baseline Bas) */
    pos = 0U;
    _pimd_append_str(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, "dSlp:");
    _pimd_append_s16_delta(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, phy->d_slope);
    _pimd_append_str(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, " (");
    _pimd_append_u16(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, phy->filt_slope);
    _pimd_append_str(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, "/");
    _pimd_append_u16(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, phy->base_slope);
    _pimd_append_str(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, ")");
    pimd_page->info_boxes[0].text_color =
        phy->metal_detected ? GUI_RGB565(255, 220, 80) : GUI_RGB565(120, 240, 255);
    gui_textbox_set_text(&pimd_page->info_boxes[0], pimd_page->info_bufs[0]);

    /* Row 1: Net peak change dPk & current Peak (mV) */
    pos = 0U;
    _pimd_append_str(pimd_page->info_bufs[1], &pos, PIMD_INFO_TEXT_LEN, "dPk :");
    _pimd_append_s16_delta(pimd_page->info_bufs[1], &pos, PIMD_INFO_TEXT_LEN, phy->d_peak);
    _pimd_append_str(pimd_page->info_bufs[1], &pos, PIMD_INFO_TEXT_LEN, " (Pk:");
    _pimd_append_u16(pimd_page->info_bufs[1], &pos, PIMD_INFO_TEXT_LEN, phy->filt_peak);
    _pimd_append_str(pimd_page->info_bufs[1], &pos, PIMD_INFO_TEXT_LEN, "mV)");
    pimd_page->info_boxes[1].text_color = GUI_RGB565(200, 230, 255);
    gui_textbox_set_text(&pimd_page->info_boxes[1], pimd_page->info_bufs[1]);

    /* Row 2: Rising-edge onset delay T0 (us) & floor voltage (mV) */
    pos = 0U;
    _pimd_append_str(pimd_page->info_bufs[2], &pos, PIMD_INFO_TEXT_LEN, "T0:");
    _pimd_append_u16(pimd_page->info_bufs[2], &pos, PIMD_INFO_TEXT_LEN, phy->rise_us);
    _pimd_append_str(pimd_page->info_bufs[2], &pos, PIMD_INFO_TEXT_LEN, "us Flr:");
    _pimd_append_u16(pimd_page->info_bufs[2], &pos, PIMD_INFO_TEXT_LEN, phy->v_floor_mv);
    _pimd_append_str(pimd_page->info_bufs[2], &pos, PIMD_INFO_TEXT_LEN, "mV");
    pimd_page->info_boxes[2].text_color = GUI_RGB565(180, 220, 190);
    gui_textbox_set_text(&pimd_page->info_boxes[2], pimd_page->info_bufs[2]);

    /* Row 3: Deadband-filtered signal score, threshold (55), and hit counter (0..2) */
    pos = 0U;
    _pimd_append_str(pimd_page->info_bufs[3], &pos, PIMD_INFO_TEXT_LEN, "Sig:");
    _pimd_append_u16(pimd_page->info_bufs[3], &pos, PIMD_INFO_TEXT_LEN, phy->sig_score);
    _pimd_append_str(pimd_page->info_bufs[3], &pos, PIMD_INFO_TEXT_LEN, "/55 Hit:");
    _pimd_append_u16(pimd_page->info_bufs[3], &pos, PIMD_INFO_TEXT_LEN, (u16)phy->hit_cnt);
    _pimd_append_str(pimd_page->info_bufs[3], &pos, PIMD_INFO_TEXT_LEN, "/2");
    pimd_page->info_boxes[3].text_color =
        phy->metal_detected ? GUI_RGB565(255, 110, 110) : GUI_RGB565(170, 195, 230);
    gui_textbox_set_text(&pimd_page->info_boxes[3], pimd_page->info_bufs[3]);
}

/*
 *@ brief    : Draw the static upper panel frame, title icon, and initial GUI_Textbox widgets.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Panel card and diagnostic textboxes rendered on LCD.
 *@ theme    : Diagnostic status layout above the 3 bottom control buttons.
 */
static void _pimd_draw_status_panel(void)
{
    if (pimd_page == null)
    {
        return;
    }

    gui_fill_round_rect(6, 4, (s16)(LCD_WIDTH - 7U), 91, 6U, PIMD_PANEL_BG);
    gui_draw_round_rect(6, 4, (s16)(LCD_WIDTH - 7U), 91, 6U, PIMD_PANEL_BORDER);

    gui_icon_draw(GUI_Icon_Pimd, 18, 13, PIMD_PANEL_BG, GUI_RGB565(0, 220, 160));
    gui_draw_str_at(28, 9, "PIMD", GUI_COLOR_WHITE, GUI_Font_6x8);

    _pimd_update_textboxes();
}

/*
 *@ brief    : Periodic software timer callback processing completed frames and re-triggering.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Evaluates previous pulse ADC samples, triggers next pulse, and refreshes UI.
 *@ theme    : Decouples main-loop UI/algorithm work from hardware Auto-ADC/DMA acquisition.
 */
static void _pimd_cycle_timer_cb(void)
{
    if (pimd_page == null || !pimd_page->phy.running)
    {
        pimd_stop_phy(pimd_page != null ? &pimd_page->phy : null);
        spu_stop_channel(SPU_Channel_1);
        return;
    }

    if (pimd_is_frame_ready())
    {
        pimd_process_frame(&pimd_page->phy);
    }

    spu_stop_channel(SPU_Channel_1);
    pimd_trigger_pulse(&pimd_page->phy);

    _pimd_update_textboxes();

    if (pimd_page->phy.beep_pending)
    {
        pimd_page->phy.beep_pending = false;
        if (gui_setting_get_volume() > 0U)
        {
            spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_SCORE, TONE_VOL_HIGH);
        }
    }

    swt_start_timer(&pimd_timer, PIMD_CYCLE_TICKS, SWT_Once, _pimd_cycle_timer_cb);
}

/*
 *@ brief    : Reset baseline calibration and start periodic PIMD measurements.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : PHY started, first pulse triggered, and cycle timer armed.
 *@ theme    : Button-controlled measurement start action.
 */
static void _pimd_start_output(void)
{
    if (pimd_page == null)
    {
        return;
    }

    pimd_start_phy(&pimd_page->phy);
    _pimd_update_textboxes();
    pimd_trigger_pulse(&pimd_page->phy);
    swt_start_timer(&pimd_timer, PIMD_CYCLE_TICKS, SWT_Once, _pimd_cycle_timer_cb);
}

/*
 *@ brief    : Stop PIMD measurement cycles, disable PHY hardware, and update UI to STOP.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Software timer and PHY stopped, status textbox set to STOP.
 *@ theme    : Button-controlled measurement stop action.
 */
static void _pimd_stop_output(void)
{
    swt_stop_timer(&pimd_timer);
    spu_stop_channel(SPU_Channel_1);
    pimd_stop_phy(pimd_page != null ? &pimd_page->phy : null);

    if (pimd_page != null)
    {
        _pimd_update_textboxes();
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
 *@ brief    : Enter the PIMD page, initialize PHY hardware, and render UI.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : PHY initialized and GUI_Textbox status panel + buttons drawn.
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

    pimd_heap_mark = heap_get_mark();
    pimd_page = HEAP_NEW(GUI_Pimd_Page);
    if (pimd_page == null)
    {
        bsp_halt_fault(BSP_Fault_HEAP_State);
    }

    pimd_page->selected_idx = (u8)PIMD_Btn_Start;
    pimd_init_phy(&pimd_page->phy);

    /* Initialize top-right status badge textbox */
    gui_textbox_init(&pimd_page->status_box, 68, 6, 80U, 14U, "STOP", GUI_Font_6x8);
    pimd_page->status_box.align = GUI_Align_Center;
    pimd_page->status_box.padding = 2U;

    /* Initialize 4 telemetry GUI_Textbox rows above the button bar */
    for (i = 0U; i < PIMD_INFO_BOX_COUNT; ++i)
    {
        s16 by = (s16)(22 + (s16)i * 17);
        pimd_page->info_bufs[i][0] = '\0';
        gui_textbox_init(&pimd_page->info_boxes[i], 10, by, 140U, 15U, pimd_page->info_bufs[i],
                         GUI_Font_6x8);
        pimd_page->info_boxes[i].align = GUI_Align_Left;
        pimd_page->info_boxes[i].padding = 3U;
        pimd_page->info_boxes[i].bg_color = PIMD_BOX_BG;
        pimd_page->info_boxes[i].border_color = PIMD_BOX_BORDER;
    }

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
 *@ brief    : Exit the PIMD page, stop PHY hardware, and release ram_heap memory.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Software timer and PHY stopped, IOB1 driven LOW, and heap reclaimed.
 *@ theme    : Zero-fragmentation page exit via heap_release_to_mark().
 */
void gui_pimd_exit(void)
{
    if (pimd_page == null)
    {
        return;
    }

    swt_stop_timer(&pimd_timer);
    spu_stop_channel(SPU_Channel_1);
    pimd_stop_phy(&pimd_page->phy);
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
