#ifndef _GUI_PROGRESSBAR_H__
#define _GUI_PROGRESSBAR_H__

typedef enum
{
    GUI_ProgressBar_Horizontal = 0,
    GUI_ProgressBar_Vertical = 1
} GUI_ProgressBar_Dir;

typedef struct
{
    s16 x;
    s16 y;
    u16 w;
    u16 h;
    s16 min_val;
    s16 max_val;
    s16 value;
    GUI_ProgressBar_Dir dir;
    u16 fg_color;
    u16 bg_color;
    u16 border_color;
    u16 text_color;
    bool show_percent;
} GUI_ProgressBar;

void gui_progressbar_init(GUI_ProgressBar *bar, s16 x, s16 y, u16 w, u16 h, s16 min_val,
                          s16 max_val, s16 value);
void gui_progressbar_draw(const GUI_ProgressBar *bar);
void gui_progressbar_set_value(GUI_ProgressBar *bar, s16 value);

#endif /* _GUI_PROGRESSBAR_H__ */
