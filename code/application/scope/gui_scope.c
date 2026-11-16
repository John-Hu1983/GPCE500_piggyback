#include "..\..\Include\spinmaster.h"

/*
 * 72-point full-wave sine lookup table (0..355 deg in 5-deg increments), scaled to [-127, +127].
 * - Channel 0 (  0 deg): index + 0
 * - Channel 1 (120 deg): index + 24
 * - Channel 2 (240 deg): index + 48
 */
static const s16 scope_sin_lut[SCOPE_LUT_SIZE] = {
    0,    11,   22,   33,   43,   54,   64,   73,   82,   90,   97,   104,  110,  115,  119,
    123,  125,  127,  127,  127,  125,  123,  119,  115,  110,  104,  97,   90,   82,   73,
    64,   54,   43,   33,   22,   11,   0,    -11,  -22,  -33,  -43,  -54,  -64,  -73,  -82,
    -90,  -97,  -104, -110, -115, -119, -123, -125, -127, -127, -127, -125, -123, -119, -115,
    -110, -104, -97,  -90,  -82,  -73,  -64,  -54,  -43,  -33,  -22,  -11};

static const u16 scope_ch_colors[SCOPE_CHANNELS] = {
    GUI_RGB565(255, 230, 0), /* CH1 (  0 deg): Bright Yellow */
    GUI_RGB565(0, 240, 255), /* CH2 (120 deg): Bright Cyan   */
    GUI_RGB565(255, 76, 196) /* CH3 (240 deg): Bright Pink   */
};

static GUI_Scope_Page *scope_page = null;
static HEAP_Mark scope_heap_mark = 0U;
static SCH_Task scope_task = SCH_TASK_INIT;
static MSG_Sub scope_btn_sub = MSG_SUB_INIT;

/*
 *@ brief    : Return the background or graticule grid color at pixel coordinate (x, y).
 *@ type     : private
 *@ argu in  : x, y: pixel coordinates inside the oscilloscope wave box.
 *@ argu out : RGB565 background/grid color at (x, y).
 *@ theme    : Preserve center axes and graticule dots when erasing moving waveforms.
 */
static u16 _scope_get_bg_pixel(s16 x, s16 y)
{
    if (y == SCOPE_CENTER_Y && ((x & 1) == 0))
    {
        return SCOPE_AXIS_COLOR;
    }
    if (x == SCOPE_CENTER_X && ((y & 1) == 0))
    {
        return SCOPE_AXIS_COLOR;
    }
    if (((x - SCOPE_CENTER_X) % 24 == 0) && ((y - SCOPE_CENTER_Y) % 24 == 0))
    {
        return SCOPE_AXIS_COLOR;
    }
    if (((x - SCOPE_CENTER_X) % 24 == 0) && ((y & 3) == 0))
    {
        return SCOPE_GRID_COLOR;
    }
    if (((y - SCOPE_CENTER_Y) % 24 == 0) && ((x & 3) == 0))
    {
        return SCOPE_GRID_COLOR;
    }
    return SCOPE_BG_COLOR;
}

/*
 *@ brief    : Fast-erase a vertical segment at column `x` from `y_a` to `y_b` in one SPI window.
 *@ type     : private
 *@ argu in  : x: screen X coordinate; y_a, y_b: vertical segment endpoints.
 *@ argu out : Old waveform segment cleared and center axis restored if crossed.
 *@ theme    : Single-transaction vertical line clear for high-FPS oscilloscope rendering.
 */
static void _scope_erase_segment(s16 x, s16 y_a, s16 y_b)
{
    s16 y0 = (y_a <= y_b) ? y_a : y_b;
    s16 y1 = (y_a <= y_b) ? y_b : y_a;

    gui_draw_vline(x, y0, y1, SCOPE_BG_COLOR);
    if (y0 <= SCOPE_CENTER_Y && SCOPE_CENTER_Y <= y1 && ((x & 1) == 0))
    {
        gui_draw_point(x, SCOPE_CENTER_Y, SCOPE_AXIS_COLOR);
    }
}

