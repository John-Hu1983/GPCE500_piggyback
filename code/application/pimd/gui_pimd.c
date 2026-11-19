#include "..\..\Include\spinmaster.h"

static const char *const pimd_btn_labels[PIMD_BTN_COUNT] = {"Start", "Stop", "Quit"};

static const gpio_pin_s Pimd_En_Pin = PIMD_EN_PIN;
static const gpio_pin_s Pimd_Adc_Pin = PIMD_ADC_PIN;
static GUI_Pimd_Page *pimd_page = null;
static HEAP_Mark pimd_heap_mark = 0U;
static SWT_Timer pimd_timer;
static MSG_Sub pimd_btn_sub = MSG_SUB_INIT;

static volatile u16 pimd_raw_samples[PIMD_SAMPLE_COUNT];
static volatile u8 pimd_sample_idx = 0U;
static volatile PIMD_Isr_Phase pimd_isr_phase = PIMD_Isr_Idle;
static volatile bool pimd_frame_ready = false;

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
 *@ brief    : TimerC (IRQ2) interrupt handler driving P1_METAL_PULSE LOW and sampling ADC.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Pulls P1_METAL_PULSE LOW at 250us, waits 250us quiet time, then captures 12 ADC points.
 *@ theme    : Separate GPIO falling-edge transient from high-precision ADC ramp sampling.
 */
static void _pimd_timerc_isr(void)
{
    if (pimd_isr_phase == PIMD_Isr_Pulse_High)
    {
        /* End of 250us excitation: pull P1_METAL_PULSE LOW and wait 250us before ADC reads */
        gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);
        tmrc_set_period_us(PIMD_PRE_ADC_WAIT_US);
        pimd_isr_phase = PIMD_Isr_Pre_Adc_Wait;
    }
    else if (pimd_isr_phase == PIMD_Isr_Pre_Adc_Wait)
    {
        /* At t = 500us: switch TimerC to 50us sampling step and capture first ADC point */
        tmrc_set_period_us(PIMD_ADC_STEP_US);
        pimd_isr_phase = PIMD_Isr_Sample_Adc;
        pimd_raw_samples[0] = (u16)adc_read_channel_isr(PIMD_ADC_CH);
        pimd_sample_idx = 1U;
    }
    else if (pimd_isr_phase == PIMD_Isr_Sample_Adc)
    {
        if (pimd_sample_idx < PIMD_SAMPLE_COUNT)
        {
            pimd_raw_samples[pimd_sample_idx] = (u16)adc_read_channel_isr(PIMD_ADC_CH);
            pimd_sample_idx++;
        }

        if (pimd_sample_idx >= PIMD_SAMPLE_COUNT)
        {
            tmrc_stop_module();
            pimd_isr_phase = PIMD_Isr_Idle;
            pimd_frame_ready = true;
        }
    }
    else
    {
        tmrc_stop_module();
    }
}

/*
 *@ brief    : Trigger one PIMD measurement pulse: drive P1_METAL_PULSE HIGH and start TimerC.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : P1_METAL_PULSE driven HIGH and TimerC armed for PIMD_PULSE_HIGH_US (250us).
 *@ theme    : Start of hardware-timed excitation + pre-ADC quiet wait + ADC sampling sequence.
 */
static void _pimd_trigger_pulse(void)
{
    if (pimd_page == null || !pimd_page->running)
    {
        return;
    }

    /* Ensure SPU Channel 1 is silent before pulsing/sampling so audio PWM never perturbs +4V5 */
    spu_stop_channel(SPU_Channel_1);

    /* Sample internal 1.5V reference only during silent calibration before beeps can occur */
    if (!pimd_page->calibrated)
    {
        u16 ref = (u16)adc_read_channel(ADC_Ch_Ref);
        if (ref >= 200U)
        {
            pimd_page->ref_raw = ref;
        }
    }

    pimd_sample_idx = 0U;
    pimd_frame_ready = false;
    pimd_isr_phase = PIMD_Isr_Pulse_High;

    /* Drive P1_METAL_PULSE (IOB1) HIGH when starting TimerC */
    gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, true);
    tmrc_start_us(PIMD_PULSE_HIGH_US);
}

