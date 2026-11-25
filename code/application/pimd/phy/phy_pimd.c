#include "..\..\..\Include\spinmaster.h"

static const gpio_pin_s Pimd_En_Pin = PIMD_EN_PIN;
static const gpio_pin_s Pimd_Adc_Pin = PIMD_ADC_PIN;

static volatile u16 pimd_raw_samples[PIMD_SAMPLE_COUNT];
static volatile PIMD_Phy_Phase pimd_phy_phase = PIMD_Phy_Idle;
static volatile bool pimd_frame_ready = false;

/*
 *@ brief    : Stop hardware TimerC and ADC auto-sampling, and return PHY to idle state.
 *@ type     : private
 *@ argu in  : mark_ready: true when a full 12-sample frame has been acquired.
 *@ argu out : TimerC and ADC auto-burst stopped; pimd_frame_ready set when mark_ready is true.
 *@ theme    : Clean hardware handoff via HAL so shared ADC remains available to main-loop callers.
 */
static void _pimd_finish_acquire(bool mark_ready)
{
    tmrc_stop_module();
    adc_stop_auto_sample();

    pimd_phy_phase = PIMD_Phy_Idle;
    if (mark_ready)
    {
        pimd_frame_ready = true;
    }
}

/*
 *@ brief    : Callback invoked by hal_adc when all 16 slope samples have been captured.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Stops TimerC trigger and marks the 16-sample frame ready for pimd_process_frame().
 *@ theme    : End-of-burst notification from ADC DMA1 or IRQ3 completion.
 */
static void _pimd_adc_done_isr(void) { _pimd_finish_acquire(true); }

/*
 *@ brief    : Configure TimerC hardware trigger (75us) and start HAL ADC auto acquisition.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : TimerC IRQ2 masked while counter runs at 75us; HAL ADC armed for 16 samples.
 *@ theme    : Non-blocking hardware-triggered slope ADC burst (DMA or lightweight IRQ).
 */
static void _pimd_start_auto_adc(void)
{
    pimd_phy_phase = PIMD_Phy_Sample_Adc;

    /* Set TimerC to 75us (3x 25us backlight PWM period) and mask CPU IRQ2 for pure HW trigger */
    tmrc_set_period_us(PIMD_ADC_STEP_US);
    tmrc_disable_irq();

#if (PIMD_PHY_MODE == PIMD_PHY_MODE_DMA)
    adc_start_auto_dma(PIMD_ADC_CH, pimd_raw_samples, PIMD_SAMPLE_COUNT, _pimd_adc_done_isr);
#else
    adc_start_auto_irq(PIMD_ADC_CH, pimd_raw_samples, PIMD_SAMPLE_COUNT, _pimd_adc_done_isr);
#endif
}

/*
 *@ brief    : TimerC (IRQ2) callback handling the 80us pulse HIGH and 300us pre-ADC quiet wait.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Pulls P1_METAL_PULSE LOW at 80us, then launches hardware auto-ADC at +300us.
 *@ theme    : Microsecond pulse shaping before handing off to hardware-triggered ADC.
 */
static void _pimd_timerc_isr(void)
{
    switch (pimd_phy_phase)
    {
    case PIMD_Phy_Pulse_High:
        /* End of 80us excitation: pull P1_METAL_PULSE LOW */
        gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);
        tmrc_set_period_us(PIMD_PRE_ADC_WAIT_US);
        pimd_phy_phase = PIMD_Phy_Pre_Adc_Wait;
        break;

    case PIMD_Phy_Pre_Adc_Wait:
        /* End of 300us pre-ADC delay: launch hardware TimerC-triggered Auto ADC (375..1500us) */
        _pimd_start_auto_adc();
        break;

    case PIMD_Phy_Sample_Adc:
    case PIMD_Phy_Idle:
    default:
        _pimd_finish_acquire(false);
        break;
    }
}

/*
 *@ brief    : Initialize PIMD physical hardware (IOB1 pulse pin, IOA11 ADC pin, and TimerC).
 *@ type     : public
 *@ argu in  : state: pointer to PIMD_Phy_State descriptor.
 *@ argu out : GPIO pins, TimerC callback, and initial state fields configured.
 *@ theme    : Physical layer initialization on page entry.
 */
