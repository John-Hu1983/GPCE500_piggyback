#include "..\..\..\Include\spinmaster.h"

static const char *const pimd_btn_labels[PIMD_BTN_COUNT] = {"Start", "Stop", "Quit"};

static GUI_Pimd_Page *pimd_page = null;
static HEAP_Mark pimd_heap_mark = 0U;
static SWT_Timer pimd_timer;
static MSG_Sub pimd_btn_sub = MSG_SUB_INIT;

static void _pimd_cycle_timer_cb(void);
static void _pimd_beep_off_timer_cb(void);

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
 *@ brief    : Append an unsigned 16-bit integer right-aligned in a fixed-width field.
 *@ type     : private
 *@ argu in  : dst: target buffer; pos: write cursor; max_len: buffer size; val: value; width: min chars.
 *@ argu out : Space-padded decimal digits appended and null-terminated.
 *@ theme    : Fixed-column numeric formatting so trailing units/brackets never jump horizontally.
 */
static void _pimd_append_u16_pad(char *dst, u8 *pos, u8 max_len, u16 val, u8 width)
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

    while (width > idx && (u8)(p + 1U) < max_len)
    {
        dst[p++] = ' ';
        width--;
    }

    while (idx > 0U && (u8)(p + 1U) < max_len)
    {
        dst[p++] = tmp[--idx];
    }
    dst[p] = '\0';
    *pos = p;
}

/*
 *@ brief    : Append a signed 16-bit delta ('+' or '-') with fixed-width magnitude padding.
 *@ type     : private
 *@ argu in  : dst: target buffer; pos: write cursor; max_len: buffer size; val: delta; width: digits.
 *@ argu out : Sign character + space-padded decimal magnitude appended.
 *@ theme    : Keep +dSlope and +dPeak columns locked in place across 1..3 digit transitions.
 */
static void _pimd_append_s16_delta(char *dst, u8 *pos, u8 max_len, s16 val, u8 width)
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
    _pimd_append_u16_pad(dst, pos, max_len, mag, width);
}

/*
 *@ brief    : Compute current status badge ID from PIMD_Phy_State.
 *@ type     : private
 *@ argu in  : phy: pointer to current PIMD_Phy_State.
 *@ argu out : PIMD_Status_Id enum value.
 *@ theme    : Detect status transitions to avoid redundant status_box redraws.
 */
static PIMD_Status_Id _pimd_get_status_id(const PIMD_Phy_State *phy)
{
    if (phy == null || !phy->running)
    {
        return PIMD_Status_Stop;
    }
    if (!phy->calibrated)
    {
        return PIMD_Status_Cal;
    }
    if (phy->metal_detected)
    {
        return PIMD_Status_Metal;
    }
    return PIMD_Status_No_Metal;
}

/*
 *@ brief    : Incrementally update one telemetry textbox row by redrawing only changed characters.
 *@ type     : private
 *@ argu in  : row: row index (0..3); new_str: formatted text; color: RGB565; full_draw: draw frame.
 *@ argu out : Only modified 6x8 character cells updated over SPI; box frame untouched when !full_draw.
 *@ theme    : Zero-flicker character-diff rendering for fixed-layout telemetry lines.
 */
static void _pimd_update_info_row(u8 row, const char *new_str, u16 color, bool full_draw)
{
    GUI_Textbox *box;
    char *old_str;
    bool color_changed;
    s16 tx, ty;
    u8 i = 0U;

    if (pimd_page == null || row >= PIMD_INFO_BOX_COUNT || new_str == null)
    {
        return;
    }

    box = &pimd_page->info_boxes[row];
    old_str = pimd_page->info_bufs[row];
    color_changed = (box->text_color != color);
    box->text_color = color;

    if (full_draw)
    {
        while (new_str[i] != '\0' && (u8)(i + 1U) < PIMD_INFO_TEXT_LEN)
        {
            old_str[i] = new_str[i];
            i++;
        }
        old_str[i] = '\0';
        box->text = old_str;
        gui_textbox_draw(box);
        return;
    }

    tx = (s16)(box->x + (s16)box->padding);
    ty = (s16)(box->y + (s16)box->padding);

    while (new_str[i] != '\0' && (u8)(i + 1U) < PIMD_INFO_TEXT_LEN)
    {
        if (color_changed || old_str[i] != new_str[i])
        {
            gui_draw_char_bg((s16)(tx + (s16)(i * 6U)), ty, new_str[i], color, box->bg_color,
                             box->font);
            old_str[i] = new_str[i];
        }
        i++;
    }

    while (old_str[i] != '\0' && (u8)(i + 1U) < PIMD_INFO_TEXT_LEN)
    {
        gui_draw_char_bg((s16)(tx + (s16)(i * 6U)), ty, ' ', color, box->bg_color, box->font);
        old_str[i] = '\0';
        i++;
    }
    old_str[i] = '\0';
}

/*
 *@ brief    : Refresh the status badge textbox and 4 diagnostic GUI_Textbox rows.
 *@ type     : private
 *@ argu in  : full_draw: true on page entry to draw box borders/backgrounds; false for diff update.
 *@ argu out : Status badge updated on state change; info rows updated via character diff.
 *@ theme    : Flicker-free GUI_Textbox telemetry panel above the button bar.
 */
