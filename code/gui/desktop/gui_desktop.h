#ifndef _GUI_DESKTOP_H__
#define _GUI_DESKTOP_H__

// clang-format off
#define DESKTOP_BTN_ROWS        2U
#define DESKTOP_BTN_COLS        3U
#define DESKTOP_BTN_COUNT       (DESKTOP_BTN_ROWS * DESKTOP_BTN_COLS)

#define DESKTOP_TOP_BAR_H       18U
#define DESKTOP_BOTTOM_BAR_H    18U

/* Middle client area after subtracting top bar and bottom bar */
#define DESKTOP_BODY_Y0         DESKTOP_TOP_BAR_H
#define DESKTOP_BODY_H          (LCD_HEIGHT - DESKTOP_TOP_BAR_H - DESKTOP_BOTTOM_BAR_H)

/* Small, balanced margins and gaps dynamically computed from LCD_WIDTH and LCD_HEIGHT */
#define DESKTOP_MARGIN_X        4U
#define DESKTOP_GAP_X           4U
#define DESKTOP_BTN_W           ((LCD_WIDTH - 2U * DESKTOP_MARGIN_X - (DESKTOP_BTN_COLS - 1U) * DESKTOP_GAP_X) / DESKTOP_BTN_COLS)
#define DESKTOP_GRID_X0         ((LCD_WIDTH - (DESKTOP_BTN_COLS * DESKTOP_BTN_W + (DESKTOP_BTN_COLS - 1U) * DESKTOP_GAP_X)) / 2U)

#define DESKTOP_MARGIN_Y        4U
#define DESKTOP_GAP_Y           4U
#define DESKTOP_BTN_H           ((DESKTOP_BODY_H - 2U * DESKTOP_MARGIN_Y - (DESKTOP_BTN_ROWS - 1U) * DESKTOP_GAP_Y) / DESKTOP_BTN_ROWS)
#define DESKTOP_GRID_Y0         (DESKTOP_BODY_Y0 + (DESKTOP_BODY_H - (DESKTOP_BTN_ROWS * DESKTOP_BTN_H + (DESKTOP_BTN_ROWS - 1U) * DESKTOP_GAP_Y)) / 2U)

#define DESKTOP_BG_COLOR        GUI_RGB565(18, 22, 34)
#define DESKTOP_TOPBAR_BG       GUI_RGB565(26, 34, 54)
#define DESKTOP_BOTBAR_BG       GUI_RGB565(24, 30, 46)
// clang-format on

typedef struct
{
    GUI_Textbox top_bar;
    GUI_Textbox bottom_bar;
    GUI_Button buttons[DESKTOP_BTN_COUNT];
    u8 selected_idx;
} GUI_Desktop_Page;

void gui_desktop_enter(void);
void gui_desktop_exit(void);
bool gui_desktop_is_active(void);
void gui_desktop_select_prev(void);
void gui_desktop_select_next(void);

#endif /* _GUI_DESKTOP_H__ */
