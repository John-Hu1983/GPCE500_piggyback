#include "..\..\..\Include\spinmaster.h"

/*
 *@ brief    : Initialize a GUI_Slider widget with track and movable knob.
 *@ type     : public
 *@ argu in  : slider: slider descriptor; x, y: top-left coordinates; w, h: bounding dimensions;
 *             min_val, max_val: value range; value: initial value.
 *@ argu out : Slider structure initialized.
 *@ theme    : Horizontal slider bar with circular thumb knob.
 */
void gui_slider_init(GUI_Slider *slider, s16 x, s16 y, u16 w, u16 h, s16 min_val, s16 max_val,
                     s16 value)
{
    if (slider == null)
    {
        return;
    }
    if (max_val <= min_val)
    {
        max_val = (s16)(min_val + 100);
    }
    if (value < min_val)
    {
        value = min_val;
    }
    if (value > max_val)
    {
        value = max_val;
    }
    slider->x = x;
    slider->y = y;
    slider->w = w;
    slider->h = (h < 10U) ? 10U : h;
    slider->min_val = min_val;
    slider->max_val = max_val;
    slider->value = value;
    slider->step = 5;
    slider->focused = false;
    slider->page_bg_color = GUI_COLOR_BLACK;
    slider->track_bg_color = GUI_COLOR_DARKGRAY;
    slider->track_fill_color = GUI_COLOR_CYAN;
    slider->knob_color = GUI_COLOR_WHITE;
    slider->border_color = GUI_COLOR_GRAY;
}

/*
 *@ brief    : Render a GUI_Slider widget onto the LCD.
 *@ type     : public
 *@ argu in  : slider: pointer to slider descriptor.
 *@ argu out : Bounding box cleared, track bar drawn, and thumb knob positioned at current value.
 *@ theme    : Flicker-free slider redraw within its bounding rectangle.
 */
void gui_slider_draw(const GUI_Slider *slider)
{
    s16 x1, y1, cy, track_h, ty0, ty1;
    s16 knob_r, kx_min, kx_max, kx;
    s16 span, pos;
    u16 knob_border;

    if (slider == null || slider->w < 12U || slider->h < 8U || slider->max_val <= slider->min_val)
    {
        return;
    }

    x1 = (s16)(slider->x + (s16)slider->w - 1);
    y1 = (s16)(slider->y + (s16)slider->h - 1);

    /* Clear bounding box so previous knob position is erased cleanly */
    gui_fill_rect(slider->x, slider->y, x1, y1, slider->page_bg_color);

    knob_r = (s16)(slider->h / 2U - 1U);
    if (knob_r < 3)
    {
        knob_r = 3;
    }
    cy = (s16)(slider->y + (s16)(slider->h / 2U));
    track_h = (s16)(slider->h / 3U);
    if (track_h < 4)
    {
        track_h = 4;
    }
    ty0 = (s16)(cy - track_h / 2);
    ty1 = (s16)(ty0 + track_h - 1);

    kx_min = (s16)(slider->x + knob_r);
    kx_max = (s16)(x1 - knob_r);
    if (kx_max <= kx_min)
    {
        return;
    }

    span = (s16)(slider->max_val - slider->min_val);
    pos = (s16)(slider->value - slider->min_val);
    if (pos < 0)
    {
        pos = 0;
    }
    if (pos > span)
    {
        pos = span;
    }

    kx = (s16)(kx_min + (s16)(((u32)(u16)(kx_max - kx_min) * (u32)(u16)pos + (u32)(u16)(span / 2)) /
                              (u32)(u16)span));

    /* Draw inactive and active track segments */
    gui_fill_rect(kx_min, ty0, kx_max, ty1, slider->track_bg_color);
    if (kx > kx_min)
    {
        gui_fill_rect(kx_min, ty0, kx, ty1, slider->track_fill_color);
    }
    gui_draw_rect(kx_min, ty0, kx_max, ty1, slider->border_color);

    /* Draw circular thumb knob */
    knob_border = slider->focused ? GUI_COLOR_YELLOW : slider->track_fill_color;
    gui_fill_circle(kx, cy, (u16)knob_r, slider->knob_color);
    gui_draw_circle(kx, cy, (u16)knob_r, knob_border);
}

/*
 *@ brief    : Set slider value, clamp to [min_val, max_val], and redraw.
 *@ type     : public
 *@ argu in  : slider: slider descriptor; value: new value.
 *@ argu out : Value updated and slider redrawn.
 *@ theme    : Direct value update.
 */
void gui_slider_set_value(GUI_Slider *slider, s16 value)
{
    if (slider == null)
    {
        return;
    }
    if (value < slider->min_val)
    {
        value = slider->min_val;
    }
    if (value > slider->max_val)
    {
        value = slider->max_val;
    }
    slider->value = value;
    gui_slider_draw(slider);
}

/*
 *@ brief    : Increment or decrement slider value by dir * step and redraw.
 *@ type     : public
 *@ argu in  : slider: slider descriptor; dir: +1 to increase, -1 to decrease.
 *@ argu out : Value stepped and slider redrawn.
 *@ theme    : Button-driven slider adjustment helper.
 */
void gui_slider_step(GUI_Slider *slider, s16 dir)
{
    if (slider == null)
    {
        return;
    }
    gui_slider_set_value(slider, (s16)(slider->value + dir * slider->step));
}
