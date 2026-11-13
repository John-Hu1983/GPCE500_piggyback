#include "..\..\..\Include\spinmaster.h"

/*
 *@ brief    : Initialize a GUI_Checkbox (or radio button) widget.
 *@ type     : public
 *@ argu in  : cb: checkbox descriptor; x, y: top-left coordinates; label: caption;
 *             font: GUI_Font_Size selector; checked: initial state.
 *@ argu out : Checkbox structure initialized.
 *@ theme    : Option box / radio selector setup.
 */
void gui_checkbox_init(GUI_Checkbox *cb, s16 x, s16 y, const char *label, GUI_Font_Size font,
                       bool checked)
{
    u16 fh;

    if (cb == null)
    {
        return;
    }
    fh = gui_font_get_char_height(font);
    cb->x = x;
    cb->y = y;
    cb->box_size = (fh >= 14U) ? 14U : 10U;
    cb->checked = checked;
    cb->focused = false;
    cb->style = GUI_Check_Box;
    cb->label = label;
    cb->font = font;
    cb->box_bg_color = GUI_COLOR_BLACK;
    cb->border_color = GUI_COLOR_WHITE;
    cb->check_color = GUI_COLOR_GREEN;
    cb->text_color = GUI_COLOR_WHITE;
    cb->page_bg_color = GUI_COLOR_BLACK;
}

/*
 *@ brief    : Render a GUI_Checkbox or Radio button widget onto the LCD.
 *@ type     : public
 *@ argu in  : cb: pointer to checkbox descriptor.
 *@ argu out : Box/circle indicator, checkmark/dot, and right-side label drawn.
 *@ theme    : Supports square Check_Box with tick mark and circular Check_Radio with inner dot.
 */
void gui_checkbox_draw(const GUI_Checkbox *cb)
{
    u16 border;
    u16 char_h;
    s16 box_y, s;

    if (cb == null || cb->box_size < 6U)
    {
        return;
    }

    char_h = gui_font_get_char_height(cb->font);
    s = (s16)cb->box_size;
    box_y = (char_h > cb->box_size) ? (s16)(cb->y + ((s16)char_h - s) / 2) : cb->y;
    border = cb->focused ? GUI_COLOR_YELLOW : cb->border_color;

    if (cb->style == GUI_Check_Radio)
    {
        s16 r = (s16)(s / 2);
        s16 cx = (s16)(cb->x + r);
        s16 cy = (s16)(box_y + r);
        gui_fill_circle(cx, cy, (u16)r, cb->box_bg_color);
        gui_draw_circle(cx, cy, (u16)r, border);
        if (cb->checked && r > 3)
        {
            gui_fill_circle(cx, cy, (u16)(r - 3), cb->check_color);
        }
    }
    else
    {
        s16 x1 = (s16)(cb->x + s - 1);
        s16 y1 = (s16)(box_y + s - 1);
        gui_fill_rect(cb->x, box_y, x1, y1, cb->box_bg_color);
        gui_draw_rect(cb->x, box_y, x1, y1, border);
        if (cb->checked)
        {
            s16 mx = (s16)(cb->x + s / 2 - 1);
            s16 my = (s16)(box_y + s - 3);
            gui_draw_line_thick((s16)(cb->x + 2), (s16)(box_y + s / 2), mx, my, 2U,
                                cb->check_color);
            gui_draw_line_thick(mx, my, (s16)(x1 - 2), (s16)(box_y + 2), 2U, cb->check_color);
        }
    }

    if (cb->label != null)
    {
        s16 tx = (s16)(cb->x + s + 5);
        s16 ty = (cb->box_size > char_h) ? (s16)(cb->y + (s - (s16)char_h) / 2) : cb->y;
        gui_draw_str_bg(tx, ty, cb->label, cb->text_color, cb->page_bg_color, cb->font);
    }
}

/*
 *@ brief    : Set checkbox state and immediately redraw.
 *@ type     : public
 *@ argu in  : cb: checkbox descriptor; checked: new boolean state.
 *@ argu out : State updated and widget redrawn.
 *@ theme    : Programmatic check state update.
 */
void gui_checkbox_set_checked(GUI_Checkbox *cb, bool checked)
{
    if (cb == null)
    {
        return;
    }
    cb->checked = checked;
    gui_checkbox_draw(cb);
}

/*
 *@ brief    : Toggle checkbox state and immediately redraw.
 *@ type     : public
 *@ argu in  : cb: checkbox descriptor.
 *@ argu out : Checked state inverted and redrawn.
 *@ theme    : User key/click toggle helper.
 */
void gui_checkbox_toggle(GUI_Checkbox *cb)
{
    if (cb == null)
    {
        return;
    }
    cb->checked = !cb->checked;
    gui_checkbox_draw(cb);
}
