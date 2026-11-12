#ifndef _GUI_BAR_H__
#define _GUI_BAR_H__

typedef enum
{
    GUI_Bar_Horizontal = 0,
    GUI_Bar_Vertical = 1
} GUI_Bar_Dir;

typedef struct
{
    s16 x;
    s16 y;
    u16 w;
    u16 h;
    s16 min_val;
    s16 max_val;
    s16 value;
    GUI_Bar_Dir dir;
    u16 fg_color;
    u16 bg_color;
    u16 border_color;
    u16 text_color;
    bool show_percent;
} GUI_Bar;

void gui_bar_init(GUI_Bar *bar, s16 x, s16 y, u16 w, u16 h, s16 min_val, s16 max_val, s16 value);
void gui_bar_draw(const GUI_Bar *bar);
void gui_bar_set_value(GUI_Bar *bar, s16 value);

#endif /* _GUI_BAR_H__ */
