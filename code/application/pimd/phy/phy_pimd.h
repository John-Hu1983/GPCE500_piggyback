#ifndef _PHY_PIMD_H__
#define _PHY_PIMD_H__

// clang-format off
/*
 * PIMD physical acquisition schemes:
 * - PIMD_PHY_MODE_DMA (0): Ultimate scheme — TimerC hardware trigger + CMPADC Auto + DMA1 transfer
 *                          (0 CPU interrupts during 12-sample burst; 1 DMA done IRQ at completion)
 * - PIMD_PHY_MODE_IRQ (1): Lightweight scheme — TimerC hardware trigger + CMPADC Auto + IRQ3 EOC
 *                          (1-line *P_CMPADC_Data register read per sample, < 1.5us ISR time)
 */
#define PIMD_PHY_MODE_DMA           0U
#define PIMD_PHY_MODE_IRQ           1U
#define PIMD_PHY_MODE               PIMD_PHY_MODE_DMA
#define PIMD_ADC_DETECT_EN          1U

/*
 * PIMD (Pulse Induction Metal Detector) hardware TimerC + ADC sampling parameters:
 * - Pulse HIGH width     : 250us HIGH on P1_METAL_PULSE (IOB1), then pulled LOW
 * - Pre-ADC quiet wait   : 150us LOW settling wait before first ADC read (total 400us)
 * - ADC sample window    : 12 samples @ 50us interval (400us .. 950us) on IOA11 (ADC_Ch_A11)
 */
#define PIMD_PULSE_HIGH_US          250U
#define PIMD_PRE_ADC_WAIT_US        150U
#define PIMD_SAMPLE_START_US        (PIMD_PULSE_HIGH_US + PIMD_PRE_ADC_WAIT_US)
#define PIMD_ADC_STEP_US            50U
#define PIMD_SAMPLE_COUNT           12U

#define PIMD_ADC_REF_MV             1500U
#define PIMD_DEFAULT_REF_RAW        2048U
#define PIMD_RISE_DETECT_MV         35U
#define PIMD_WARMUP_PULSES          12U
#define PIMD_CAL_PULSES             8U
#define PIMD_SLOPE_DEADBAND_MV      18U
#define PIMD_PEAK_DEADBAND_MV       30U
#define PIMD_DETECT_ON_TH           55U
#define PIMD_DETECT_OFF_TH          35U
#define PIMD_DETECT_HIT_COUNT       2U
// clang-format on

typedef enum
{
    PIMD_Phy_Idle = 0,
    PIMD_Phy_Pulse_High = 1,
    PIMD_Phy_Pre_Adc_Wait = 2,
    PIMD_Phy_Sample_Adc = 3
} PIMD_Phy_Phase;

typedef struct
{
    u8 hit_cnt;
    u8 stable_cnt;
    bool running;
    bool calibrated;
    bool metal_detected;
    bool beep_pending;
    u16 pulse_cnt;
    u16 ref_raw;
    u16 v_floor_mv;
    u16 v_peak_mv;
    u16 rise_us;
    u16 raw_slope;
    u16 filt_slope;
    u16 filt_peak;
    u16 base_slope;
    u16 base_peak;
    s16 d_slope;
    s16 d_peak;
    u16 sig_score;
    u32 cal_slope_sum;
    u32 cal_peak_sum;
} PIMD_Phy_State;

void pimd_init_phy(PIMD_Phy_State *state);
void pimd_start_phy(PIMD_Phy_State *state);
void pimd_stop_phy(PIMD_Phy_State *state);
void pimd_trigger_pulse(PIMD_Phy_State *state);
bool pimd_is_frame_ready(void);
bool pimd_is_phy_busy(void);
void pimd_process_frame(PIMD_Phy_State *state);
void pimd_service_irq(void);

#endif /* _PHY_PIMD_H__ */