static void _pimd_update_textboxes(bool full_draw)
{
    const PIMD_Phy_State *phy;
    char line_buf[PIMD_INFO_TEXT_LEN];
    PIMD_Status_Id status_id;
    u8 pos;
    u16 row_color;

    if (pimd_page == null)
    {
        return;
    }

    phy = &pimd_page->phy;
    status_id = _pimd_get_status_id(phy);

    /* Update top-right state badge textbox only when status actually changes or on full_draw */
    if (full_draw || status_id != pimd_page->last_status_id)
    {
        pimd_page->last_status_id = status_id;
        switch (status_id)
        {
        case PIMD_Status_Stop:
            pimd_page->status_box.bg_color = GUI_RGB565(64, 74, 92);
            pimd_page->status_box.border_color = GUI_RGB565(96, 112, 138);
            pimd_page->status_box.text_color = GUI_COLOR_WHITE;
            gui_textbox_set_text(&pimd_page->status_box, "STOP");
            break;

        case PIMD_Status_Cal:
            pimd_page->status_box.bg_color = GUI_RGB565(180, 120, 0);
            pimd_page->status_box.border_color = GUI_RGB565(240, 180, 40);
            pimd_page->status_box.text_color = GUI_COLOR_WHITE;
            gui_textbox_set_text(&pimd_page->status_box, "CAL...");
            break;

        case PIMD_Status_Metal:
            pimd_page->status_box.bg_color = GUI_RGB565(210, 36, 36);
            pimd_page->status_box.border_color = GUI_RGB565(255, 120, 120);
            pimd_page->status_box.text_color = GUI_COLOR_WHITE;
            gui_textbox_set_text(&pimd_page->status_box, "METAL !");
            break;

        case PIMD_Status_No_Metal:
        case PIMD_Status_None:
        default:
            pimd_page->status_box.bg_color = GUI_RGB565(0, 148, 86);
            pimd_page->status_box.border_color = GUI_RGB565(60, 220, 150);
            pimd_page->status_box.text_color = GUI_COLOR_WHITE;
            gui_textbox_set_text(&pimd_page->status_box, "NO METAL");
            break;
        }
    }

    pimd_page->last_hit_cnt = phy->hit_cnt;

    /* Row 0: Net slope change dSlp & (current Slp / baseline Bas) with fixed column widths */
    pos = 0U;
    line_buf[0] = '\0';
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, "dSlp:");
    _pimd_append_s16_delta(line_buf, &pos, PIMD_INFO_TEXT_LEN, phy->d_slope, 3U);
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, " (");
    _pimd_append_u16_pad(line_buf, &pos, PIMD_INFO_TEXT_LEN, phy->filt_slope, 3U);
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, "/");
    _pimd_append_u16_pad(line_buf, &pos, PIMD_INFO_TEXT_LEN, phy->base_slope, 3U);
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, ")");
    row_color = phy->metal_detected ? GUI_RGB565(255, 220, 80) : GUI_RGB565(120, 240, 255);
    _pimd_update_info_row(0U, line_buf, row_color, full_draw);

    /* Row 1: Net peak change dPk & current Peak (mV) with fixed column widths */
    pos = 0U;
    line_buf[0] = '\0';
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, "dPk :");
    _pimd_append_s16_delta(line_buf, &pos, PIMD_INFO_TEXT_LEN, phy->d_peak, 3U);
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, " (Pk:");
    _pimd_append_u16_pad(line_buf, &pos, PIMD_INFO_TEXT_LEN, phy->filt_peak, 4U);
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, "mV)");
    _pimd_update_info_row(1U, line_buf, GUI_RGB565(200, 230, 255), full_draw);

    /* Row 2: Rising-edge onset delay T0 (us) & floor voltage (mV) with fixed column widths */
    pos = 0U;
    line_buf[0] = '\0';
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, "T0:");
    _pimd_append_u16_pad(line_buf, &pos, PIMD_INFO_TEXT_LEN, phy->rise_us, 3U);
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, "us Flr:");
    _pimd_append_u16_pad(line_buf, &pos, PIMD_INFO_TEXT_LEN, phy->v_floor_mv, 4U);
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, "mV");
    _pimd_update_info_row(2U, line_buf, GUI_RGB565(180, 220, 190), full_draw);

    /* Row 3: Deadband-filtered signal score, threshold (55), and hit counter (0..2) */
    pos = 0U;
    line_buf[0] = '\0';
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, "Sig:");
    _pimd_append_u16_pad(line_buf, &pos, PIMD_INFO_TEXT_LEN, phy->sig_score, 3U);
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, "/55 Hit:");
    _pimd_append_u16_pad(line_buf, &pos, PIMD_INFO_TEXT_LEN, (u16)phy->hit_cnt, 1U);
    _pimd_append_str(line_buf, &pos, PIMD_INFO_TEXT_LEN, "/2");
    row_color = phy->metal_detected ? GUI_RGB565(255, 110, 110) : GUI_RGB565(170, 195, 230);
    _pimd_update_info_row(3U, line_buf, row_color, full_draw);
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

    _pimd_update_textboxes(true);
}

