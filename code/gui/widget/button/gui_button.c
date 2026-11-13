#include "..\..\..\Include\spinmaster.h"

/*
 *@ brief    : Draw a crisp vector-style PyQt icon inside a circular badge at (cx, cy).
 *@ type     : private
 *@ argu in  : icon: GUI_Btn_Icon type; cx, cy: badge center; badge_bg: circle color; fg: icon color.
 *@ argu out : Vector icon rendered inside circular badge.
 *@ theme    : Modern PyQt / Material flat card icon rendering.
 */
static void _gui_button_draw_icon(GUI_Btn_Icon icon, s16 cx, s16 cy, u16 badge_bg, u16 fg)
{
    /* Outer soft circular badge (r = 10) */
    gui_fill_circle(cx, cy, 10U, badge_bg);

    switch (icon)
    {
    case GUI_Icon_Music:
        /* Dual beamed eighth notes */
        gui_fill_circle((s16)(cx - 4), (s16)(cy + 4), 2U, fg);
        gui_fill_circle((s16)(cx + 3), (s16)(cy + 3), 2U, fg);
        gui_draw_vline((s16)(cx - 2), (s16)(cy - 5), (s16)(cy + 4), fg);
        gui_draw_vline((s16)(cx + 5), (s16)(cy - 6), (s16)(cy + 3), fg);
        gui_draw_line_thick((s16)(cx - 2), (s16)(cy - 5), (s16)(cx + 5), (s16)(cy - 6), 2U, fg);
        break;

    case GUI_Icon_Voice:
        /* Studio microphone capsule + stand */
        gui_fill_round_rect((s16)(cx - 2), (s16)(cy - 6), (s16)(cx + 2), (s16)cy, 2U, fg);
        gui_draw_quadrant_arc(cx, (s16)(cy - 1), 5U,
                              (u8)(GUI_QUAD_BOTTOM_LEFT | GUI_QUAD_BOTTOM_RIGHT), fg);
        gui_draw_vline(cx, (s16)(cy + 4), (s16)(cy + 6), fg);
        gui_draw_hline((s16)(cx - 3), (s16)(cx + 3), (s16)(cy + 6), fg);
        break;

    case GUI_Icon_Volume:
        /* Speaker cone + sound waves */
        gui_fill_rect((s16)(cx - 6), (s16)(cy - 2), (s16)(cx - 4), (s16)(cy + 2), fg);
        gui_fill_triangle((s16)(cx - 4), (s16)cy, (s16)cx, (s16)(cy - 5), (s16)cx, (s16)(cy + 5),
                          fg);
        gui_draw_arc((s16)(cx + 1), cy, 3U, 315U, 45U, fg);
        gui_draw_arc((s16)(cx + 1), cy, 6U, 315U, 45U, fg);
        break;

    case GUI_Icon_Light:
        /* Sun / brightness icon with 8 rays */
        gui_fill_circle(cx, cy, 4U, fg);
        gui_draw_vline(cx, (s16)(cy - 7), (s16)(cy - 6), fg);
        gui_draw_vline(cx, (s16)(cy + 6), (s16)(cy + 7), fg);
        gui_draw_hline((s16)(cx - 7), (s16)(cx - 6), cy, fg);
        gui_draw_hline((s16)(cx + 6), (s16)(cx + 7), cy, fg);
        gui_draw_point((s16)(cx - 5), (s16)(cy - 5), fg);
        gui_draw_point((s16)(cx + 5), (s16)(cy - 5), fg);
        gui_draw_point((s16)(cx - 5), (s16)(cy + 5), fg);
        gui_draw_point((s16)(cx + 5), (s16)(cy + 5), fg);
        break;

    case GUI_Icon_Timer:
        /* Clock face + hour/minute hands */
        gui_draw_circle(cx, cy, 7U, fg);
        gui_draw_vline(cx, (s16)(cy - 4), cy, fg);
        gui_draw_hline(cx, (s16)(cx + 3), cy, fg);
        gui_draw_hline((s16)(cx - 2), (s16)(cx + 2), (s16)(cy - 8), fg);
        break;

    case GUI_Icon_Setup:
        /* Gear cogwheel with center hole */
        gui_fill_rect((s16)(cx - 1), (s16)(cy - 7), (s16)(cx + 1), (s16)(cy + 7), fg);
        gui_fill_rect((s16)(cx - 7), (s16)(cy - 1), (s16)(cx + 7), (s16)(cy + 1), fg);
        gui_draw_line_thick((s16)(cx - 5), (s16)(cy - 5), (s16)(cx + 5), (s16)(cy + 5), 2U, fg);
        gui_draw_line_thick((s16)(cx - 5), (s16)(cy + 5), (s16)(cx + 5), (s16)(cy - 5), 2U, fg);
        gui_fill_circle(cx, cy, 5U, fg);
        gui_fill_circle(cx, cy, 2U, badge_bg);
        break;

    case GUI_Icon_Snake:
        /* S-shaped coiled snake + apple dot */
        gui_fill_rect((s16)(cx - 5), (s16)(cy - 5), (s16)(cx + 2), (s16)(cy - 3), fg);
        gui_fill_rect((s16)(cx - 5), (s16)(cy - 3), (s16)(cx - 3), (s16)(cy + 1), fg);
        gui_fill_rect((s16)(cx - 5), (s16)(cy - 1), (s16)(cx + 4), (s16)(cy + 1), fg);
        gui_fill_rect((s16)(cx + 2), (s16)(cy + 1), (s16)(cx + 4), (s16)(cy + 5), fg);
        gui_fill_rect((s16)(cx - 4), (s16)(cy + 3), (s16)(cx + 4), (s16)(cy + 5), fg);
        gui_fill_circle((s16)(cx + 5), (s16)(cy - 4), 2U, GUI_COLOR_YELLOW);
        break;

    case GUI_Icon_Tetris:
        /* Classic T-tetromino blocks */
        gui_fill_rect((s16)(cx - 6), (s16)(cy - 4), (s16)(cx - 3), (s16)(cy - 1), fg);
        gui_fill_rect((s16)(cx - 1), (s16)(cy - 4), (s16)(cx + 2), (s16)(cy - 1), fg);
        gui_fill_rect((s16)(cx + 4), (s16)(cy - 4), (s16)(cx + 7), (s16)(cy - 1), fg);
        gui_fill_rect((s16)(cx - 1), (s16)(cy + 1), (s16)(cx + 2), (s16)(cy + 4), fg);
        break;

    case GUI_Icon_Brick:
        /* Bricks on top + ball + paddle at bottom */
        gui_fill_rect((s16)(cx - 6), (s16)(cy - 6), (s16)(cx - 1), (s16)(cy - 4), fg);
        gui_fill_rect((s16)(cx + 1), (s16)(cy - 6), (s16)(cx + 6), (s16)(cy - 4), fg);
        gui_fill_circle(cx, (s16)cy, 2U, GUI_COLOR_YELLOW);
        gui_fill_rect((s16)(cx - 5), (s16)(cy + 4), (s16)(cx + 5), (s16)(cy + 6), fg);
        break;

    case GUI_Icon_Flappy:
        /* Bird body + wing + eye + beak */
        gui_fill_circle((s16)(cx - 1), cy, 5U, fg);
        gui_fill_circle((s16)(cx - 3), (s16)(cy + 1), 2U, badge_bg);
        gui_draw_point((s16)(cx + 1), (s16)(cy - 2), badge_bg);
        gui_fill_triangle((s16)(cx + 4), (s16)(cy - 1), (s16)(cx + 7), cy, (s16)(cx + 4),
                          (s16)(cy + 2), GUI_COLOR_YELLOW);
        break;

    case GUI_Icon_Shooter:
        /* Starfighter delta wings + fuselage + laser */
        gui_fill_triangle(cx, (s16)(cy - 6), (s16)(cx - 6), (s16)(cy + 4), (s16)(cx + 6),
                          (s16)(cy + 4), fg);
        gui_fill_rect((s16)(cx - 1), (s16)(cy - 7), (s16)(cx + 1), (s16)(cy + 6), GUI_COLOR_YELLOW);
        break;

    case GUI_Icon_2048:
        /* 2x2 puzzle number tiles */
        gui_fill_rect((s16)(cx - 6), (s16)(cy - 6), (s16)(cx - 1), (s16)(cy - 1), fg);
        gui_fill_rect((s16)(cx + 1), (s16)(cy - 6), (s16)(cx + 6), (s16)(cy - 1), GUI_COLOR_YELLOW);
        gui_fill_rect((s16)(cx - 6), (s16)(cy + 1), (s16)(cx - 1), (s16)(cy + 6), GUI_COLOR_YELLOW);
        gui_fill_rect((s16)(cx + 1), (s16)(cy + 1), (s16)(cx + 6), (s16)(cy + 6), fg);
        break;

    case GUI_Icon_Game:
        /* Handheld gamepad body + D-pad + A/B buttons */
        gui_fill_round_rect((s16)(cx - 7), (s16)(cy - 4), (s16)(cx + 7), (s16)(cy + 4), 3U, fg);
        gui_draw_hline((s16)(cx - 5), (s16)(cx - 1), cy, badge_bg);
        gui_draw_vline((s16)(cx - 3), (s16)(cy - 2), (s16)(cy + 2), badge_bg);
        gui_draw_point((s16)(cx + 4), (s16)(cy - 1), GUI_COLOR_RED);
        gui_draw_point((s16)(cx + 2), (s16)(cy + 1), GUI_COLOR_BLUE);
        break;

    case GUI_Icon_None:
    default:
        break;
    }
}