/*
 *@ brief    : Draw the top HUD bar showing 3-phase channel legend and RUN/HOLD indicator.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Top 15px oscilloscope header bar rendered.
 *@ theme    : Oscilloscope status header.
 */
static void _scope_draw_hud(void)
{
    if (scope_page == null)
    {
        return;
    }

    gui_fill_rect(0, 0, (s16)(LCD_WIDTH - 1U), 14, SCOPE_HUD_BG);
    gui_draw_hline(0, (s16)(LCD_WIDTH - 1U), 14, SCOPE_BORDER_COLOR);

    /* Channel legend with 120-degree phase indicators */
    gui_fill_rect(4, 4, 8, 10, scope_ch_colors[0]);
    gui_draw_str_at(11, 3, "0", scope_ch_colors[0], GUI_Font_6x8);

    gui_fill_rect(24, 4, 28, 10, scope_ch_colors[1]);
    gui_draw_str_at(31, 3, "120", scope_ch_colors[1], GUI_Font_6x8);

    gui_fill_rect(56, 4, 60, 10, scope_ch_colors[2]);
    gui_draw_str_at(63, 3, "240", scope_ch_colors[2], GUI_Font_6x8);

    /* RUN / HOLD status badge on the right */
    if (scope_page->paused)
    {
        gui_fill_round_rect(122, 2, 155, 12, 2U, GUI_RGB565(180, 50, 50));
        gui_draw_str_at(127, 3, "HOLD", GUI_COLOR_WHITE, GUI_Font_6x8);
    }
    else
    {
        gui_fill_round_rect(122, 2, 155, 12, 2U, GUI_RGB565(0, 150, 90));
        gui_draw_str_at(130, 3, "RUN", GUI_COLOR_WHITE, GUI_Font_6x8);
    }
}

/*
 *@ brief    : Render the full static oscilloscope graticule box and grid lines once on entry.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Border frame, center crosshair axes, and division grid rendered.
 *@ theme    : Initial oscilloscope screen setup.
 */
static void _scope_draw_graticule(void)
{
    s16 x, y;

    lcd_fill_color(SCOPE_BG_COLOR);
    _scope_draw_hud();
    gui_draw_rect(SCOPE_BOX_X0, SCOPE_BOX_Y0, SCOPE_BOX_X1, SCOPE_BOX_Y1, SCOPE_BORDER_COLOR);

    for (y = SCOPE_WAVE_Y0; y < (s16)(SCOPE_WAVE_Y0 + (s16)SCOPE_WAVE_H); ++y)
    {
        for (x = SCOPE_WAVE_X0; x < (s16)(SCOPE_WAVE_X0 + (s16)SCOPE_WAVE_W); ++x)
        {
            u16 col = _scope_get_bg_pixel(x, y);
            if (col != SCOPE_BG_COLOR)
            {
                gui_draw_point(x, y, col);
            }
        }
    }
}

/*
 *@ brief    : Compute clamped screen Y coordinate from a LUT index and amplitude.
 *@ type     : private
 *@ argu in  : lut_idx: 0..71 sine LUT index; amp: wave amplitude in pixels.
 *@ argu out : Clamped vertical pixel coordinate inside the wave box.
 *@ theme    : Single-shift (>> 7) sine scaling without division or RAM tables.
 */
static s16 _scope_calc_y(u8 lut_idx, s16 amp)
{
    s16 offset = (s16)((scope_sin_lut[lut_idx] * amp) >> 7);
    s16 y = (s16)(SCOPE_CENTER_Y - offset);

    if (y < SCOPE_WAVE_Y0)
    {
        y = SCOPE_WAVE_Y0;
    }
    else if (y >= (s16)(SCOPE_WAVE_Y0 + (s16)SCOPE_WAVE_H))
    {
        y = (s16)(SCOPE_WAVE_Y0 + (s16)SCOPE_WAVE_H - 1);
    }
    return y;
}

/*
 *@ brief    : Prepare a new 148-column frame by latching current and previous wave parameters.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : 3-phase LUT indices initialized for column 0 of both old and new waveforms.
 *@ theme    : O(1) RAM frame setup without per-column history buffers.
 */
