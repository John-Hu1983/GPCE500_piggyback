#ifndef _GUI_ICON_H__
#define _GUI_ICON_H__

// clang-format off
#define GUI_ICON_BADGE_RADIUS   10U
// clang-format on

typedef enum
{
    GUI_Icon_None = 0,
    GUI_Icon_Music = 1,
    GUI_Icon_Voice = 2,
    GUI_Icon_Volume = 3,
    GUI_Icon_Light = 4,
    GUI_Icon_Timer = 5,
    GUI_Icon_Setup = 6,
    GUI_Icon_Snake = 7,
    GUI_Icon_Tetris = 8,
    GUI_Icon_Brick = 9,
    GUI_Icon_Flappy = 10,
    GUI_Icon_Shooter = 11,
    GUI_Icon_2048 = 12,
    GUI_Icon_Game = 13
} GUI_Icon;

typedef GUI_Icon GUI_Btn_Icon;

void gui_icon_draw(GUI_Icon icon, s16 cx, s16 cy, u16 bg_color, u16 fg_color);
void gui_icon_draw_badge(GUI_Icon icon, s16 cx, s16 cy, u16 badge_bg, u16 fg_color);
void gui_icon_draw_battery(s16 x, s16 cy, u8 percent, u16 border_color, u16 fill_color);

#endif /* _GUI_ICON_H__ */
