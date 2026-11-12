#ifndef _GUI_SWITCH_H__
#define _GUI_SWITCH_H__

typedef struct
{
    s16 x;
    s16 y;
    u16 w;
    u16 h;
    bool on;
    bool focused;
    u16 on_color;
    u16 off_color;
    u16 knob_color;
    u16 border_color;
} GUI_Switch;

void gui_switch_init(GUI_Switch *sw, s16 x, s16 y, u16 w, u16 h, bool on);
void gui_switch_draw(const GUI_Switch *sw);
void gui_switch_set_on(GUI_Switch *sw, bool on);
void gui_switch_toggle(GUI_Switch *sw);

#endif /* _GUI_SWITCH_H__ */