static void _scope_begin_frame(void)
{
    if (scope_page == null)
    {
        return;
    }

    scope_page->render_phase = scope_page->phase;
    scope_page->render_freq = scope_page->freq_step;
    scope_page->render_amp = scope_page->amplitude;
    scope_page->col_cursor = 0U;
    scope_page->step_acc = 0U;
    scope_page->old_step_acc = 0U;

    scope_page->idx0 = scope_page->render_phase;
    scope_page->idx1 = (u8)(scope_page->idx0 + SCOPE_PHASE_120_STEPS);
    if (scope_page->idx1 >= SCOPE_LUT_SIZE)
    {
        scope_page->idx1 = (u8)(scope_page->idx1 - SCOPE_LUT_SIZE);
    }
    scope_page->idx2 = (u8)(scope_page->idx1 + SCOPE_PHASE_120_STEPS);
    if (scope_page->idx2 >= SCOPE_LUT_SIZE)
    {
        scope_page->idx2 = (u8)(scope_page->idx2 - SCOPE_LUT_SIZE);
    }

    scope_page->old_idx0 = scope_page->old_phase;
    scope_page->old_idx1 = (u8)(scope_page->old_idx0 + SCOPE_PHASE_120_STEPS);
    if (scope_page->old_idx1 >= SCOPE_LUT_SIZE)
    {
        scope_page->old_idx1 = (u8)(scope_page->old_idx1 - SCOPE_LUT_SIZE);
    }
    scope_page->old_idx2 = (u8)(scope_page->old_idx1 + SCOPE_PHASE_120_STEPS);
    if (scope_page->old_idx2 >= SCOPE_LUT_SIZE)
    {
        scope_page->old_idx2 = (u8)(scope_page->old_idx2 - SCOPE_LUT_SIZE);
    }
}

/*
 *@ brief    : Render up to `max_cols` columns of the current frame and return to main loop.
 *@ type     : private
 *@ argu in  : max_cols: number of columns to render in this slice (e.g., 37 or 148).
 *@ argu out : Slice of columns updated on LCD; `initialized` set when column 147 completes.
 *@ theme    : Non-blocking sliced rendering so main loop services button timers every ~8ms.
 */
