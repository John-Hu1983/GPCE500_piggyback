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
static SWT_Timer scope_timer;
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
 *@ brief    : Compute and incrementally render one frame of the 3-channel 120-deg sine waves.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Old wave segments erased and new 3-phase sine curves drawn column-by-column.
 *@ theme    : Flicker-free incremental oscilloscope waveform renderer.
 */
static void _scope_render_frame(void)
{
    u8 col, ch;
    s16 old_prev[SCOPE_CHANNELS];
    s16 new_prev[SCOPE_CHANNELS];
    bool first_frame;

    if (scope_page == null)
    {
        return;
    }

    first_frame = !scope_page->initialized;

    for (col = 0U; col < SCOPE_WAVE_W; ++col)
    {
        s16 x = (s16)(SCOPE_WAVE_X0 + (s16)col);
        u8 base_idx =
            (u8)((scope_page->phase + (u16)col * (u16)scope_page->freq_step / 2U) % SCOPE_LUT_SIZE);
        s16 new_y[SCOPE_CHANNELS];

        /* 1. Compute new Y for all 3 channels (0 deg, 120 deg, 240 deg) at column `col` */
        for (ch = 0U; ch < SCOPE_CHANNELS; ++ch)
        {
            u8 lut_idx = (u8)((base_idx + ch * SCOPE_PHASE_120_STEPS) % SCOPE_LUT_SIZE);
            s16 offset = (s16)(((s32)scope_sin_lut[lut_idx] * (s32)scope_page->amplitude) / 127L);
            s16 y = (s16)(SCOPE_CENTER_Y - offset);

            if (y < SCOPE_WAVE_Y0)
            {
                y = SCOPE_WAVE_Y0;
            }
            if (y >= (s16)(SCOPE_WAVE_Y0 + (s16)SCOPE_WAVE_H))
            {
                y = (s16)(SCOPE_WAVE_Y0 + (s16)SCOPE_WAVE_H - 1);
            }
            new_y[ch] = y;
        }

        /* 2. Erase old segments at column `x` first so crossing channels never erase each other */
        if (!first_frame)
        {
            for (ch = 0U; ch < SCOPE_CHANNELS; ++ch)
            {
                s16 old_y = (s16)scope_page->prev_y[ch][col];
                if (col == 0U)
                {
                    gui_draw_point(x, old_y, _scope_get_bg_pixel(x, old_y));
                }
                else
                {
                    _scope_erase_segment(x, old_prev[ch], old_y);
                }
                old_prev[ch] = old_y;
            }
        }

        /* 3. Draw new segments for all 3 channels at column `x` */
        for (ch = 0U; ch < SCOPE_CHANNELS; ++ch)
        {
            if (col == 0U)
            {
                gui_draw_point(x, new_y[ch], scope_ch_colors[ch]);
            }
            else
            {
                s16 y0 = (new_prev[ch] <= new_y[ch]) ? new_prev[ch] : new_y[ch];
                s16 y1 = (new_prev[ch] <= new_y[ch]) ? new_y[ch] : new_prev[ch];
                gui_draw_vline(x, y0, y1, scope_ch_colors[ch]);
            }
            new_prev[ch] = new_y[ch];
            scope_page->prev_y[ch][col] = (u8)new_y[ch];
        }
    }

    scope_page->initialized = true;
}

/*
 *@ brief    : Periodic 32FPS software timer callback advancing the 3-phase sine wave phase.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Phase incremented and next frame rendered when not paused.
 *@ theme    : Real-time oscilloscope sweep timer.
 */
static void _scope_on_timer(void)
{
    if (scope_page == null || scope_page->paused)
    {
        return;
    }

    scope_page->phase = (u8)((scope_page->phase + SCOPE_PHASE_STEP) % SCOPE_LUT_SIZE);
    _scope_render_frame();
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
            _scope_render_frame();
        }
    }
    else if (btn->key == Btn_A5_Down)
    {
        if (scope_page->amplitude - SCOPE_AMP_STEP >= SCOPE_AMP_MIN)
        {
            scope_page->amplitude = (s16)(scope_page->amplitude - SCOPE_AMP_STEP);
            _scope_render_frame();
        }
    }
    else if (btn->key == Btn_A6_Right)
    {
        if (scope_page->freq_step < SCOPE_FREQ_MAX)
        {
            scope_page->freq_step++;
            _scope_render_frame();
        }
    }
    else if (btn->key == Btn_A4_Left)
    {
        if (scope_page->freq_step > SCOPE_FREQ_MIN)
        {
            scope_page->freq_step--;
            _scope_render_frame();
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
    scope_page->amplitude = SCOPE_AMP_DEFAULT;
    scope_page->paused = false;
    scope_page->initialized = false;

    _scope_draw_graticule();
    _scope_render_frame();

    msg_subscribe_type(&scope_btn_sub, MSG_Button, null, _scope_on_button_msg);
    swt_start_timer(&scope_timer, SCOPE_TICK_PERIOD, SWT_Repeat, _scope_on_timer);
}

/*
 *@ brief    : Exit the Oscilloscope page, stop sweep timer, and release ram_heap memory.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Timer stopped, button subscription detached, and heap memory freed.
 *@ theme    : Zero-fragmentation page exit via heap_release_to_mark().
 */
void gui_scope_exit(void)
{
    if (scope_page == null)
    {
        return;
    }

    swt_stop_timer(&scope_timer);
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
