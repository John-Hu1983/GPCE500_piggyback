#ifndef _GUI_SETTING_H__
#define _GUI_SETTING_H__

// clang-format off
#define SETTING_BG_COLOR            GUI_RGB565(18, 22, 34)
#define SETTING_CARD_BG_NORMAL      GUI_RGB565(26, 34, 52)
#define SETTING_CARD_BG_FOCUS       GUI_RGB565(34, 48, 78)
#define SETTING_CARD_BORDER_NORMAL  GUI_RGB565(56, 76, 112)
#define SETTING_CARD_BORDER_FOCUS   GUI_RGB565(255, 210, 0)

#define SETTING_CARD_X              8
#define SETTING_CARD_W              (LCD_WIDTH - 16U)
#define SETTING_CARD_H              52U
#define SETTING_CARD_RADIUS         6U

#define SETTING_BL_CARD_Y           8
#define SETTING_VOL_CARD_Y          68

#define SETTING_SLIDER_X            18
#define SETTING_SLIDER_W            (LCD_WIDTH - 36U)
#define SETTING_SLIDER_H            16U

#define SETTING_BL_MIN              5
#define SETTING_BL_MAX              100
#define SETTING_BL_STEP             5

#define SETTING_VOL_MIN             0
#define SETTING_VOL_MAX             15
#define SETTING_VOL_STEP            1
// clang-format on

typedef enum
{
    GUI_Setting_Brightness = 0,
    GUI_Setting_Volume = 1,
    GUI_Setting_Count = 2
} GUI_Setting_Item;

typedef struct
{
    GUI_Slider bl_slider;
    GUI_Slider vol_slider;
    u8 selected_idx;
} GUI_Setting_Page;

void gui_setting_enter(void);
void gui_setting_enter_item(GUI_Setting_Item item);
void gui_setting_exit(void);
bool gui_setting_is_active(void);
u16 gui_setting_get_brightness(void);
u8 gui_setting_get_volume(void);

#endif /* _GUI_SETTING_H__ */