void pimd_init_phy(PIMD_Phy_State *state)
{
    gpio_init_pattern(Pimd_En_Pin.port, Pimd_En_Pin.pin, GPIO_Out_Low);
    gpio_init_pattern(Pimd_Adc_Pin.port, Pimd_Adc_Pin.pin, GPIO_In_Float);
    tmrc_init_module(_pimd_timerc_isr);
    _pimd_finish_acquire(false);
    pimd_frame_ready = false;

    if (state != null)
    {
        state->hit_cnt = 0U;
        state->stable_cnt = 0U;
        state->running = false;
        state->calibrated = false;
        state->metal_detected = false;
        state->beep_pending = false;
        state->pulse_cnt = 0U;
        state->ref_raw = PIMD_DEFAULT_REF_RAW;
        state->v_floor_mv = 0U;
        state->v_peak_mv = 0U;
        state->rise_us = 0U;
        state->raw_slope = 0U;
        state->filt_slope = 0U;
        state->filt_peak = 0U;
        state->base_slope = 0U;
        state->base_peak = 0U;
        state->d_slope = 0;
        state->d_peak = 0;
        state->sig_score = 0U;
        state->cal_slope_sum = 0UL;
        state->cal_peak_sum = 0UL;
    }
}

/*
 *@ brief    : Reset baseline calibration state and enable PIMD physical measurement.
 *@ type     : public
 *@ argu in  : state: pointer to PIMD_Phy_State descriptor.
 *@ argu out : Calibration accumulators cleared and state->running set true.
 *@ theme    : Start measurement session.
 */
void pimd_start_phy(PIMD_Phy_State *state)
{
    if (state == null)
    {
        return;
    }

    _pimd_finish_acquire(false);
    pimd_frame_ready = false;
    gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);

    state->running = true;
    state->calibrated = false;
    state->metal_detected = false;
    state->beep_pending = false;
    state->hit_cnt = 0U;
    state->stable_cnt = 0U;
    state->pulse_cnt = 0U;
    state->cal_slope_sum = 0UL;
    state->cal_peak_sum = 0UL;
    state->d_slope = 0;
    state->d_peak = 0;
    state->sig_score = 0U;
}

/*
 *@ brief    : Stop PIMD physical acquisition, disable TimerC/DMA/ADC, and pull IOB1 LOW.
 *@ type     : public
 *@ argu in  : state: pointer to PIMD_Phy_State descriptor (may be null).
 *@ argu out : Hardware timers/DMA stopped, IOB1 driven LOW, state->running cleared.
 *@ theme    : Safe hardware shutdown on Stop button or page exit.
 */
void pimd_stop_phy(PIMD_Phy_State *state)
{
    _pimd_finish_acquire(false);
    pimd_frame_ready = false;
    gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);

    if (state != null)
    {
        state->running = false;
        state->metal_detected = false;
        state->beep_pending = false;
    }
}

/*
 *@ brief    : Trigger one hardware excitation pulse on P1_METAL_PULSE and start TimerC sequence.
 *@ type     : public
 *@ argu in  : state: pointer to PIMD_Phy_State descriptor.
 *@ argu out : P1_METAL_PULSE driven HIGH and TimerC armed for 80us excitation phase.
 *@ theme    : Start of 80us HIGH + 300us LOW + 1200us hardware Auto-ADC burst.
 */
void pimd_trigger_pulse(PIMD_Phy_State *state)
{
    if (state == null || !state->running)
    {
        return;
    }

    _pimd_finish_acquire(false);

    /* Sample internal 1.5V reference only during silent calibration before beeps can occur */
    if (!state->calibrated)
    {
        u16 ref = (u16)adc_read_channel(ADC_Ch_Ref);
        if (ref >= 200U)
        {
            state->ref_raw = ref;
        }
    }

    pimd_frame_ready = false;
    pimd_phy_phase = PIMD_Phy_Pulse_High;

    /* Drive P1_METAL_PULSE (IOB1) HIGH when starting TimerC */
    gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, true);
    tmrc_start_us(PIMD_PULSE_HIGH_US);
}

/*
 *@ brief    : Query and clear the completed 12-sample frame ready flag.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if a new 12-sample frame is ready for pimd_process_frame().
 *@ theme    : Handshake between IRQ3 completion and main-loop processing.
 */