/*
 *@ brief    : Process the 12-point ADC ramp buffer to extract slope, peak, T0, and metal state.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Updates smoothed slope, peak, deadband-filtered score, and debounced metal_detected.
 *@ theme    : 3-point curve smoothing + 200us span slope + warmup/deadband/debounce detector.
 */
static void _pimd_process_frame(void)
{
    u16 raw_mv[PIMD_SAMPLE_COUNT];
    u16 sm_mv[PIMD_SAMPLE_COUNT];
    u16 ref_raw;
    u16 v_min = 0xFFFFU;
    u16 v_max = 0U;
    u8 max_idx = 0U;
    u16 max_slope = 0U;
    u16 rise_us = 0U;
    u16 jitter;
    u8 i;

    if (pimd_page == null)
    {
        return;
    }

    ref_raw = (pimd_page->ref_raw >= 200U) ? pimd_page->ref_raw : PIMD_DEFAULT_REF_RAW;

    /* 1. Convert raw ADC counts on P1_METAL_ADC (IOA11) to millivolts (mV) */
    for (i = 0U; i < PIMD_SAMPLE_COUNT; ++i)
    {
        u16 raw = pimd_raw_samples[i];
        raw_mv[i] =
            (u16)(((u32)raw * (u32)PIMD_ADC_REF_MV + (u32)(ref_raw / 2U)) / (u32)ref_raw);
    }

    /* 2. Apply 3-point [1, 2, 1]/4 smoothing along the curve to reject single-sample ADC spikes */
    sm_mv[0] = (u16)(((u32)raw_mv[0] * 3UL + (u32)raw_mv[1] + 2UL) / 4UL);
    sm_mv[PIMD_SAMPLE_COUNT - 1U] =
        (u16)(((u32)raw_mv[PIMD_SAMPLE_COUNT - 2U] + (u32)raw_mv[PIMD_SAMPLE_COUNT - 1U] * 3UL +
               2UL) /
              4UL);
    for (i = 1U; (u8)(i + 1U) < PIMD_SAMPLE_COUNT; ++i)
    {
        sm_mv[i] =
            (u16)(((u32)raw_mv[i - 1U] + (u32)raw_mv[i] * 2UL + (u32)raw_mv[i + 1U] + 2UL) / 4UL);
    }

    for (i = 0U; i < PIMD_SAMPLE_COUNT; ++i)
    {
        if (sm_mv[i] < v_min)
        {
            v_min = sm_mv[i];
        }
        if (sm_mv[i] >= v_max)
        {
            v_max = sm_mv[i];
            max_idx = i;
        }
    }

    /* 3. Locate rising-edge start time T0 (us) where smoothed curve rises above floor + 35mV */
    for (i = 0U; i < PIMD_SAMPLE_COUNT; ++i)
    {
        if (sm_mv[i] >= (u16)(v_min + PIMD_RISE_DETECT_MV))
        {
            rise_us = (u16)(PIMD_SAMPLE_START_US + (u16)i * PIMD_ADC_STEP_US);
            break;
        }
    }

    /* 4. Compute rising slope over a 4-step (200us) span normalized to mV/100us */
    for (i = 0U; (u8)(i + 4U) < PIMD_SAMPLE_COUNT; ++i)
    {
        if (i <= max_idx && sm_mv[i + 4U] > sm_mv[i])
        {
            u16 slp = (u16)((sm_mv[i + 4U] - sm_mv[i] + 1U) / 2U);
            if (slp > max_slope)
            {
                max_slope = slp;
            }
        }
    }
    if (max_slope == 0U && max_idx > 0U && sm_mv[max_idx] > v_min)
    {
        max_slope = (u16)((sm_mv[max_idx] - v_min + 1U) / 2U);
    }

    pimd_page->pulse_cnt++;
    pimd_page->v_floor_mv = v_min;
    pimd_page->v_peak_mv = v_max;
    pimd_page->rise_us = rise_us;
    pimd_page->raw_slope = max_slope;

    /* 5. Step-aware adaptive IIR filter: fast response (1/2) on transitions, steady (1/4) at rest */
    if (pimd_page->pulse_cnt <= 2U)
    {
        pimd_page->filt_slope = max_slope;
        pimd_page->filt_peak = v_max;
    }
    else
    {
        u16 slp_diff = (max_slope >= pimd_page->filt_slope)
                           ? (u16)(max_slope - pimd_page->filt_slope)
                           : (u16)(pimd_page->filt_slope - max_slope);
        u16 pk_diff = (v_max >= pimd_page->filt_peak) ? (u16)(v_max - pimd_page->filt_peak)
                                                      : (u16)(pimd_page->filt_peak - v_max);

        if (slp_diff > 15U)
        {
            pimd_page->filt_slope = (u16)(((u32)pimd_page->filt_slope + (u32)max_slope + 1UL) / 2UL);
        }
        else
        {
            pimd_page->filt_slope =
                (u16)(((u32)pimd_page->filt_slope * 3UL + (u32)max_slope + 2UL) / 4UL);
        }

        if (pk_diff > 25U)
        {
            pimd_page->filt_peak = (u16)(((u32)pimd_page->filt_peak + (u32)v_max + 1UL) / 2UL);
        }
        else
        {
            pimd_page->filt_peak = (u16)(((u32)pimd_page->filt_peak * 3UL + (u32)v_max + 2UL) / 4UL);
        }
    }

    /* 6. Skip warmup pulses (0.25s), then calibrate baseline over 8 pulses (0.25s) */
    if (!pimd_page->calibrated)
    {
        if (pimd_page->pulse_cnt > PIMD_WARMUP_PULSES)
        {
            pimd_page->cal_slope_sum += (u32)pimd_page->filt_slope;
            pimd_page->cal_peak_sum += (u32)pimd_page->filt_peak;
            if (pimd_page->pulse_cnt >= (u16)(PIMD_WARMUP_PULSES + PIMD_CAL_PULSES))
            {
                pimd_page->base_slope = (u16)(pimd_page->cal_slope_sum / (u32)PIMD_CAL_PULSES);
                pimd_page->base_peak = (u16)(pimd_page->cal_peak_sum / (u32)PIMD_CAL_PULSES);
                pimd_page->calibrated = true;
            }
        }
        pimd_page->d_slope = 0;
        pimd_page->d_peak = 0;
        pimd_page->sig_score = 0U;
        pimd_page->hit_cnt = 0U;
        pimd_page->stable_cnt = 0U;
        pimd_page->metal_detected = false;
        pimd_page->beep_pending = false;
        return;
    }

    /* 7. Asymmetric baseline floor tracking:
     *    - If filt drops below base (dSlp < 0 or dPk < 0), pull base down immediately (1/2 step)
     *      so dSlp/dPk never stay negative and sensitivity is never lost.
     *    - If filt is slightly above base while NO METAL is present, creep up very slowly (+1mV)
     *      to track temperature drift without ever learning a metal target as baseline.
     */
    if (pimd_page->filt_slope < pimd_page->base_slope)
    {
        pimd_page->base_slope =
            (u16)(((u32)pimd_page->base_slope + (u32)pimd_page->filt_slope) / 2UL);
    }
    if (pimd_page->filt_peak < pimd_page->base_peak)
    {
        pimd_page->base_peak =
            (u16)(((u32)pimd_page->base_peak + (u32)pimd_page->filt_peak) / 2UL);
    }

    pimd_page->d_slope = (s16)pimd_page->filt_slope - (s16)pimd_page->base_slope;
    pimd_page->d_peak = (s16)pimd_page->filt_peak - (s16)pimd_page->base_peak;

    {
        u16 eff_slp = (pimd_page->d_slope > (s16)PIMD_SLOPE_DEADBAND_MV)
                          ? (u16)(pimd_page->d_slope - (s16)PIMD_SLOPE_DEADBAND_MV)
                          : 0U;
        u16 eff_pk = (pimd_page->d_peak > (s16)PIMD_PEAK_DEADBAND_MV)
                         ? (u16)(pimd_page->d_peak - (s16)PIMD_PEAK_DEADBAND_MV)
                         : 0U;

        pimd_page->sig_score = (u16)(eff_slp * 2U + eff_pk);

        /* 8. Fast 2-frame debounce on enter (beep once on rising edge), release below OFF_TH */
        if (!pimd_page->metal_detected)
        {
            if (pimd_page->sig_score >= PIMD_DETECT_ON_TH)
            {
                if (pimd_page->hit_cnt < PIMD_DETECT_HIT_COUNT)
                {
                    pimd_page->hit_cnt++;
                }
                if (pimd_page->hit_cnt >= PIMD_DETECT_HIT_COUNT)
                {
                    pimd_page->metal_detected = true;
                    pimd_page->beep_pending = true;
                }
            }
            else if (pimd_page->hit_cnt > 0U)
            {
                pimd_page->hit_cnt--;
            }
        }
        else
        {
            if (pimd_page->sig_score < PIMD_DETECT_OFF_TH)
            {
                pimd_page->metal_detected = false;
                pimd_page->hit_cnt = 0U;
            }
        }

        /* 9. Slow upward drift tracking ONLY in quiet no-metal state (never during METAL!) */
        if (!pimd_page->metal_detected && pimd_page->d_slope <= 25 && pimd_page->d_peak <= 40)
        {
            jitter = (max_slope >= pimd_page->filt_slope)
                         ? (u16)(max_slope - pimd_page->filt_slope)
                         : (u16)(pimd_page->filt_slope - max_slope);
            if (jitter <= 12U && (pimd_page->pulse_cnt & 0x07U) == 0U)
            {
                if (pimd_page->filt_slope > pimd_page->base_slope)
                {
                    pimd_page->base_slope++;
                }
                if (pimd_page->filt_peak > pimd_page->base_peak)
                {
                    pimd_page->base_peak++;
                }
            }
        }
    }
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
    u8 pos;

    if (pimd_page == null)
    {
        return;
    }

    /* Update top-right state badge textbox */
    if (!pimd_page->running)
    {
        pimd_page->status_box.bg_color = GUI_RGB565(64, 74, 92);
        pimd_page->status_box.border_color = GUI_RGB565(96, 112, 138);
        pimd_page->status_box.text_color = GUI_COLOR_WHITE;
        gui_textbox_set_text(&pimd_page->status_box, "STOP");
    }
    else if (!pimd_page->calibrated)
    {
        pimd_page->status_box.bg_color = GUI_RGB565(180, 120, 0);
        pimd_page->status_box.border_color = GUI_RGB565(240, 180, 40);
        pimd_page->status_box.text_color = GUI_COLOR_WHITE;
        gui_textbox_set_text(&pimd_page->status_box, "CAL...");
    }
    else if (pimd_page->metal_detected)
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
    _pimd_append_s16_delta(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, pimd_page->d_slope);
    _pimd_append_str(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, " (");
    _pimd_append_u16(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, pimd_page->filt_slope);
    _pimd_append_str(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, "/");
    _pimd_append_u16(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, pimd_page->base_slope);
    _pimd_append_str(pimd_page->info_bufs[0], &pos, PIMD_INFO_TEXT_LEN, ")");
    pimd_page->info_boxes[0].text_color =
        pimd_page->metal_detected ? GUI_RGB565(255, 220, 80) : GUI_RGB565(120, 240, 255);
    gui_textbox_set_text(&pimd_page->info_boxes[0], pimd_page->info_bufs[0]);

    /* Row 1: Net peak change dPk & current Peak (mV) */
    pos = 0U;
    _pimd_append_str(pimd_page->info_bufs[1], &pos, PIMD_INFO_TEXT_LEN, "dPk :");
    _pimd_append_s16_delta(pimd_page->info_bufs[1], &pos, PIMD_INFO_TEXT_LEN, pimd_page->d_peak);
    _pimd_append_str(pimd_page->info_bufs[1], &pos, PIMD_INFO_TEXT_LEN, " (Pk:");
    _pimd_append_u16(pimd_page->info_bufs[1], &pos, PIMD_INFO_TEXT_LEN, pimd_page->filt_peak);
    _pimd_append_str(pimd_page->info_bufs[1], &pos, PIMD_INFO_TEXT_LEN, "mV)");
    pimd_page->info_boxes[1].text_color = GUI_RGB565(200, 230, 255);
    gui_textbox_set_text(&pimd_page->info_boxes[1], pimd_page->info_bufs[1]);

    /* Row 2: Rising-edge onset delay T0 (us) & floor voltage (mV) */
    pos = 0U;
    _pimd_append_str(pimd_page->info_bufs[2], &pos, PIMD_INFO_TEXT_LEN, "T0:");
    _pimd_append_u16(pimd_page->info_bufs[2], &pos, PIMD_INFO_TEXT_LEN, pimd_page->rise_us);
    _pimd_append_str(pimd_page->info_bufs[2], &pos, PIMD_INFO_TEXT_LEN, "us Flr:");
    _pimd_append_u16(pimd_page->info_bufs[2], &pos, PIMD_INFO_TEXT_LEN, pimd_page->v_floor_mv);
    _pimd_append_str(pimd_page->info_bufs[2], &pos, PIMD_INFO_TEXT_LEN, "mV");
    pimd_page->info_boxes[2].text_color = GUI_RGB565(180, 220, 190);
    gui_textbox_set_text(&pimd_page->info_boxes[2], pimd_page->info_bufs[2]);

    /* Row 3: Deadband-filtered signal score, threshold (55), and hit counter (0..2) */
    pos = 0U;
    _pimd_append_str(pimd_page->info_bufs[3], &pos, PIMD_INFO_TEXT_LEN, "Sig:");
    _pimd_append_u16(pimd_page->info_bufs[3], &pos, PIMD_INFO_TEXT_LEN, pimd_page->sig_score);
    _pimd_append_str(pimd_page->info_bufs[3], &pos, PIMD_INFO_TEXT_LEN, "/55 Hit:");
    _pimd_append_u16(pimd_page->info_bufs[3], &pos, PIMD_INFO_TEXT_LEN, (u16)pimd_page->hit_cnt);
    _pimd_append_str(pimd_page->info_bufs[3], &pos, PIMD_INFO_TEXT_LEN, "/2");
    pimd_page->info_boxes[3].text_color =
        pimd_page->metal_detected ? GUI_RGB565(255, 110, 110) : GUI_RGB565(170, 195, 230);
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
 *@ brief    : Periodic 100ms software timer callback processing completed frames and re-triggering.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Evaluates previous pulse ADC samples, refreshes UI textboxes, and triggers next pulse.
 *@ theme    : Decouples main-loop UI/algorithm work from microsecond TimerC ISR sampling.
 */
static void _pimd_cycle_timer_cb(void)
{
    if (pimd_page == null || !pimd_page->running)
    {
        tmrc_stop_module();
        spu_stop_channel(SPU_Channel_1);
        gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);
        return;
    }

    /* 1. Process previous completed ADC frame first (while speaker is still off) */
    if (pimd_frame_ready)
    {
        pimd_frame_ready = false;
        _pimd_process_frame();
    }

    /* 2. Trigger new 1.05ms TimerC pulse + ADC burst BEFORE playing audio or drawing LCD */
    _pimd_trigger_pulse();

    /* 3. Refresh LCD textboxes (~12-15ms SPI draw; TimerC finishes ADC burst at ~1.05ms) */
    _pimd_update_textboxes();

    /* 4. Beep once on newly detected metal after TimerC ADC capture has finished */
    if (pimd_page->beep_pending)
    {
        pimd_page->beep_pending = false;
        if (gui_setting_get_volume() > 0U)
        {
            spu_play_tone(SPU_Channel_1, TONE_Wave_Pulse, GAME_TONE_SCORE, TONE_VOL_HIGH);
        }
    }

    swt_start_timer(&pimd_timer, PIMD_CYCLE_TICKS, SWT_Once, _pimd_cycle_timer_cb);
}

