#ifndef _GUI_SCOPE_H__
#define _GUI_SCOPE_H__

// clang-format off
#define SCOPE_CHANNELS          3U
#define SCOPE_LUT_SIZE          72U     /* 72 steps = 360 deg (5 deg/step); 24 steps = 120 deg */
#define SCOPE_PHASE_120_STEPS   24U

#define SCOPE_TICK_PERIOD       4U      /* 4 RTC ticks @ 2048Hz (~2ms slice interval) */
#define SCOPE_COLS_PER_SLICE    37U     /* 148 columns / 4 slices = 37 columns (~8ms) per slice */
#define SCOPE_PHASE_STEP        6U      /* Phase steps advanced per full frame (6 * 5 deg = 30 deg) */

#define SCOPE_BG_COLOR          GUI_RGB565(10, 14, 22)
#define SCOPE_HUD_BG            GUI_RGB565(22, 30, 48)
#define SCOPE_BORDER_COLOR      GUI_RGB565(56, 84, 128)
#define SCOPE_AXIS_COLOR        GUI_RGB565(38, 56, 86)
#define SCOPE_GRID_COLOR        GUI_RGB565(28, 42, 66)

#define SCOPE_BOX_X0            5
#define SCOPE_BOX_Y0            16
#define SCOPE_BOX_X1            (s16)(LCD_WIDTH - 6U)
#define SCOPE_BOX_Y1            (s16)(LCD_HEIGHT - 4U)

#define SCOPE_WAVE_X0           (SCOPE_BOX_X0 + 1)
#define SCOPE_WAVE_Y0           (SCOPE_BOX_Y0 + 1)
#define SCOPE_WAVE_W            148U
#define SCOPE_WAVE_H            107U
#define SCOPE_CENTER_X          80
#define SCOPE_CENTER_Y          70

#define SCOPE_AMP_MIN           14
#define SCOPE_AMP_MAX           48
#define SCOPE_AMP_DEFAULT       38
#define SCOPE_AMP_STEP          4

#define SCOPE_FREQ_MIN          1U
#define SCOPE_FREQ_MAX          4U
#define SCOPE_FREQ_DEFAULT      2U
// clang-format on

typedef struct
{
    u8 phase;
    u8 old_phase;
    u8 render_phase;
    u8 freq_step;
    u8 old_freq;
    u8 render_freq;
    u8 col_cursor;
    u8 idx0;
    u8 idx1;
    u8 idx2;
    u8 step_acc;
    u8 old_idx0;
    u8 old_idx1;
    u8 old_idx2;
    u8 old_step_acc;
    s16 amplitude;
    s16 old_amp;
    s16 render_amp;
    s16 old_prev[SCOPE_CHANNELS];
    s16 new_prev[SCOPE_CHANNELS];
    bool paused;
    bool dirty;
    bool initialized;
} GUI_Scope_Page;

void gui_scope_enter(void);
void gui_scope_exit(void);
bool gui_scope_is_active(void);

#endif /* _GUI_SCOPE_H__ */
