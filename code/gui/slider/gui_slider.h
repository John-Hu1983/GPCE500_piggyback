#ifndef _GUI_SLIDER_H__
#define _GUI_SLIDER_H__

typedef struct
{
    s16 x;
    s16 y;
    u16 w;
    u16 h;
    s16 min_val;
    s16 max_val;
    s16 value;
    s16 step;
    bool focused;
    u16 page_bg_color;
    u16 track_bg_color;
    u16 track_fill_color;
    u16 knob_color;
    u16 border_color;
} GUI_Slider;

void gui_slider_init(GUI_Slider *slider, s16 x, s16 y, u16 w, u16 h, s16 min_val, s16 max_val,
                     s16 value);
void gui_slider_draw(const GUI_Slider *slider);
void gui_slider_set_value(GUI_Slider *slider, s16 value);
void gui_slider_step(GUI_Slider *slider, s16 dir);

#endif /* _GUI_SLIDER_H__ */