/*
 *@ brief    : Initialize a GUI_Button widget with modern PyQt-style card colors.
 *@ type     : public
 *@ argu in  : btn: button descriptor; x, y: top-left coordinates; w, h: dimensions;
 *             label: button caption; font: GUI_Font_Size selector.
 *@ argu out : Button structure populated in normal state.
 *@ theme    : Modern PyQt card button setup.
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
    btn->radius = 6U;
    btn->label = label;
    btn->font = font;
    btn->icon = GUI_Icon_None;
    btn->icon_bg_color = GUI_RGB565(56, 132, 255);
    btn->bg_normal = GUI_RGB565(34, 42, 62);
    btn->bg_focused = GUI_RGB565(48, 64, 96);
    btn->bg_pressed = GUI_RGB565(24, 90, 180);
    btn->border_color = GUI_RGB565(68, 84, 118);
    btn->focus_color = GUI_RGB565(0, 216, 255);
    btn->shadow_color = GUI_RGB565(12, 16, 24);
    btn->text_color = GUI_COLOR_WHITE;
    btn->state = GUI_Btn_Normal;
}

/*
 *@ brief    : Render a PyQt-styled GUI_Button widget (3D bevel, drop shadow, icon badge, caption).
 *@ type     : public
 *@ argu in  : btn: pointer to button descriptor.
 *@ argu out : Card body, top specular highlight, drop shadow, icon badge, and caption drawn.
 *@ theme    : Rich PyQt / Material dark card appearance for Normal, Focused, Pressed, Disabled.
 */
