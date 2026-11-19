#ifndef _GUI_PIMD_H__
#define _GUI_PIMD_H__

// clang-format off
/*
 * PIMD (Pulse Induction Metal Detector) hardware TimerC + ADC sampling parameters:
 * - Pulse trigger period : 110 RTC ticks @ 2048Hz (~53.7ms per frame, ~18.6Hz pulse rate)
 * - Pulse HIGH width     : 250us HIGH on P1_METAL_PULSE (IOB1), then pulled LOW
 * - Pre-ADC quiet wait   : 250us LOW settling wait before first ADC read (total 500us)
 * - ADC sample window    : 12 samples @ 50us interval (500us .. 1050us) on IOA11 (ADC_Ch_A11)
 */
#define PIMD_CYCLE_TICKS            110U
#define PIMD_PULSE_HIGH_US          250U
#define PIMD_PRE_ADC_WAIT_US        250U
#define PIMD_SAMPLE_START_US        (PIMD_PULSE_HIGH_US + PIMD_PRE_ADC_WAIT_US)
#define PIMD_ADC_STEP_US            50U
#define PIMD_SAMPLE_COUNT           12U

#define PIMD_ADC_REF_MV             1500U
#define PIMD_DEFAULT_REF_RAW        2048U
#define PIMD_RISE_DETECT_MV         35U
#define PIMD_WARMUP_PULSES          6U
#define PIMD_CAL_PULSES             6U
#define PIMD_SLOPE_DEADBAND_MV      18U
#define PIMD_PEAK_DEADBAND_MV       30U
#define PIMD_DETECT_ON_TH           55U
#define PIMD_DETECT_OFF_TH          35U
#define PIMD_DETECT_HIT_COUNT       2U

#define PIMD_BG_COLOR           GUI_RGB565(18, 22, 34)
#define PIMD_PANEL_BG           GUI_RGB565(26, 34, 52)
#define PIMD_PANEL_BORDER       GUI_RGB565(56, 80, 120)
#define PIMD_BOX_BG             GUI_RGB565(14, 20, 32)
#define PIMD_BOX_BORDER         GUI_RGB565(45, 66, 98)

#define PIMD_INFO_BOX_COUNT     4U
#define PIMD_INFO_TEXT_LEN      24U

#define PIMD_BTN_COUNT          3U
#define PIMD_BTN_MARGIN_X       6U
#define PIMD_BTN_GAP_X          6U
#define PIMD_BTN_W              ((LCD_WIDTH - 2U * PIMD_BTN_MARGIN_X - (PIMD_BTN_COUNT - 1U) * PIMD_BTN_GAP_X) / PIMD_BTN_COUNT)
#define PIMD_BTN_X0             ((LCD_WIDTH - (PIMD_BTN_COUNT * PIMD_BTN_W + (PIMD_BTN_COUNT - 1U) * PIMD_BTN_GAP_X)) / 2U)
#define PIMD_BTN_H              24U
#define PIMD_BTN_Y              (s16)(LCD_HEIGHT - PIMD_BTN_H - 6U)
// clang-format on

typedef enum
{
    PIMD_Btn_Start = 0,
    PIMD_Btn_Stop = 1,
    PIMD_Btn_Quit = 2
} PIMD_Btn_Id;

typedef enum
{
    PIMD_Isr_Idle = 0,
    PIMD_Isr_Pulse_High = 1,
    PIMD_Isr_Pre_Adc_Wait = 2,
    PIMD_Isr_Sample_Adc = 3
} PIMD_Isr_Phase;

typedef struct
{
    GUI_Button buttons[PIMD_BTN_COUNT];
    GUI_Textbox status_box;
    GUI_Textbox info_boxes[PIMD_INFO_BOX_COUNT];
    char info_bufs[PIMD_INFO_BOX_COUNT][PIMD_INFO_TEXT_LEN];
    u8 selected_idx;
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
} GUI_Pimd_Page;

void gui_pimd_enter(void);
void gui_pimd_exit(void);
bool gui_pimd_is_active(void);

#endif /* _GUI_PIMD_H__ */
