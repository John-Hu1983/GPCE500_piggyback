#include "..\..\..\Include\spinmaster.h"

static const gpio_pin_s Pimd_En_Pin = PIMD_EN_PIN;
static const gpio_pin_s Pimd_Adc_Pin = PIMD_ADC_PIN;

static volatile u16 pimd_raw_samples[PIMD_SAMPLE_COUNT];
static volatile u8 pimd_sample_idx = 0U;
static volatile PIMD_Phy_Phase pimd_phy_phase = PIMD_Phy_Idle;
static volatile bool pimd_frame_ready = false;

/*
 *@ brief    : Stop hardware TimerC, DMA1/ADC auto-mode, and restore manual ADC idle state.
 *@ type     : private
 *@ argu in  : mark_ready: true when a full 12-sample frame has been acquired.
 *@ argu out : TimerC stopped, DMA1/ADC IRQ3 masked, CMPADC returned to manual discharge mode.
 *@ theme    : Clean hardware handoff so shared ADC remains available to main-loop callers.
 */
static void _pimd_finish_acquire(bool mark_ready)
{
    tmrc_stop_module();

#if (PIMD_PHY_MODE == PIMD_PHY_MODE_DMA)
    *P_DMA1_CTRL1 = C_DMA_Stop;
    *P_DMA1_CTRL0 = C_DMA_Disable;
    *P_INT_Ctrl &= ~C_IRQ3_DMA1;
    *P_DMA_INTSTS = C_DMA1_TX_Done_Flag | C_DMA1_TX_Error_Flag;
    *P_INT_Status = C_IRQ3_DMA1;
#else
    *P_INT_Ctrl &= ~C_IRQ3_ADC;
#endif

    *P_CMPADC_Ctrl1 = ADC_MODE;
    *P_CMPADC_Ctrl0 = ADC_CTRL0_BASE | C_CMPADC_Enable;
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;

    pimd_phy_phase = PIMD_Phy_Idle;
    if (mark_ready)
    {
        pimd_frame_ready = true;
    }
}

/*
 *@ brief    : Configure TimerC hardware trigger (50us) and start CMPADC Auto acquisition.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : TimerC IRQ2 masked while counter runs at 50us; DMA1 or IRQ3 armed for 12 samples.
 *@ theme    : Non-blocking hardware-triggered slope ADC acquisition (DMA or lightweight IRQ).
 */
static void _pimd_start_auto_adc(void)
{
    pimd_sample_idx = 0U;
    pimd_phy_phase = PIMD_Phy_Sample_Adc;

    /* Set TimerC to 50us interval and mask CPU IRQ2 so TimerC acts purely as hardware trigger */
    tmrc_set_period_us(PIMD_ADC_STEP_US);
    tmrc_disable_irq();

    /* Select IOA11 (PIMD_ADC_CH) with hardware 4us auto S&H discharge and 60uA ramp current */
    *P_CMPADC_Ctrl1 = C_CMPADC_Stop;
    *P_CMPADC_Ctrl0 = ((unsigned)PIMD_ADC_CH << 10) | C_CMPADC_SH_8us |
                      C_CMPADC_Hysteresis_Enable | C_CMPADC_IBIAS_60uA | C_CMPADC_Enable;
    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;

#if (PIMD_PHY_MODE == PIMD_PHY_MODE_DMA)
    /* Ultimate scheme: DMA1 moves 12 words from P_CMPADC_Data to pimd_raw_samples with 0 CPU IRQs */
    *P_DMA1_CTRL1 = C_DMA_Stop;
    *P_DMA1_CTRL0 = C_DMA_Disable;
    *P_DMA_INTSTS = C_DMA1_TX_Done_Flag | C_DMA1_TX_Error_Flag;
    *P_INT_Status = C_IRQ3_DMA1;

    *P_DMA1_SRCADR = (unsigned)(u32)P_CMPADC_Data;
    *P_DMA1_DSTADR = (unsigned)(u32)pimd_raw_samples;
    *P_DMA1_DTN = PIMD_SAMPLE_COUNT;
    *P_DMA1_CTRL1 = C_DMA_Double_Buf_Disable | C_DMA_Stop_Immdly;
    *P_DMA1_CTRL0 = C_DMA_Request_ADC | C_DMA_Cont_Disable | C_DMA_Burst_Disable |
                    C_DMA_Src_Inc_Disable | C_DMA_Dest_Inc_Enable | C_DMA_Circular_Disable |
                    C_DMA_Error_INT_Disable | C_DMA_Done_INT_Enable | C_DMA_Enable;

    *P_FIQ_Sel &= ~C_IRQ3_DMA1;
    *P_INT_Ctrl |= C_IRQ3_DMA1;

    *P_CMPADC_Ctrl1 = C_CMPADC_DMA_Enable | C_CMPADC_Auto_Enable | C_CMPADC_CMPO_None |
                      C_CMPADC_TMC | C_CMPADC_Start;
#else
    /* Lightweight IRQ scheme: TimerC triggers CMPADC in hardware; IRQ3 reads *P_CMPADC_Data */
    *P_FIQ_Sel &= ~C_IRQ3_ADC;
    *P_INT_Ctrl |= C_IRQ3_ADC;

    *P_CMPADC_Ctrl1 = C_CMPADC_DMA_Disable | C_CMPADC_Auto_Enable | C_CMPADC_CMPO_None |
                      C_CMPADC_TMC | C_CMPADC_Start;
#endif
}