static void _scope_render_slice(u8 max_cols)
{
    u8 count = 0U;
    u8 ch;
    bool first_frame;

    if (scope_page == null)
    {
        return;
    }

    if (scope_page->col_cursor == 0U)
    {
        _scope_begin_frame();
    }

    first_frame = !scope_page->initialized;

    while (scope_page->col_cursor < SCOPE_WAVE_W && count < max_cols)
    {
        u8 col = scope_page->col_cursor;
        s16 x = (s16)(SCOPE_WAVE_X0 + (s16)col);
        s16 new_y[SCOPE_CHANNELS];
        s16 old_y[SCOPE_CHANNELS];

        new_y[0] = _scope_calc_y(scope_page->idx0, scope_page->render_amp);
        new_y[1] = _scope_calc_y(scope_page->idx1, scope_page->render_amp);
        new_y[2] = _scope_calc_y(scope_page->idx2, scope_page->render_amp);

        scope_page->step_acc = (u8)(scope_page->step_acc + scope_page->render_freq);
        while (scope_page->step_acc >= 2U)
        {
            scope_page->step_acc = (u8)(scope_page->step_acc - 2U);
            if (++scope_page->idx0 >= SCOPE_LUT_SIZE)
            {
                scope_page->idx0 = 0U;
            }
            if (++scope_page->idx1 >= SCOPE_LUT_SIZE)
            {
                scope_page->idx1 = 0U;
            }
            if (++scope_page->idx2 >= SCOPE_LUT_SIZE)
            {
                scope_page->idx2 = 0U;
            }
        }

        if (!first_frame)
        {
            old_y[0] = _scope_calc_y(scope_page->old_idx0, scope_page->old_amp);
            old_y[1] = _scope_calc_y(scope_page->old_idx1, scope_page->old_amp);
            old_y[2] = _scope_calc_y(scope_page->old_idx2, scope_page->old_amp);

            scope_page->old_step_acc = (u8)(scope_page->old_step_acc + scope_page->old_freq);
            while (scope_page->old_step_acc >= 2U)
            {
                scope_page->old_step_acc = (u8)(scope_page->old_step_acc - 2U);
                if (++scope_page->old_idx0 >= SCOPE_LUT_SIZE)
                {
                    scope_page->old_idx0 = 0U;
                }
                if (++scope_page->old_idx1 >= SCOPE_LUT_SIZE)
                {
                    scope_page->old_idx1 = 0U;
                }
                if (++scope_page->old_idx2 >= SCOPE_LUT_SIZE)
                {
                    scope_page->old_idx2 = 0U;
                }
            }

            for (ch = 0U; ch < SCOPE_CHANNELS; ++ch)
            {
                if (col == 0U)
                {
                    if (old_y[ch] != new_y[ch])
                    {
                        gui_draw_point(x, old_y[ch], _scope_get_bg_pixel(x, old_y[ch]));
                    }
                }
                else if (scope_page->old_prev[ch] != scope_page->new_prev[ch] ||
                         old_y[ch] != new_y[ch])
                {
                    _scope_erase_segment(x, scope_page->old_prev[ch], old_y[ch]);
                }
                scope_page->old_prev[ch] = old_y[ch];
            }
        }

        /* Draw new segments for all 3 channels at column `x` */
        for (ch = 0U; ch < SCOPE_CHANNELS; ++ch)
        {
            if (col == 0U || scope_page->new_prev[ch] == new_y[ch])
            {
                gui_draw_point(x, new_y[ch], scope_ch_colors[ch]);
            }
            else
            {
                s16 y0 = (scope_page->new_prev[ch] <= new_y[ch]) ? scope_page->new_prev[ch]
                                                                 : new_y[ch];
                s16 y1 = (scope_page->new_prev[ch] <= new_y[ch]) ? new_y[ch]
                                                                 : scope_page->new_prev[ch];
                gui_draw_vline(x, y0, y1, scope_ch_colors[ch]);
            }
            scope_page->new_prev[ch] = new_y[ch];
        }

        scope_page->col_cursor++;
        count++;
    }

    if (scope_page->col_cursor >= SCOPE_WAVE_W)
    {
        scope_page->col_cursor = 0U;
        scope_page->old_phase = scope_page->render_phase;
        scope_page->old_freq = scope_page->render_freq;
        scope_page->old_amp = scope_page->render_amp;
        scope_page->initialized = true;
    }
}

/*
 *@ brief    : Periodic scheduler task rendering one 37-column slice (~8ms) per invocation.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Next 37 columns rendered; phase advanced when a full 148-column frame completes.
 *@ theme    : Cooperative sliced rendering yielding back to main loop every ~8ms for button scan.
 */
static void _scope_service_task(void)
{
    if (scope_page == null)
    {
        return;
    }

    if (scope_page->col_cursor != 0U)
    {
        /* Continue rendering the in-progress frame slice */
        _scope_render_slice(SCOPE_COLS_PER_SLICE);
        return;
    }

    if (!scope_page->paused)
    {
        scope_page->phase = (u8)(scope_page->phase + SCOPE_PHASE_STEP);
        if (scope_page->phase >= SCOPE_LUT_SIZE)
        {
            scope_page->phase = (u8)(scope_page->phase - SCOPE_LUT_SIZE);
        }
        scope_page->dirty = false;
        _scope_render_slice(SCOPE_COLS_PER_SLICE);
    }
    else if (scope_page->dirty)
    {
        scope_page->dirty = false;
        _scope_render_slice(SCOPE_COLS_PER_SLICE);
    }
}

/*
 *@ brief    : Handle button messages while the Oscilloscope page is active.
 *@ type     : private
 *@ argu in  : message: MSG_Button message with BTN_Message payload.
 *@ argu out : Up/Down adjusts amplitude; Left/Right adjusts frequency; Enter toggles RUN/HOLD;
 *             Back returns to Desktop (Click) or powers off (Hold).
 *@ theme    : Oscilloscope key input handler.
 */
