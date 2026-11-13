#include "..\..\..\Include\spinmaster.h"

/*
 *@ brief    : Initialize a GUI_ProgressBar progress/level indicator widget.
 *@ type     : public
 *@ argu in  : bar: progressbar descriptor; x, y: top-left coordinates; w, h: dimensions;
 *             min_val, max_val: value range; value: initial value.
 *@ argu out : ProgressBar initialized in horizontal mode.
 *@ theme    : Progress or level indicator initialization.
 */
void gui_progressbar_init(GUI_ProgressBar *bar, s16 x, s16 y, u16 w, u16 h, s16 min_val,
                          s16 max_val, s16 value)
{
    if (bar == null)
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
    bar->x = x;
    bar->y = y;
    bar->w = w;
    bar->h = h;
    bar->min_val = min_val;
    bar->max_val = max_val;
    bar->value = value;
    bar->dir = GUI_ProgressBar_Horizontal;
    bar->fg_color = GUI_COLOR_GREEN;
    bar->bg_color = GUI_COLOR_DARKGRAY;
    bar->border_color = GUI_COLOR_WHITE;
    bar->text_color = GUI_COLOR_WHITE;
    bar->show_percent = false;
}

/*
 *@ brief    : Render a GUI_ProgressBar widget onto the LCD.
 *@ type     : public
 *@ argu in  : bar: pointer to progressbar descriptor.
 *@ argu out : Border, active fill, background track, and optional percentage text drawn.
 *@ theme    : Supports both horizontal (left-to-right) and vertical (bottom-to-top) bars.
 */
void gui_progressbar_draw(const GUI_ProgressBar *bar)
{
    s16 x1, y1, ix0, iy0, ix1, iy1;
    s16 inner_w, inner_h, span, pos;
    u32 pct;

    if (bar == null || bar->w < 4U || bar->h < 4U || bar->max_val <= bar->min_val)
    {
        return;
    }

    x1 = (s16)(bar->x + (s16)bar->w - 1);
    y1 = (s16)(bar->y + (s16)bar->h - 1);
    gui_draw_rect(bar->x, bar->y, x1, y1, bar->border_color);

    ix0 = (s16)(bar->x + 1);
    iy0 = (s16)(bar->y + 1);
    ix1 = (s16)(x1 - 1);
    iy1 = (s16)(y1 - 1);
    inner_w = (s16)(ix1 - ix0 + 1);
    inner_h = (s16)(iy1 - iy0 + 1);

    span = (s16)(bar->max_val - bar->min_val);
    pos = (s16)(bar->value - bar->min_val);
    if (pos < 0)
    {
        pos = 0;
    }
    if (pos > span)
    {
        pos = span;
    }

    pct = ((u32)(u16)pos * 100UL + (u32)(u16)(span / 2)) / (u32)(u16)span;

    if (bar->dir == GUI_ProgressBar_Vertical)
    {
        s16 fill_h =
            (s16)(((u32)(u16)inner_h * (u32)(u16)pos + (u32)(u16)(span / 2)) / (u32)(u16)span);
        if (fill_h < inner_h)
        {
            gui_fill_rect(ix0, iy0, ix1, (s16)(iy1 - fill_h), bar->bg_color);
        }
        if (fill_h > 0)
        {
            gui_fill_rect(ix0, (s16)(iy1 - fill_h + 1), ix1, iy1, bar->fg_color);
        }
    }
    else
    {
        s16 fill_w =
            (s16)(((u32)(u16)inner_w * (u32)(u16)pos + (u32)(u16)(span / 2)) / (u32)(u16)span);
        if (fill_w > 0)
        {
            gui_fill_rect(ix0, iy0, (s16)(ix0 + fill_w - 1), iy1, bar->fg_color);
        }
        if (fill_w < inner_w)
        {
            gui_fill_rect((s16)(ix0 + fill_w), iy0, ix1, iy1, bar->bg_color);
        }
    }

    if (bar->show_percent && bar->h >= 10U)
    {
        char buf[6];
        u8 idx = 5U;
        u32 v = pct;
        u16 tw;
        s16 tx, ty;

        buf[idx] = '\0';
        buf[--idx] = '%';
        do
        {
            buf[--idx] = (char)('0' + (v % 10UL));
            v /= 10UL;
        } while (v != 0UL && idx > 0U);

        tw = gui_font_get_str_width(&buf[idx], GUI_Font_6x8);
        tx = (s16)(bar->x + ((s16)bar->w - (s16)tw) / 2);
        ty = (s16)(bar->y + ((s16)bar->h - 8) / 2);
        gui_draw_str_at(tx, ty, &buf[idx], bar->text_color, GUI_Font_6x8);
    }
}

/*
 *@ brief    : Update progressbar value, clamp to [min_val, max_val], and redraw.
 *@ type     : public
 *@ argu in  : bar: progressbar descriptor; value: new value.
 *@ argu out : Value updated and progressbar redrawn.
 *@ theme    : Dynamic progress/level update.
 */
void gui_progressbar_set_value(GUI_ProgressBar *bar, s16 value)
{
    if (bar == null)
    {
        return;
    }
    if (value < bar->min_val)
    {
        value = bar->min_val;
    }
    if (value > bar->max_val)
    {
        value = bar->max_val;
    }
    bar->value = value;
    gui_progressbar_draw(bar);
}
