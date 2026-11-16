#ifndef _GUI_PIMD_H__
#define _GUI_PIMD_H__

// clang-format off
/*
 * PIMD (Pulse Induction Metal Detector) 100ms pulse timing on IOB1 (PIMD_EN)
 * at RTC_Freq_2K (2048Hz):
 * - First 10ms HIGH : 20 ticks  (~9.77ms)
 * - Next  90ms LOW  : 185 ticks (~90.33ms)
 * - Total period    : 205 ticks (~100.10ms)
 */
#define PIMD_HIGH_TICKS         20U
#define PIMD_LOW_TICKS          185U

#define PIMD_BG_COLOR           GUI_RGB565(18, 22, 34)
#define PIMD_PANEL_BG           GUI_RGB565(26, 34, 52)
#define PIMD_PANEL_BORDER       GUI_RGB565(56, 80, 120)

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

typedef struct
{
    GUI_Button buttons[PIMD_BTN_COUNT];
    u8 selected_idx;
    bool running;
    bool pin_high;
} GUI_Pimd_Page;

void gui_pimd_enter(void);
void gui_pimd_exit(void);
bool gui_pimd_is_active(void);

#endif /* _GUI_PIMD_H__ */
