#include "..\..\Include\spinmaster.h"

/*
 *@ brief    : Initialize a GUI_Button widget with default theme colors.
 *@ type     : public
 *@ argu in  : btn: button descriptor; x, y: top-left coordinates; w, h: dimensions;
 *             label: button caption; font: GUI_Font_Size selector.
 *@ argu out : Button structure populated in normal state.
 *@ theme    : Default rounded-rectangle button setup.
 */
void gui_button_init(GUI_Button *btn, s16 x, s16 y, u16 w, u16 h, const char *label,
                     GUI_Font_Size font)
{
    if (btn == null)
    {
        return;
    }
    btn->x = x;
    btn->y = y;
    btn->w = w;
    btn->h = h;
    btn->radius = 4U;
    btn->label = label;
    btn->font = font;
    btn->bg_normal = GUI_COLOR_NAVY;
    btn->bg_pressed = GUI_COLOR_BLUE;
    btn->border_color = GUI_COLOR_WHITE;
    btn->focus_color = GUI_COLOR_YELLOW;
    btn->text_color = GUI_COLOR_WHITE;
    btn->state = GUI_Btn_Normal;
}

/*
 *@ brief    : Render a GUI_Button widget onto the LCD.
 *@ type     : public
 *@ argu in  : btn: pointer to button descriptor.
 *@ argu out : Button body, border, and centered caption drawn.
 *@ theme    : Visual feedback for Normal, Focused, Pressed, and Disabled states.
 */
void gui_button_draw(const GUI_Button *btn)
{
    s16 x1, y1;
    u16 bg, border, fg;

    if (btn == null || btn->w == 0U || btn->h == 0U)
    {
        return;
    }

    x1 = (s16)(btn->x + (s16)btn->w - 1);
    y1 = (s16)(btn->y + (s16)btn->h - 1);

    switch (btn->state)
    {
    case GUI_Btn_Pressed:
        bg = btn->bg_pressed;
        border = btn->focus_color;
        fg = btn->text_color;
        break;
    case GUI_Btn_Focused:
        bg = btn->bg_normal;
        border = btn->focus_color;
        fg = btn->focus_color;
        break;
    case GUI_Btn_Disabled:
        bg = GUI_COLOR_DARKGRAY;
        border = GUI_COLOR_GRAY;
        fg = GUI_COLOR_LIGHTGRAY;
        break;
    case GUI_Btn_Normal:
    default:
        bg = btn->bg_normal;
        border = btn->border_color;
        fg = btn->text_color;
        break;
    }

    if (btn->radius > 0U)
    {
        gui_fill_round_rect(btn->x, btn->y, x1, y1, btn->radius, bg);
        gui_draw_round_rect(btn->x, btn->y, x1, y1, btn->radius, border);
        if (btn->state == GUI_Btn_Focused && btn->w > 4U && btn->h > 4U && btn->radius > 1U)
        {
            gui_draw_round_rect((s16)(btn->x + 1), (s16)(btn->y + 1), (s16)(x1 - 1), (s16)(y1 - 1),
                                (u16)(btn->radius - 1U), border);
        }
    }
    else
    {
        gui_fill_rect(btn->x, btn->y, x1, y1, bg);
        gui_draw_rect(btn->x, btn->y, x1, y1, border);
    }

    if (btn->label != null)
    {
        u16 text_w = gui_font_get_str_width(btn->label, btn->font);
        u16 text_h = gui_font_get_char_height(btn->font);
        s16 tx = (s16)(btn->x + ((s16)btn->w - (s16)text_w) / 2);
        s16 ty = (s16)(btn->y + ((s16)btn->h - (s16)text_h) / 2);
        if (btn->state == GUI_Btn_Pressed)
        {
            tx = (s16)(tx + 1);
            ty = (s16)(ty + 1);
        }
        gui_draw_str_at(tx, ty, btn->label, fg, btn->font);
    }
}

/*
 *@ brief    : Update button state and immediately redraw.
 *@ type     : public
 *@ argu in  : btn: button descriptor; state: new GUI_Btn_State.
 *@ argu out : State updated and widget redrawn.
 *@ theme    : Interactive state transition helper.
 */
void gui_button_set_state(GUI_Button *btn, GUI_Btn_State state)
{
    if (btn == null)
    {
        return;
    }
    btn->state = state;
    gui_button_draw(btn);
}

/*
 *@ brief    : Update button caption text and immediately redraw.
 *@ type     : public
 *@ argu in  : btn: button descriptor; label: new caption string.
 *@ argu out : Label updated and widget redrawn.
 *@ theme    : Dynamic button caption update.
 */
void gui_button_set_label(GUI_Button *btn, const char *label)
{
    if (btn == null)
    {
        return;
    }
    btn->label = label;
    gui_button_draw(btn);
}