/*
 *@ brief    : TimerC (IRQ2) callback handling the 250us pulse HIGH and 250us pre-ADC quiet wait.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Pulls P1_METAL_PULSE LOW at 250us, then launches hardware auto-ADC at 500us.
 *@ theme    : Microsecond pulse shaping before handing off to hardware-triggered ADC.
 */
static void _pimd_timerc_isr(void)
{
    if (pimd_phy_phase == PIMD_Phy_Pulse_High)
    {
        /* End of 250us excitation: pull P1_METAL_PULSE LOW and wait 250us before ADC window */
        gpio_write_level(Pimd_En_Pin.port, Pimd_En_Pin.pin, false);
        tmrc_set_period_us(PIMD_PRE_ADC_WAIT_US);
        pimd_phy_phase = PIMD_Phy_Pre_Adc_Wait;
    }
    else if (pimd_phy_phase == PIMD_Phy_Pre_Adc_Wait)
    {
        /* End of 500us pre-ADC delay: launch hardware TimerC-triggered Auto ADC (DMA or IRQ) */
        _pimd_start_auto_adc();
    }
    else
    {
        _pimd_finish_acquire(false);
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
 *@ argu out : P1_METAL_PULSE driven HIGH and TimerC armed for 250us excitation phase.
 *@ theme    : Start of 250us HIGH + 250us LOW + 600us hardware Auto-ADC burst.
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

    pimd_sample_idx = 0U;
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

    state->pulse_cnt++;
    state->v_floor_mv = v_min;
    state->v_peak_mv = v_max;
    state->rise_us = rise_us;
    state->raw_slope = max_slope;

    /* 5. Step-aware adaptive IIR filter: fast response (1/2) on transitions, steady (1/4) at rest */
    if (state->pulse_cnt <= 2U)
    {
        state->filt_slope = max_slope;
        state->filt_peak = v_max;
    }
    else
    {
        u16 slp_diff = (max_slope >= state->filt_slope) ? (u16)(max_slope - state->filt_slope)
                                                        : (u16)(state->filt_slope - max_slope);
        u16 pk_diff =
            (v_max >= state->filt_peak) ? (u16)(v_max - state->filt_peak) : (u16)(state->filt_peak - v_max);

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

    /* 7. Asymmetric baseline floor tracking */
    if (state->filt_slope < state->base_slope)
    {
        state->base_slope = (u16)(((u32)state->base_slope + (u32)state->filt_slope) / 2UL);
    }
    if (state->filt_peak < state->base_peak)
    {
        state->base_peak = (u16)(((u32)state->base_peak + (u32)state->filt_peak) / 2UL);
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

        /* 9. Slow upward drift tracking ONLY in quiet no-metal state */
        if (!state->metal_detected && state->d_slope <= 25 && state->d_peak <= 40)
        {
            jitter = (max_slope >= state->filt_slope) ? (u16)(max_slope - state->filt_slope)
                                                      : (u16)(state->filt_slope - max_slope);
            if (jitter <= 12U && (state->pulse_cnt & 0x07U) == 0U)
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
}

/*
 *@ brief    : Service IRQ3 for PIMD acquisition (DMA1 transfer done or ADC conversion complete).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Completes 12-sample acquisition in DMA mode, or reads 1 sample in lightweight IRQ mode.
 *@ theme    : Ultra-low-latency IRQ3 handler (< 1.5us) compatible with A1800 62.5us audio ISR.
 */
void pimd_service_irq(void)
{
#if (PIMD_PHY_MODE == PIMD_PHY_MODE_DMA)
    unsigned flags = *P_INT_Status & *P_INT_Ctrl & ~*P_FIQ_Sel & C_IRQ3_DMA1;

    if (flags == 0U || pimd_phy_phase != PIMD_Phy_Sample_Adc)
    {
        return;
    }

    if ((*P_DMA_INTSTS & (C_DMA1_TX_Done_Flag | C_DMA1_TX_Error_Flag)) != 0U)
    {
        _pimd_finish_acquire(true);
    }
    else
    {
        *P_INT_Status = C_IRQ3_DMA1;
    }
#else
    unsigned flags = *P_INT_Status & *P_INT_Ctrl & ~*P_FIQ_Sel & C_IRQ3_ADC;

    if (flags == 0U || pimd_phy_phase != PIMD_Phy_Sample_Adc)
    {
        return;
    }

    *P_CMPADC_Status = ADC_FLAGS;
    *P_INT_Status = C_IRQ3_ADC;

    if (pimd_sample_idx < PIMD_SAMPLE_COUNT)
    {
        pimd_raw_samples[pimd_sample_idx] = (u16)(*P_CMPADC_Data);
        pimd_sample_idx++;
    }

    if (pimd_sample_idx >= PIMD_SAMPLE_COUNT)
    {
        _pimd_finish_acquire(true);
    }
#endif
}