static void _scope_on_button_msg(const MSG_Message *message)
{
    const BTN_Message *btn;

    if (scope_page == null || message == null || message->type != MSG_Button)
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
            if (gui_setting_get_volume() > 0U)
            {
                spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_MOVE, TONE_VOL_HIGH);
            }
            gui_scope_exit();
            gui_desktop_enter();
        }
        return;
    }

    if (btn->key == Btn_A7_Enter && btn->event == BTN_Click)
    {
        scope_page->paused = !scope_page->paused;
        if (gui_setting_get_volume() > 0U)
        {
            spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_HIT, TONE_VOL_HIGH);
        }
        _scope_draw_hud();
        return;
    }

    if (btn->event != BTN_Click && btn->event != BTN_Long_Press)
    {
        return;
    }

    if (btn->key == Btn_A1_Up)
    {
        if (scope_page->amplitude + SCOPE_AMP_STEP <= SCOPE_AMP_MAX)
        {
            scope_page->amplitude = (s16)(scope_page->amplitude + SCOPE_AMP_STEP);
            scope_page->dirty = true;
        }
    }
    else if (btn->key == Btn_A5_Down)
    {
        if (scope_page->amplitude - SCOPE_AMP_STEP >= SCOPE_AMP_MIN)
        {
            scope_page->amplitude = (s16)(scope_page->amplitude - SCOPE_AMP_STEP);
            scope_page->dirty = true;
        }
    }
    else if (btn->key == Btn_A6_Right)
    {
        if (scope_page->freq_step < SCOPE_FREQ_MAX)
        {
            scope_page->freq_step++;
            scope_page->dirty = true;
        }
    }
    else if (btn->key == Btn_A4_Left)
    {
        if (scope_page->freq_step > SCOPE_FREQ_MIN)
        {
            scope_page->freq_step--;
            scope_page->dirty = true;
        }
    }
}

/*
 *@ brief    : Enter the 3-channel 120-deg Sine Wave Oscilloscope page.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Scope state allocated from ram_heap, graticule and 3-phase waves rendered.
 *@ theme    : Dynamic RAM allocation via ram_heap with watermark reclamation.
 */
void gui_scope_enter(void)
{
    if (scope_page != null)
    {
        return;
    }

    gui_desktop_exit();

    scope_heap_mark = heap_get_mark();
    scope_page = HEAP_NEW(GUI_Scope_Page);
    if (scope_page == null)
    {
        bsp_halt_fault(BSP_Fault_HEAP_State);
    }

    scope_page->phase = 0U;
    scope_page->freq_step = SCOPE_FREQ_DEFAULT;
    scope_page->col_cursor = 0U;
    scope_page->amplitude = SCOPE_AMP_DEFAULT;
    scope_page->paused = false;
    scope_page->dirty = false;
    scope_page->initialized = false;

    _scope_draw_graticule();
    _scope_render_slice(SCOPE_WAVE_W);

    msg_subscribe_type(&scope_btn_sub, MSG_Button, null, _scope_on_button_msg);
    sch_start_task(&scope_task, SCOPE_TICK_PERIOD, SCOPE_TICK_PERIOD, _scope_service_task);
}

/*
 *@ brief    : Exit the Oscilloscope page, stop sweep task, and release ram_heap memory.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Task stopped, button subscription detached, and heap memory freed.
 *@ theme    : Zero-fragmentation page exit via heap_release_to_mark().
 */
void gui_scope_exit(void)
{
    if (scope_page == null)
    {
        return;
    }

    sch_stop_task(&scope_task);
    msg_unsubscribe_node(&scope_btn_sub);
    scope_page = null;
    heap_release_to_mark(scope_heap_mark);
}

/*
 *@ brief    : Check whether the Oscilloscope page is currently active.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if Scope page is active.
 *@ theme    : State query helper.
 */
bool gui_scope_is_active(void) { return scope_page != null; }
