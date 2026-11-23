#ifndef _GUI_PIMD_H__
#define _GUI_PIMD_H__

// clang-format off
#define PIMD_CYCLE_TICKS        110U
#define PIMD_BEEP_ON_TICKS      24U
#define PIMD_BEEP_QUIET_TICKS   (PIMD_CYCLE_TICKS - PIMD_BEEP_ON_TICKS)

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

typedef struct
{
    GUI_Button buttons[PIMD_BTN_COUNT];
    GUI_Textbox status_box;
    GUI_Textbox info_boxes[PIMD_INFO_BOX_COUNT];
    char info_bufs[PIMD_INFO_BOX_COUNT][PIMD_INFO_TEXT_LEN];
    u8 selected_idx;
    PIMD_Phy_State phy;
} GUI_Pimd_Page;

void gui_pimd_enter(void);
void gui_pimd_exit(void);
bool gui_pimd_is_active(void);

#endif /* _GUI_PIMD_H__ */