/*
 *@ brief    : Reset baseline calibration and start periodic TimerC-driven PIMD measurements.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Calibration counters cleared, first pulse triggered, and 100ms cycle timer armed.
 *@ theme    : Button-controlled measurement start action.
 */
static void _pimd_start_output(void)
{
    if (pimd_page == null)
    {
        return;
    }

    tmrc_stop_module();
    gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);

    pimd_page->running = true;
    pimd_page->calibrated = false;
    pimd_page->metal_detected = false;
    pimd_page->beep_pending = false;
    pimd_page->hit_cnt = 0U;
    pimd_page->stable_cnt = 0U;
    pimd_page->pulse_cnt = 0U;
    pimd_page->cal_slope_sum = 0UL;
    pimd_page->cal_peak_sum = 0UL;
    pimd_page->d_slope = 0;
    pimd_page->d_peak = 0;
    pimd_page->sig_score = 0U;

    _pimd_update_textboxes();
    _pimd_trigger_pulse();
    swt_start_timer(&pimd_timer, PIMD_CYCLE_TICKS, SWT_Once, _pimd_cycle_timer_cb);
}

/*
 *@ brief    : Stop PIMD measurement cycles, disable TimerC, and force P1_METAL_PULSE LOW.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Software timer and TimerC stopped, IOB1 driven LOW, status textbox set to STOP.
 *@ theme    : Button-controlled measurement stop action.
 */