void gui_button_draw(const GUI_Button *btn)
{
    s16 x1, y1;
    u16 bg, border, fg, top_shine;

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
        fg = GUI_COLOR_WHITE;
        top_shine = btn->bg_pressed;
        break;
    case GUI_Btn_Focused:
        bg = btn->bg_focused;
        border = btn->focus_color;
        fg = GUI_COLOR_YELLOW;
        top_shine = GUI_RGB565(130, 235, 255);
        break;
    case GUI_Btn_Disabled:
        bg = GUI_RGB565(36, 38, 44);
        border = GUI_RGB565(64, 68, 76);
        fg = GUI_COLOR_GRAY;
        top_shine = GUI_RGB565(50, 54, 62);
        break;
    case GUI_Btn_Normal:
    default:
        bg = btn->bg_normal;
        border = btn->border_color;
        fg = btn->text_color;
        top_shine = GUI_RGB565(92, 114, 156);
        break;
    }

    if (btn->radius > 0U)
    {
        /* 1. Soft bottom drop shadow */
        if (y1 + 1 < (s16)LCD_HEIGHT && btn->w > (u16)(btn->radius * 2U))
        {
            gui_draw_hline((s16)(btn->x + (s16)btn->radius), (s16)(x1 - (s16)btn->radius),
                           (s16)(y1 + 1), btn->shadow_color);
        }

        /* 2. Rounded card body + outer border */
        gui_fill_round_rect(btn->x, btn->y, x1, y1, btn->radius, bg);
        gui_draw_round_rect(btn->x, btn->y, x1, y1, btn->radius, border);

        /* 3. Top inner specular highlight line (3D raised effect) */
        if (btn->w > (u16)(btn->radius * 2U + 2U))
        {
            gui_draw_hline((s16)(btn->x + (s16)btn->radius), (s16)(x1 - (s16)btn->radius),
                           (s16)(btn->y + 1), top_shine);
        }

        /* 4. Glowing inner focus ring when selected */
        if (btn->state == GUI_Btn_Focused && btn->w > 6U && btn->h > 6U && btn->radius > 1U)
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

    /* 5. Optional circular vector icon badge + bottom caption */
    if (btn->icon != GUI_Icon_None && btn->h >= 34U)
    {
        u16 text_h = gui_font_get_char_height(btn->font);
        s16 ty = (s16)(y1 - (s16)text_h - 2);
        s16 icx = (s16)(btn->x + (s16)(btn->w / 2U));
        s16 icy = (s16)(btn->y + (ty - btn->y) / 2 + 1);
        _gui_button_draw_icon(btn->icon, icx, icy, btn->icon_bg_color, GUI_COLOR_WHITE);

        if (btn->label != null)
        {
            u16 text_w = gui_font_get_str_width(btn->label, btn->font);
            s16 tx = (s16)(btn->x + ((s16)btn->w - (s16)text_w) / 2);
            gui_draw_str_at(tx, ty, btn->label, fg, btn->font);
        }
    }
    else if (btn->label != null)
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
