#include "..\..\Include\spinmaster.h"

/*
 *@ brief    : Draw a standalone vector icon centered at (cx, cy).
 *@ type     : public
 *@ argu in  : icon: GUI_Icon selector; cx, cy: icon center coordinates;
 *             bg_color: background cutout color; fg_color: primary icon foreground color.
 *@ argu out : Vector icon rendered on LCD using 2D geometry primitives.
 *@ theme    : Decoupled zero-ROM-bitmap vector icon renderer.
 */
void gui_icon_draw(GUI_Icon icon, s16 cx, s16 cy, u16 bg_color, u16 fg_color)
{
    switch (icon)
    {
    case GUI_Icon_Music:
        /* Dual beamed eighth notes */
        gui_fill_circle((s16)(cx - 4), (s16)(cy + 4), 2U, fg_color);
        gui_fill_circle((s16)(cx + 3), (s16)(cy + 3), 2U, fg_color);
        gui_draw_vline((s16)(cx - 2), (s16)(cy - 5), (s16)(cy + 4), fg_color);
        gui_draw_vline((s16)(cx + 5), (s16)(cy - 6), (s16)(cy + 3), fg_color);
        gui_draw_line_thick((s16)(cx - 2), (s16)(cy - 5), (s16)(cx + 5), (s16)(cy - 6), 2U,
                            fg_color);
        break;

    case GUI_Icon_Voice:
        /* Studio microphone capsule + U-stand */
        gui_fill_round_rect((s16)(cx - 2), (s16)(cy - 6), (s16)(cx + 2), (s16)cy, 2U, fg_color);
        gui_draw_quadrant_arc(cx, (s16)(cy - 1), 5U,
                              (u8)(GUI_QUAD_BOTTOM_LEFT | GUI_QUAD_BOTTOM_RIGHT), fg_color);
        gui_draw_vline(cx, (s16)(cy + 4), (s16)(cy + 6), fg_color);
        gui_draw_hline((s16)(cx - 3), (s16)(cx + 3), (s16)(cy + 6), fg_color);
        break;

    case GUI_Icon_Volume:
        /* Speaker cone + dual sound wave arcs */
        gui_fill_rect((s16)(cx - 6), (s16)(cy - 2), (s16)(cx - 4), (s16)(cy + 2), fg_color);
        gui_fill_triangle((s16)(cx - 4), (s16)cy, (s16)cx, (s16)(cy - 5), (s16)cx, (s16)(cy + 5),
                          fg_color);
        gui_draw_arc((s16)(cx + 1), cy, 3U, 315U, 45U, fg_color);
        gui_draw_arc((s16)(cx + 1), cy, 6U, 315U, 45U, fg_color);
        break;

    case GUI_Icon_Light:
        /* Sun core + 8 surrounding brightness rays */
        gui_fill_circle(cx, cy, 4U, fg_color);
        gui_draw_vline(cx, (s16)(cy - 7), (s16)(cy - 6), fg_color);
        gui_draw_vline(cx, (s16)(cy + 6), (s16)(cy + 7), fg_color);
        gui_draw_hline((s16)(cx - 7), (s16)(cx - 6), cy, fg_color);
        gui_draw_hline((s16)(cx + 6), (s16)(cx + 7), cy, fg_color);
        gui_draw_point((s16)(cx - 5), (s16)(cy - 5), fg_color);
        gui_draw_point((s16)(cx + 5), (s16)(cy - 5), fg_color);
        gui_draw_point((s16)(cx - 5), (s16)(cy + 5), fg_color);
        gui_draw_point((s16)(cx + 5), (s16)(cy + 5), fg_color);
        break;

    case GUI_Icon_Timer:
        /* Stopwatch dial + top button + hands */
        gui_draw_circle(cx, cy, 7U, fg_color);
        gui_draw_vline(cx, (s16)(cy - 4), cy, fg_color);
        gui_draw_hline(cx, (s16)(cx + 3), cy, fg_color);
        gui_draw_hline((s16)(cx - 2), (s16)(cx + 2), (s16)(cy - 8), fg_color);
        break;

    case GUI_Icon_Setup:
        /* 8-tooth gear cogwheel with center cutout */
        gui_fill_rect((s16)(cx - 1), (s16)(cy - 7), (s16)(cx + 1), (s16)(cy + 7), fg_color);
        gui_fill_rect((s16)(cx - 7), (s16)(cy - 1), (s16)(cx + 7), (s16)(cy + 1), fg_color);
        gui_draw_line_thick((s16)(cx - 5), (s16)(cy - 5), (s16)(cx + 5), (s16)(cy + 5), 2U,
                            fg_color);
        gui_draw_line_thick((s16)(cx - 5), (s16)(cy + 5), (s16)(cx + 5), (s16)(cy - 5), 2U,
                            fg_color);
        gui_fill_circle(cx, cy, 5U, fg_color);
        gui_fill_circle(cx, cy, 2U, bg_color);
        break;

    case GUI_Icon_Snake:
        /* S-shaped coiled snake + apple dot */
        gui_fill_rect((s16)(cx - 5), (s16)(cy - 5), (s16)(cx + 2), (s16)(cy - 3), fg_color);
        gui_fill_rect((s16)(cx - 5), (s16)(cy - 3), (s16)(cx - 3), (s16)(cy + 1), fg_color);
        gui_fill_rect((s16)(cx - 5), (s16)(cy - 1), (s16)(cx + 4), (s16)(cy + 1), fg_color);
        gui_fill_rect((s16)(cx + 2), (s16)(cy + 1), (s16)(cx + 4), (s16)(cy + 5), fg_color);
        gui_fill_rect((s16)(cx - 4), (s16)(cy + 3), (s16)(cx + 4), (s16)(cy + 5), fg_color);
        gui_fill_circle((s16)(cx + 5), (s16)(cy - 4), 2U, GUI_COLOR_YELLOW);
        break;

    case GUI_Icon_Tetris:
        /* Classic T-tetromino blocks */
        gui_fill_rect((s16)(cx - 6), (s16)(cy - 4), (s16)(cx - 3), (s16)(cy - 1), fg_color);
        gui_fill_rect((s16)(cx - 1), (s16)(cy - 4), (s16)(cx + 2), (s16)(cy - 1), fg_color);
        gui_fill_rect((s16)(cx + 4), (s16)(cy - 4), (s16)(cx + 7), (s16)(cy - 1), fg_color);
        gui_fill_rect((s16)(cx - 1), (s16)(cy + 1), (s16)(cx + 2), (s16)(cy + 4), fg_color);
        break;

    case GUI_Icon_Brick:
        /* Top bricks + bouncing ball + bottom paddle */
        gui_fill_rect((s16)(cx - 6), (s16)(cy - 6), (s16)(cx - 1), (s16)(cy - 4), fg_color);
        gui_fill_rect((s16)(cx + 1), (s16)(cy - 6), (s16)(cx + 6), (s16)(cy - 4), fg_color);
        gui_fill_circle(cx, (s16)cy, 2U, GUI_COLOR_YELLOW);
        gui_fill_rect((s16)(cx - 5), (s16)(cy + 4), (s16)(cx + 5), (s16)(cy + 6), fg_color);
        break;

    case GUI_Icon_Flappy:
        /* Bird body + wing cutout + eye + beak */
        gui_fill_circle((s16)(cx - 1), cy, 5U, fg_color);
        gui_fill_circle((s16)(cx - 3), (s16)(cy + 1), 2U, bg_color);
        gui_draw_point((s16)(cx + 1), (s16)(cy - 2), bg_color);
        gui_fill_triangle((s16)(cx + 4), (s16)(cy - 1), (s16)(cx + 7), cy, (s16)(cx + 4),
                          (s16)(cy + 2), GUI_COLOR_YELLOW);
        break;

    case GUI_Icon_Shooter:
        /* Starfighter delta wings + fuselage + laser */
        gui_fill_triangle(cx, (s16)(cy - 6), (s16)(cx - 6), (s16)(cy + 4), (s16)(cx + 6),
                          (s16)(cy + 4), fg_color);
        gui_fill_rect((s16)(cx - 1), (s16)(cy - 7), (s16)(cx + 1), (s16)(cy + 6),
                      GUI_COLOR_YELLOW);
        break;

    case GUI_Icon_2048:
        /* 2x2 puzzle number tiles */
        gui_fill_rect((s16)(cx - 6), (s16)(cy - 6), (s16)(cx - 1), (s16)(cy - 1), fg_color);
        gui_fill_rect((s16)(cx + 1), (s16)(cy - 6), (s16)(cx + 6), (s16)(cy - 1),
                      GUI_COLOR_YELLOW);
        gui_fill_rect((s16)(cx - 6), (s16)(cy + 1), (s16)(cx - 1), (s16)(cy + 6),
                      GUI_COLOR_YELLOW);
        gui_fill_rect((s16)(cx + 1), (s16)(cy + 1), (s16)(cx + 6), (s16)(cy + 6), fg_color);
        break;

    case GUI_Icon_Game:
        /* Handheld gamepad body + D-pad + A/B buttons */
        gui_fill_round_rect((s16)(cx - 7), (s16)(cy - 4), (s16)(cx + 7), (s16)(cy + 4), 3U,
                            fg_color);
        gui_draw_hline((s16)(cx - 5), (s16)(cx - 1), cy, bg_color);
        gui_draw_vline((s16)(cx - 3), (s16)(cy - 2), (s16)(cy + 2), bg_color);
        gui_draw_point((s16)(cx + 4), (s16)(cy - 1), GUI_COLOR_RED);
        gui_draw_point((s16)(cx + 2), (s16)(cy + 1), GUI_COLOR_BLUE);
        break;

    case GUI_Icon_None:
    default:
        break;
    }
}