static void _pimd_stop_output(void)
{
    swt_stop_timer(&pimd_timer);
    tmrc_stop_module();
    spu_stop_channel(SPU_Channel_1);
    pimd_isr_phase = PIMD_Isr_Idle;
    pimd_frame_ready = false;
    gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);

    if (pimd_page != null)
    {
        pimd_page->running = false;
        pimd_page->metal_detected = false;
        pimd_page->beep_pending = false;
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
 *@ brief    : Enter the PIMD page, initialize IOB1/IOA11 & TimerC, and render UI.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Hardware pins/TimerC configured and GUI_Textbox status panel + buttons drawn.
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

    /* Configure IOB1 (P1_METAL_PULSE) output LOW, IOA11 (P1_METAL_ADC) float, and TimerC ISR */
    gpio_init_pattern(Pimd_En_Pin.port, Pimd_En_Pin.pin, GPIO_Out_Low);
    gpio_init_pattern(Pimd_Adc_Pin.port, Pimd_Adc_Pin.pin, GPIO_In_Float);
    tmrc_init_module(_pimd_timerc_isr);

    pimd_heap_mark = heap_get_mark();
    pimd_page = HEAP_NEW(GUI_Pimd_Page);
    if (pimd_page == null)
    {
        bsp_halt_fault(BSP_Fault_HEAP_State);
    }

    pimd_page->selected_idx = (u8)PIMD_Btn_Start;
    pimd_page->hit_cnt = 0U;
    pimd_page->stable_cnt = 0U;
    pimd_page->running = false;
    pimd_page->calibrated = false;
    pimd_page->metal_detected = false;
    pimd_page->beep_pending = false;
    pimd_page->pulse_cnt = 0U;
    pimd_page->ref_raw = PIMD_DEFAULT_REF_RAW;
    pimd_page->v_floor_mv = 0U;
    pimd_page->v_peak_mv = 0U;
    pimd_page->rise_us = 0U;
    pimd_page->raw_slope = 0U;
    pimd_page->filt_slope = 0U;
    pimd_page->filt_peak = 0U;
    pimd_page->base_slope = 0U;
    pimd_page->base_peak = 0U;
    pimd_page->d_slope = 0;
    pimd_page->d_peak = 0;
    pimd_page->sig_score = 0U;
    pimd_page->cal_slope_sum = 0UL;
    pimd_page->cal_peak_sum = 0UL;

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
 *@ brief    : Exit the PIMD page, stop TimerC and pulse output, and release ram_heap memory.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : TimerC and software timer stopped, IOB1 driven LOW, and heap reclaimed.
 *@ theme    : Zero-fragmentation page exit via heap_release_to_mark().
 */
void gui_pimd_exit(void)
{
    if (pimd_page == null)
    {
        return;
    }

    swt_stop_timer(&pimd_timer);
    tmrc_stop_module();
    spu_stop_channel(SPU_Channel_1);
    pimd_isr_phase = PIMD_Isr_Idle;
    pimd_frame_ready = false;
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
