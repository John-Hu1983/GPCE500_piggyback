#ifndef _GUI_SCOPE_H__
#define _GUI_SCOPE_H__

// clang-format off
#define SCOPE_CHANNELS          3U
#define SCOPE_LUT_SIZE          72U     /* 72 steps = 360 deg (5 deg/step); 24 steps = 120 deg */
#define SCOPE_PHASE_120_STEPS   24U

#define SCOPE_TICK_PERIOD       16U     /* 16 RTC ticks @ 2048Hz = 7.8ms */
#define SCOPE_PHASE_STEP        4U      /* Phase steps advanced per frame (4 * 5 deg = 20 deg) */

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
    u8 prev_y[SCOPE_CHANNELS][SCOPE_WAVE_W];
    u8 phase;
    u8 freq_step;
    s16 amplitude;
    bool paused;
    bool initialized;
} GUI_Scope_Page;

void gui_scope_enter(void);
void gui_scope_exit(void);
bool gui_scope_is_active(void);

#endif /* _GUI_SCOPE_H__ */