/*
 *@ brief    : Stop the short detection beep well before the next excitation pulse.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : SPU Channel 1 silenced and quiet settling delay armed before next pulse.
 *@ theme    : Guarantee >40ms of power-rail silence after a beep so C104 settles cleanly.
 */
static void _pimd_beep_off_timer_cb(void)
{
    spu_stop_channel(SPU_Channel_1);

    if (pimd_page == null || !pimd_page->phy.running)
    {
        return;
    }

    swt_start_timer(&pimd_timer, PIMD_BEEP_QUIET_TICKS, SWT_Once, _pimd_cycle_timer_cb);
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
    u16 wait_guard = 25000U;

    if (pimd_page == null || !pimd_page->phy.running)
    {
        pimd_stop_phy(pimd_page != null ? &pimd_page->phy : null);
        spu_stop_channel(SPU_Channel_1);
        return;
    }

    /* 1. Silence speaker before starting analog excitation and ADC burst */
    spu_stop_channel(SPU_Channel_1);

    /* 2. Run the ~950us pulse + Auto-ADC burst while SPI2/LCD/GPIOB are completely quiet */
    pimd_trigger_pulse(&pimd_page->phy);
    while (pimd_is_phy_busy() && --wait_guard != 0U)
    {
        system_clear_watchdog();
    }

    /* 3. Process the newly acquired 12-sample frame and refresh LCD textboxes */
    if (pimd_is_frame_ready())
    {
        pimd_process_frame(&pimd_page->phy);
    }

    pimd_page->ui_div_cnt++;
    if (_pimd_get_status_id(&pimd_page->phy) != pimd_page->last_status_id ||
        pimd_page->phy.hit_cnt != pimd_page->last_hit_cnt ||
        pimd_page->ui_div_cnt >= PIMD_UI_REFRESH_DIV)
    {
        pimd_page->ui_div_cnt = 0U;
        _pimd_update_textboxes(false);
    }

    /* 4. Play a short 12ms beep, then leave 42ms of silence for +4V5 & C104 to settle */
    if (pimd_page->phy.beep_pending)
    {
        pimd_page->phy.beep_pending = false;
        if (gui_setting_get_volume() > 0U)
        {
            spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_SCORE, TONE_VOL_MID);
            swt_start_timer(&pimd_timer, PIMD_BEEP_ON_TICKS, SWT_Once, _pimd_beep_off_timer_cb);
            return;
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
    pimd_page->ui_div_cnt = 0U;
    _pimd_update_textboxes(false);
    swt_start_timer(&pimd_timer, 2U, SWT_Once, _pimd_cycle_timer_cb);
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
        pimd_page->ui_div_cnt = 0U;
        _pimd_update_textboxes(false);
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
    u8 next;

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
        switch (btn->event)
        {
        case BTN_Hold:
            _pimd_stop_output();
            bsp_power_off();
            break;

        case BTN_Click:
            if (gui_setting_get_volume() > 0U)
            {
                spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
            }
            gui_pimd_exit();
            gui_desktop_enter();
            break;

        default:
            break;
        }
        return;
    }

    if (btn->event != BTN_Click && btn->event != BTN_Long_Press)
    {
        return;
    }

    switch (btn->key)
    {
    case Btn_A4_Left:
    case Btn_A1_Up:
        next = (pimd_page->selected_idx == 0U) ? (u8)(PIMD_BTN_COUNT - 1U)
                                               : (u8)(pimd_page->selected_idx - 1U);
        if (gui_setting_get_volume() > 0U)
        {
            spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
        }
        _pimd_set_focus(next);
        break;

    case Btn_A6_Right:
    case Btn_A5_Down:
        next = (u8)((pimd_page->selected_idx + 1U) % PIMD_BTN_COUNT);
        if (gui_setting_get_volume() > 0U)
        {
            spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
        }
        _pimd_set_focus(next);
        break;

    case Btn_A7_Enter:
        if (btn->event != BTN_Click)
        {
            break;
        }
        switch ((PIMD_Btn_Id)pimd_page->selected_idx)
        {
        case PIMD_Btn_Start:
            if (gui_setting_get_volume() > 0U)
            {
                spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_SCORE, TONE_VOL_MAX);
            }
            _pimd_start_output();
            break;

        case PIMD_Btn_Stop:
            if (gui_setting_get_volume() > 0U)
            {
                spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_HIT, TONE_VOL_HIGH);
            }
            _pimd_stop_output();
            break;

        case PIMD_Btn_Quit:
            if (gui_setting_get_volume() > 0U)
            {
                spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
            }
            gui_pimd_exit();
            gui_desktop_enter();
            break;

        default:
            break;
        }
        break;

    default:
        break;
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
    pimd_page->ui_div_cnt = 0U;
    pimd_page->last_status_id = PIMD_Status_None;
    pimd_page->last_hit_cnt = 0U;
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