bool pimd_is_frame_ready(void)
{
    if (pimd_frame_ready)
    {
        pimd_frame_ready = false;
        return true;
    }
    return false;
}

/*
 *@ brief    : Check whether PIMD hardware acquisition is currently active on TimerC/CMPADC.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if excitation or ADC sampling is in progress.
 *@ theme    : Prevent main-loop battery ADC polling from colliding with Auto-ADC burst.
 */
bool pimd_is_phy_busy(void) { return pimd_phy_phase != PIMD_Phy_Idle; }

/*
 *@ brief    : Process the 12-point ADC ramp buffer to extract slope, peak, T0, and metal state.
 *@ type     : public
 *@ argu in  : state: pointer to PIMD_Phy_State descriptor.
 *@ argu out : Updates smoothed slope, peak, deadband-filtered score, and debounced metal_detected.
 *@ theme    : 3-point curve smoothing + 200us span slope + floor-following baseline detector.
 */
void pimd_process_frame(PIMD_Phy_State *state)
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

    if (state == null)
    {
        return;
    }

    ref_raw = (state->ref_raw >= 200U) ? state->ref_raw : PIMD_DEFAULT_REF_RAW;

    /* 1. Convert raw ADC counts on P1_METAL_ADC (IOA11) to millivolts (mV) */
    for (i = 0U; i < PIMD_SAMPLE_COUNT; ++i)
    {
        u16 raw = pimd_raw_samples[i];
        raw_mv[i] = (u16)(((u32)raw * (u32)PIMD_ADC_REF_MV + (u32)(ref_raw / 2U)) / (u32)ref_raw);
    }

    /* 2. Apply 3-point [1, 2, 1]/4 smoothing along the curve to reject single-sample ADC spikes */
    sm_mv[0] = (u16)(((u32)raw_mv[0] * 3UL + (u32)raw_mv[1] + 2UL) / 4UL);
    sm_mv[PIMD_SAMPLE_COUNT - 1U] = (u16)(((u32)raw_mv[PIMD_SAMPLE_COUNT - 2U] +
                                           (u32)raw_mv[PIMD_SAMPLE_COUNT - 1U] * 3UL + 2UL) /
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

    /* 4. Compute rising slope over a 3-step (225us @ 75us/step) span normalized to mV/100us */
    for (i = 0U; (u8)(i + PIMD_SLOPE_SPAN_STEPS) < PIMD_SAMPLE_COUNT; ++i)
    {
        if (i <= max_idx && sm_mv[i + PIMD_SLOPE_SPAN_STEPS] > sm_mv[i])
        {
            u16 diff_mv = (u16)(sm_mv[i + PIMD_SLOPE_SPAN_STEPS] - sm_mv[i]);
            u16 slp = (u16)(((u32)diff_mv * 4UL + 4UL) / 9UL);
            if (slp > max_slope)
            {
                max_slope = slp;
            }
        }
    }
    if (max_slope == 0U && max_idx > 0U && sm_mv[max_idx] > v_min)
    {
        max_slope = (u16)(((u32)(sm_mv[max_idx] - v_min) * 4UL + 4UL) / 9UL);
    }

    state->pulse_cnt++;
    state->v_floor_mv = v_min;
    state->v_peak_mv = v_max;
    state->rise_us = rise_us;
    state->raw_slope = max_slope;

    /* 5. Step-aware adaptive IIR filter: fast response (1/2) on transitions, steady (1/4) at rest
     */
    if (state->pulse_cnt <= 2U)
    {
        state->filt_slope = max_slope;
        state->filt_peak = v_max;
    }
    else
    {
        u16 slp_diff = (max_slope >= state->filt_slope) ? (u16)(max_slope - state->filt_slope)
                                                        : (u16)(state->filt_slope - max_slope);
        u16 pk_diff = (v_max >= state->filt_peak) ? (u16)(v_max - state->filt_peak)
                                                  : (u16)(state->filt_peak - v_max);

        if (slp_diff > 15U)
        {
            state->filt_slope = (u16)(((u32)state->filt_slope + (u32)max_slope + 1UL) / 2UL);
        }
        else
        {
            state->filt_slope = (u16)(((u32)state->filt_slope * 3UL + (u32)max_slope + 2UL) / 4UL);
        }

        if (pk_diff > 25U)
        {
            state->filt_peak = (u16)(((u32)state->filt_peak + (u32)v_max + 1UL) / 2UL);
        }
        else
        {
            state->filt_peak = (u16)(((u32)state->filt_peak * 3UL + (u32)v_max + 2UL) / 4UL);
        }
    }

    /* 6. Skip warmup pulses, then calibrate baseline over PIMD_CAL_PULSES */
    if (!state->calibrated)
    {
        if (state->pulse_cnt > PIMD_WARMUP_PULSES)
        {
            state->cal_slope_sum += (u32)state->filt_slope;
            state->cal_peak_sum += (u32)state->filt_peak;
            if (state->pulse_cnt >= (u16)(PIMD_WARMUP_PULSES + PIMD_CAL_PULSES))
            {
                state->base_slope = (u16)(state->cal_slope_sum / (u32)PIMD_CAL_PULSES);
                state->base_peak = (u16)(state->cal_peak_sum / (u32)PIMD_CAL_PULSES);
                state->calibrated = true;
            }
        }
        state->d_slope = 0;
        state->d_peak = 0;
        state->sig_score = 0U;
        state->hit_cnt = 0U;
        state->stable_cnt = 0U;
        state->metal_detected = false;
        state->beep_pending = false;
        return;
    }

    /* 7. Stability-gated baseline tracking (prevents one-way downward ratchet deadlock) */
    jitter = (max_slope >= state->filt_slope) ? (u16)(max_slope - state->filt_slope)
                                              : (u16)(state->filt_slope - max_slope);
    if (jitter <= 12U)
    {
        if (state->stable_cnt < 20U)
        {
            state->stable_cnt++;
        }
    }
    else
    {
        state->stable_cnt = 0U;
    }

    if (state->stable_cnt >= 3U)
    {
        if (state->filt_slope < state->base_slope)
        {
            u16 step = (u16)((state->base_slope - state->filt_slope + 7U) / 8U);
            state->base_slope = (u16)(state->base_slope - step);
        }
        if (state->filt_peak < state->base_peak)
        {
            u16 step = (u16)((state->base_peak - state->filt_peak + 7U) / 8U);
            state->base_peak = (u16)(state->base_peak - step);
        }
    }

    state->d_slope = (s16)state->filt_slope - (s16)state->base_slope;
    state->d_peak = (s16)state->filt_peak - (s16)state->base_peak;

    {
        u16 eff_slp = (state->d_slope > (s16)PIMD_SLOPE_DEADBAND_MV)
                          ? (u16)(state->d_slope - (s16)PIMD_SLOPE_DEADBAND_MV)
                          : 0U;
        u16 eff_pk = (state->d_peak > (s16)PIMD_PEAK_DEADBAND_MV)
                         ? (u16)(state->d_peak - (s16)PIMD_PEAK_DEADBAND_MV)
                         : 0U;

        state->sig_score = (u16)(eff_slp * 2U + eff_pk);

        /* 8. Fast 2-frame debounce on enter (beep once on rising edge), release below OFF_TH */
        if (!state->metal_detected)
        {
            if (state->sig_score >= PIMD_DETECT_ON_TH)
            {
                if (state->hit_cnt < PIMD_DETECT_HIT_COUNT)
                {
                    state->hit_cnt++;
                }
                if (state->hit_cnt >= PIMD_DETECT_HIT_COUNT)
                {
                    state->metal_detected = true;
                    state->beep_pending = true;
                }
            }
            else if (state->hit_cnt > 0U)
            {
                state->hit_cnt--;
            }
        }
        else
        {
            if (state->sig_score < PIMD_DETECT_OFF_TH)
            {
                state->metal_detected = false;
                state->hit_cnt = 0U;
            }
        }

        /* 9. Slow upward drift tracking when stationary and not in active metal state */
        if (!state->metal_detected && state->stable_cnt >= 4U && (state->pulse_cnt & 0x03U) == 0U)
        {
            if (state->filt_slope > state->base_slope)
            {
                state->base_slope++;
            }
            if (state->filt_peak > state->base_peak)
            {
                state->base_peak++;
            }
        }
    }
}
