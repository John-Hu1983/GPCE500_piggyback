#include "..\..\Include\spinmaster.h"

/*
 *@ brief    : Initialize a pill-shaped ON/OFF GUI_Switch toggle widget.
 *@ type     : public
 *@ argu in  : sw: switch descriptor; x, y: top-left coordinates; w, h: dimensions; on: initial state.
 *@ argu out : Switch structure initialized.
 *@ theme    : Modern pill toggle switch setup.
 */
void gui_switch_init(GUI_Switch *sw, s16 x, s16 y, u16 w, u16 h, bool on)
{
    if (sw == null)
    {
        return;
    }
    sw->x = x;
    sw->y = y;
    sw->w = (w < 20U) ? 28U : w;
    sw->h = (h < 10U) ? 14U : h;
    sw->on = on;
    sw->focused = false;
    sw->on_color = GUI_COLOR_GREEN;
    sw->off_color = GUI_COLOR_DARKGRAY;
    sw->knob_color = GUI_COLOR_WHITE;
    sw->border_color = GUI_COLOR_WHITE;
}

/*
 *@ brief    : Render a GUI_Switch toggle widget onto the LCD.
 *@ type     : public
 *@ argu in  : sw: pointer to switch descriptor.
 *@ argu out : Pill track and left/right circular thumb drawn.
 *@ theme    : Visual ON (right, green) and OFF (left, gray) switch rendering.
 */
void gui_switch_draw(const GUI_Switch *sw)
{
    s16 x1, y1, r, knob_r, kx, cy;
    u16 bg, border;

    if (sw == null || sw->w < 12U || sw->h < 8U)
    {
        return;
    }

    x1 = (s16)(sw->x + (s16)sw->w - 1);
    y1 = (s16)(sw->y + (s16)sw->h - 1);
    r = (s16)(sw->h / 2U);
    cy = (s16)(sw->y + r);

    bg = sw->on ? sw->on_color : sw->off_color;
    border = sw->focused ? GUI_COLOR_YELLOW : sw->border_color;

    gui_fill_round_rect(sw->x, sw->y, x1, y1, (u16)r, bg);
    gui_draw_round_rect(sw->x, sw->y, x1, y1, (u16)r, border);

    knob_r = (s16)(r - 2);
    if (knob_r < 2)
    {
        knob_r = 2;
    }
    kx = sw->on ? (s16)(x1 - r) : (s16)(sw->x + r);
    gui_fill_circle(kx, cy, (u16)knob_r, sw->knob_color);
}

/*
 *@ brief    : Set switch ON/OFF state and immediately redraw.
 *@ type     : public
 *@ argu in  : sw: switch descriptor; on: true for ON, false for OFF.
 *@ argu out : State updated and switch redrawn.
 *@ theme    : Programmatic toggle switch update.
 */
void gui_switch_set_on(GUI_Switch *sw, bool on)
{
    if (sw == null)
    {
        return;
    }
    sw->on = on;
    gui_switch_draw(sw);
}

/*
 *@ brief    : Invert switch ON/OFF state and immediately redraw.
 *@ type     : public
 *@ argu in  : sw: switch descriptor.
 *@ argu out : State toggled and switch redrawn.
 *@ theme    : User click/key toggle helper.
 */
void gui_switch_toggle(GUI_Switch *sw)
{
    if (sw == null)
    {
        return;
    }
    sw->on = !sw->on;
    gui_switch_draw(sw);
}