/*
 *@ brief    : Draw a vector icon inside a colored circular badge at (cx, cy).
 *@ type     : public
 *@ argu in  : icon: GUI_Icon selector; cx, cy: badge center;
 *             badge_bg: circular badge fill color; fg_color: icon foreground color.
 *@ argu out : Circular badge and inner vector icon rendered.
 *@ theme    : PyQt / Material circular icon badge for cards and buttons.
 */
void gui_icon_draw_badge(GUI_Icon icon, s16 cx, s16 cy, u16 badge_bg, u16 fg_color)
{
    gui_fill_circle(cx, cy, GUI_ICON_BADGE_RADIUS, badge_bg);
    gui_icon_draw(icon, cx, cy, badge_bg, fg_color);
}

/*
 *@ brief    : Draw a status-bar battery icon with proportional charge fill.
 *@ type     : public
 *@ argu in  : x: left coordinate; cy: vertical center; percent: 0..100 charge level;
 *             border_color: battery shell color; fill_color: charge bar color.
 *@ argu out : Battery outline, positive terminal cap, and inner level bar rendered.
 *@ theme    : Reusable status-bar battery indicator.
 */
void gui_icon_draw_battery(s16 x, s16 cy, u8 percent, u16 border_color, u16 fill_color)
{
    s16 fill_w;

    if (percent > 100U)
    {
        percent = 100U;
    }
    gui_draw_rect(x, (s16)(cy - 4), (s16)(x + 14), (s16)(cy + 3), border_color);
    gui_fill_rect((s16)(x + 15), (s16)(cy - 2), (s16)(x + 16), (s16)(cy + 1), border_color);

    fill_w = (s16)(((u16)percent * 10U + 50U) / 100U);
    if (fill_w > 0)
    {
        gui_fill_rect((s16)(x + 2), (s16)(cy - 2), (s16)(x + 1 + fill_w), (s16)(cy + 1),
                      fill_color);
    }
}
