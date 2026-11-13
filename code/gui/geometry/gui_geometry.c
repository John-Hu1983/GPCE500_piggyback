#include "..\..\Include\spinmaster.h"

/*
 * Quarter-wave sine table for 0..90 degrees scaled by 256 (0 -> 0, 90 -> 256).
 * Used for integer-only degree-angle checks in gui_draw_arc / gui_fill_sector.
 */
static const u16 gui_sin_q8[91] = {
    0U,   4U,   9U,   13U,  18U,  22U,  27U,  31U,  36U,  40U,  44U,  49U,  53U,  58U,  62U,  66U,
    71U,  75U,  79U,  83U,  88U,  92U,  96U,  100U, 104U, 108U, 112U, 116U, 120U, 124U, 128U, 132U,
    136U, 139U, 143U, 147U, 150U, 154U, 158U, 161U, 165U, 168U, 171U, 175U, 178U, 181U, 184U, 187U,
    190U, 193U, 196U, 199U, 202U, 204U, 207U, 210U, 212U, 215U, 217U, 219U, 222U, 224U, 226U, 228U,
    230U, 232U, 234U, 236U, 237U, 239U, 241U, 242U, 243U, 245U, 246U, 247U, 248U, 249U, 250U, 251U,
    252U, 253U, 254U, 254U, 255U, 255U, 255U, 256U, 256U, 256U, 256U};

/*
 *@ brief    : Swap two signed 16-bit integers.
 *@ type     : private
 *@ argu in  : a, b: pointers to values to swap.
 *@ argu out : Values exchanged.
 *@ theme    : Coordinate normalization helper.
 */
static void _gui_swap_s16(s16 *a, s16 *b)
{
    s16 tmp = *a;
    *a = *b;
    *b = tmp;
}

/*
 *@ brief    : Evaluate sin(deg) scaled by 256 for any angle in degrees.
 *@ type     : private
 *@ argu in  : deg: angle in degrees.
 *@ argu out : Signed value in [-256, 256].
 *@ theme    : Table lookup without floating-point math.
 */
static s16 _gui_sin_deg(u16 deg)
{
    u16 a = deg % 360U;

    if (a <= 90U)
    {
        return (s16)gui_sin_q8[a];
    }
    if (a <= 180U)
    {
        return (s16)gui_sin_q8[180U - a];
    }
    if (a <= 270U)
    {
        return -(s16)gui_sin_q8[a - 180U];
    }
    return -(s16)gui_sin_q8[360U - a];
}

/*
 *@ brief    : Evaluate cos(deg) scaled by 256 for any angle in degrees.
 *@ type     : private
 *@ argu in  : deg: angle in degrees.
 *@ argu out : Signed value in [-256, 256].
 *@ theme    : cos(deg) = sin(deg + 90).
 */
static s16 _gui_cos_deg(u16 deg) { return _gui_sin_deg((u16)(deg + 90U)); }

/*
 *@ brief    : Test whether vector (dx, dy) in screen coordinates lies within [start_deg, end_deg].
 *@ type     : private
 *@ argu in  : dx: x - xc; dy: yc - y (math up-positive); start_deg, end_deg: counter-clockwise
 *deg.
 *@ argu out : true if point lies inside the angular sweep.
 *@ theme    : Half-plane cross-product check using integer sine/cosine.
 */
static bool _gui_point_in_angle(s16 dx, s16 dy, u16 start_deg, u16 end_deg)
{
    u16 s = start_deg % 360U;
    u16 e = end_deg % 360U;
    u16 span;
    s16 sx, sy, ex, ey;
    s32 cross_s, cross_e;

    if (dx == 0 && dy == 0)
    {
        return true;
    }
    if (start_deg != end_deg && s == e)
    {
        return true;
    }

    span = (e >= s) ? (e - s) : (u16)(360U - s + e);
    sx = _gui_cos_deg(s);
    sy = _gui_sin_deg(s);
    ex = _gui_cos_deg(e);
    ey = _gui_sin_deg(e);

    /* Cross product: start_vec x P = sx*dy - sy*dx >= 0 means counter-clockwise from start_deg. */
    cross_s = (s32)sx * (s32)dy - (s32)sy * (s32)dx;
    /* Cross product: P x end_vec = dx*ey - dy*ex >= 0 means clockwise from end_deg. */
    cross_e = (s32)dx * (s32)ey - (s32)dy * (s32)ex;

    if (span <= 180U)
    {
        return (cross_s >= 0L) && (cross_e >= 0L);
    }
    return (cross_s >= 0L) || (cross_e >= 0L);
}

/*
 *@ brief    : Convert 8-bit R, G, B (0..255) components into 16-bit RGB565 color.
 *@ type     : public
 *@ argu in  : r, g, b: 8-bit color channels.
 *@ argu out : 16-bit RGB565 value.
 *@ theme    : Bit-pack 5-6-5 color channels.
 */
u16 gui_rgb565(u8 r, u8 g, u8 b) { return GUI_RGB565(r, g, b); }

/*
 *@ brief    : Clear the entire screen with a solid RGB565 color.
 *@ type     : public
 *@ argu in  : color: 16-bit RGB565 color.
 *@ argu out : Entire screen filled.
 *@ theme    : Delegate directly to lcd_fill_color.
 */
void gui_clear_screen(u16 color) { lcd_fill_color(color); }

/*
 *@ brief    : Draw one clipped pixel at signed coordinates (x, y).
 *@ type     : public
 *@ argu in  : x, y: pixel coordinates; color: RGB565 color.
 *@ argu out : Pixel written if within [0..LCD_WIDTH-1, 0..LCD_HEIGHT-1].
 *@ theme    : Safe clipping allows shapes partially off-screen.
 */
void gui_draw_point(s16 x, s16 y, u16 color)
{
    if (x < 0 || y < 0 || (u16)x >= LCD_WIDTH || (u16)y >= LCD_HEIGHT)
    {
        return;
    }
    lcd_set_window((u16)x, (u16)y, (u16)x, (u16)y);
    lcd_get_device()->write_data_u16(color);
}

/*
 *@ brief    : Legacy 8-bit coordinate pixel drawing wrapper.
 *@ type     : public
 *@ argu in  : x, y: pixel coordinates; color: RGB565 color.
 *@ argu out : Pixel drawn.
 *@ theme    : Preserve compatibility with gui_draw_pixel(u8, u8, u16).
 */
void gui_draw_pixel(u8 x, u8 y, u16 color) { gui_draw_point((s16)x, (s16)y, color); }

/*
 *@ brief    : Draw a thick point (filled disc of radius r) at (x, y).
 *@ type     : public
 *@ argu in  : x, y: center coordinates; radius: pixel radius (0 = 1px); color: RGB565 color.
 *@ argu out : Point or disc drawn.
 *@ theme    : Building block for thick lines and markers.
 */
void gui_draw_point_thick(s16 x, s16 y, u8 radius, u16 color)
{
    if (radius == 0U)
    {
        gui_draw_point(x, y, color);
    }
    else
    {
        gui_fill_circle(x, y, radius, color);
    }
}

/*
 *@ brief    : Draw a clipped horizontal line from x0 to x1 at row y.
 *@ type     : public
 *@ argu in  : x0, x1: horizontal endpoints; y: row coordinate; color: RGB565 color.
 *@ argu out : Horizontal span filled via hardware window.
 *@ theme    : Fast span primitive for filled shapes.
 */
void gui_draw_hline(s16 x0, s16 x1, s16 y, u16 color)
{
    if (y < 0 || (u16)y >= LCD_HEIGHT)
    {
        return;
    }
    if (x0 > x1)
    {
        _gui_swap_s16(&x0, &x1);
    }
    if (x1 < 0 || x0 >= (s16)LCD_WIDTH)
    {
        return;
    }
    if (x0 < 0)
    {
        x0 = 0;
    }
    if (x1 >= (s16)LCD_WIDTH)
    {
        x1 = (s16)(LCD_WIDTH - 1U);
    }
    lcd_fill_rect((u16)x0, (u16)y, (u16)(x1 - x0 + 1), 1U, color);
}

/*
 *@ brief    : Draw a clipped vertical line from y0 to y1 at column x.
 *@ type     : public
 *@ argu in  : x: column coordinate; y0, y1: vertical endpoints; color: RGB565 color.
 *@ argu out : Vertical span filled via hardware window.
 *@ theme    : Fast vertical span primitive.
 */
void gui_draw_vline(s16 x, s16 y0, s16 y1, u16 color)
{
    if (x < 0 || (u16)x >= LCD_WIDTH)
    {
        return;
    }
    if (y0 > y1)
    {
        _gui_swap_s16(&y0, &y1);
    }
    if (y1 < 0 || y0 >= (s16)LCD_HEIGHT)
    {
        return;
    }
    if (y0 < 0)
    {
        y0 = 0;
    }
    if (y1 >= (s16)LCD_HEIGHT)
    {
        y1 = (s16)(LCD_HEIGHT - 1U);
    }
    lcd_fill_rect((u16)x, (u16)y0, 1U, (u16)(y1 - y0 + 1), color);
}

/*
 *@ brief    : Draw a line between (x0, y0) and (x1, y1) using Bresenham's algorithm.
 *@ type     : public
 *@ argu in  : x0, y0: start point; x1, y1: end point; color: RGB565 color.
 *@ argu out : Line drawn; axis-aligned lines use fast hardware spans.
 *@ theme    : Integer Bresenham line rasterizer.
 */
void gui_draw_line(s16 x0, s16 y0, s16 x1, s16 y1, u16 color)
{
    s16 dx, dy, sx, sy, err, e2;

    if (y0 == y1)
    {
        gui_draw_hline(x0, x1, y0, color);
        return;
    }
    if (x0 == x1)
    {
        gui_draw_vline(x0, y0, y1, color);
        return;
    }

    dx = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
    dy = (y1 >= y0) ? (y0 - y1) : (y1 - y0);
    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    err = dx + dy;

    for (;;)
    {
        gui_draw_point(x0, y0, color);
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        e2 = err << 1;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

/*
 *@ brief    : Draw a line with configurable pixel thickness.
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1: endpoints; thickness: width in pixels (>=1); color: RGB565 color.
 *@ argu out : Thick line drawn.
 *@ theme    : Stamp filled discs along the Bresenham path.
 */
void gui_draw_line_thick(s16 x0, s16 y0, s16 x1, s16 y1, u8 thickness, u16 color)
{
    s16 dx, dy, sx, sy, err, e2;
    u8 r;

    if (thickness <= 1U)
    {
        gui_draw_line(x0, y0, x1, y1, color);
        return;
    }

    r = (u8)(thickness >> 1);
    dx = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
    dy = (y1 >= y0) ? (y0 - y1) : (y1 - y0);
    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    err = dx + dy;

    for (;;)
    {
        gui_fill_circle(x0, y0, r, color);
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        e2 = err << 1;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

/*
 *@ brief    : Draw a dashed/dotted line between (x0, y0) and (x1, y1).
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1: endpoints; dash_len: on pixels; gap_len: off pixels; color: RGB565.
 *@ argu out : Dashed line drawn.
 *@ theme    : Step counter modulo (dash_len + gap_len) along Bresenham path.
 */
void gui_draw_dashed_line(s16 x0, s16 y0, s16 x1, s16 y1, u8 dash_len, u8 gap_len, u16 color)
{
    s16 dx, dy, sx, sy, err, e2;
    u16 period;
    u16 step = 0U;

    if (dash_len == 0U)
    {
        return;
    }
    if (gap_len == 0U)
    {
        gui_draw_line(x0, y0, x1, y1, color);
        return;
    }

    period = (u16)dash_len + (u16)gap_len;
    dx = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
    dy = (y1 >= y0) ? (y0 - y1) : (y1 - y0);
    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    err = dx + dy;

    for (;;)
    {
        if ((step % period) < dash_len)
        {
            gui_draw_point(x0, y0, color);
        }
        ++step;
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        e2 = err << 1;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

/*
 *@ brief    : Draw a hollow 1px-border rectangle between (x0, y0) and (x1, y1).
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1: corner coordinates; color: RGB565 color.
 *@ argu out : Hollow rectangle drawn.
 *@ theme    : Four fast hardware-span edges.
 */
void gui_draw_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 color)
{
    if (x0 > x1)
    {
        _gui_swap_s16(&x0, &x1);
    }
    if (y0 > y1)
    {
        _gui_swap_s16(&y0, &y1);
    }
    gui_draw_hline(x0, x1, y0, color);
    gui_draw_hline(x0, x1, y1, color);
    if (y1 - y0 > 1)
    {
        gui_draw_vline(x0, (s16)(y0 + 1), (s16)(y1 - 1), color);
        gui_draw_vline(x1, (s16)(y0 + 1), (s16)(y1 - 1), color);
    }
}

/*
 *@ brief    : Fill a solid rectangle between (x0, y0) and (x1, y1) with clipping.
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1: corner coordinates; color: RGB565 color.
 *@ argu out : Solid rectangle filled.
 *@ theme    : Clip to screen bounds and stream via lcd_fill_rect.
 */
void gui_fill_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 color)
{
    if (x0 > x1)
    {
        _gui_swap_s16(&x0, &x1);
    }
    if (y0 > y1)
    {
        _gui_swap_s16(&y0, &y1);
    }
    if (x1 < 0 || y1 < 0 || x0 >= (s16)LCD_WIDTH || y0 >= (s16)LCD_HEIGHT)
    {
        return;
    }
    if (x0 < 0)
    {
        x0 = 0;
    }
    if (y0 < 0)
    {
        y0 = 0;
    }
    if (x1 >= (s16)LCD_WIDTH)
    {
        x1 = (s16)(LCD_WIDTH - 1U);
    }
    if (y1 >= (s16)LCD_HEIGHT)
    {
        y1 = (s16)(LCD_HEIGHT - 1U);
    }
    lcd_fill_rect((u16)x0, (u16)y0, (u16)(x1 - x0 + 1), (u16)(y1 - y0 + 1), color);
}

/*
 *@ brief    : Draw a hollow rectangle with configurable border thickness.
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1: outer bounds; thickness: border width in pixels; color: RGB565 color.
 *@ argu out : Thick border rectangle drawn.
 *@ theme    : Four filled border strips.
 */
void gui_draw_rect_thick(s16 x0, s16 y0, s16 x1, s16 y1, u8 thickness, u16 color)
{
    s16 t = (s16)thickness;

    if (t <= 0)
    {
        return;
    }
    if (x0 > x1)
    {
        _gui_swap_s16(&x0, &x1);
    }
    if (y0 > y1)
    {
        _gui_swap_s16(&y0, &y1);
    }
    if ((x1 - x0 + 1) <= (t << 1) || (y1 - y0 + 1) <= (t << 1))
    {
        gui_fill_rect(x0, y0, x1, y1, color);
        return;
    }
    gui_fill_rect(x0, y0, x1, (s16)(y0 + t - 1), color);
    gui_fill_rect(x0, (s16)(y1 - t + 1), x1, y1, color);
    gui_fill_rect(x0, (s16)(y0 + t), (s16)(x0 + t - 1), (s16)(y1 - t), color);
    gui_fill_rect((s16)(x1 - t + 1), (s16)(y0 + t), x1, (s16)(y1 - t), color);
}

/*
 *@ brief    : Draw a hollow or filled rectangle according to style.
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1: corner coordinates; color: RGB565; style: Hollow or Filled.
 *@ argu out : Rectangle rendered.
 *@ theme    : Unified hollow/filled rectangle entry point.
 */
void gui_render_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 color, GUI_Style style)
{
    if (style == GUI_Style_Filled)
    {
        gui_fill_rect(x0, y0, x1, y1, color);
    }
    else
    {
        gui_draw_rect(x0, y0, x1, y1, color);
    }
}

/*
 *@ brief    : Draw a quarter-circle arc for selected quadrants.
 *@ type     : public
 *@ argu in  : xc, yc: center; r: radius; quad_mask: GUI_QUAD_* bitmask; color: RGB565 color.
 *@ argu out : Selected quadrant arcs drawn.
 *@ theme    : Midpoint circle algorithm with quadrant filtering.
 */
void gui_draw_quadrant_arc(s16 xc, s16 yc, u16 r, u8 quad_mask, u16 color)
{
    s16 x = 0;
    s16 y = (s16)r;
    s16 d = (s16)(3 - ((s16)r << 1));

    if (quad_mask == 0U)
    {
        return;
    }
    if (r == 0U)
    {
        gui_draw_point(xc, yc, color);
        return;
    }

    while (x <= y)
    {
        if ((quad_mask & GUI_QUAD_TOP_RIGHT) != 0U)
        {
            gui_draw_point((s16)(xc + x), (s16)(yc - y), color);
            gui_draw_point((s16)(xc + y), (s16)(yc - x), color);
        }
        if ((quad_mask & GUI_QUAD_TOP_LEFT) != 0U)
        {
            gui_draw_point((s16)(xc - x), (s16)(yc - y), color);
            gui_draw_point((s16)(xc - y), (s16)(yc - x), color);
        }
        if ((quad_mask & GUI_QUAD_BOTTOM_LEFT) != 0U)
        {
            gui_draw_point((s16)(xc - x), (s16)(yc + y), color);
            gui_draw_point((s16)(xc - y), (s16)(yc + x), color);
        }
        if ((quad_mask & GUI_QUAD_BOTTOM_RIGHT) != 0U)
        {
            gui_draw_point((s16)(xc + x), (s16)(yc + y), color);
            gui_draw_point((s16)(xc + y), (s16)(yc + x), color);
        }

        if (d < 0)
        {
            d = (s16)(d + (x << 2) + 6);
        }
        else
        {
            d = (s16)(d + ((x - y) << 2) + 10);
            --y;
        }
        ++x;
    }
}

/*
 *@ brief    : Draw a hollow rounded-corner rectangle.
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1: outer bounds; r: corner radius; color: RGB565 color.
 *@ argu out : Hollow rounded rectangle drawn.
 *@ theme    : Four straight edges plus four quarter-circle corner arcs.
 */
void gui_draw_round_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 r, u16 color)
{
    u16 max_r;
    s16 sr;

    if (x0 > x1)
    {
        _gui_swap_s16(&x0, &x1);
    }
    if (y0 > y1)
    {
        _gui_swap_s16(&y0, &y1);
    }

    max_r = (u16)(((x1 - x0) < (y1 - y0) ? (x1 - x0) : (y1 - y0)) >> 1);
    if (r > max_r)
    {
        r = max_r;
    }
    if (r == 0U)
    {
        gui_draw_rect(x0, y0, x1, y1, color);
        return;
    }

    sr = (s16)r;
    gui_draw_hline((s16)(x0 + sr), (s16)(x1 - sr), y0, color);
    gui_draw_hline((s16)(x0 + sr), (s16)(x1 - sr), y1, color);
    gui_draw_vline(x0, (s16)(y0 + sr), (s16)(y1 - sr), color);
    gui_draw_vline(x1, (s16)(y0 + sr), (s16)(y1 - sr), color);

    gui_draw_quadrant_arc((s16)(x0 + sr), (s16)(y0 + sr), r, GUI_QUAD_TOP_LEFT, color);
    gui_draw_quadrant_arc((s16)(x1 - sr), (s16)(y0 + sr), r, GUI_QUAD_TOP_RIGHT, color);
    gui_draw_quadrant_arc((s16)(x0 + sr), (s16)(y1 - sr), r, GUI_QUAD_BOTTOM_LEFT, color);
    gui_draw_quadrant_arc((s16)(x1 - sr), (s16)(y1 - sr), r, GUI_QUAD_BOTTOM_RIGHT, color);
}

/*
 *@ brief    : Fill a solid rounded-corner rectangle.
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1: outer bounds; r: corner radius; color: RGB565 color.
 *@ argu out : Solid rounded rectangle filled.
 *@ theme    : Center body block plus scanline-filled top and bottom rounded caps.
 */
void gui_fill_round_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 r, u16 color)
{
    u16 max_r;
    s16 sr, x, y, d;

    if (x0 > x1)
    {
        _gui_swap_s16(&x0, &x1);
    }
    if (y0 > y1)
    {
        _gui_swap_s16(&y0, &y1);
    }

    max_r = (u16)(((x1 - x0) < (y1 - y0) ? (x1 - x0) : (y1 - y0)) >> 1);
    if (r > max_r)
    {
        r = max_r;
    }
    if (r == 0U)
    {
        gui_fill_rect(x0, y0, x1, y1, color);
        return;
    }

    sr = (s16)r;
    gui_fill_rect(x0, (s16)(y0 + sr), x1, (s16)(y1 - sr), color);

    x = 0;
    y = sr;
    d = (s16)(3 - (sr << 1));
    while (x <= y)
    {
        gui_draw_hline((s16)(x0 + sr - x), (s16)(x1 - sr + x), (s16)(y0 + sr - y), color);
        gui_draw_hline((s16)(x0 + sr - y), (s16)(x1 - sr + y), (s16)(y0 + sr - x), color);
        gui_draw_hline((s16)(x0 + sr - x), (s16)(x1 - sr + x), (s16)(y1 - sr + y), color);
        gui_draw_hline((s16)(x0 + sr - y), (s16)(x1 - sr + y), (s16)(y1 - sr + x), color);

        if (d < 0)
        {
            d = (s16)(d + (x << 2) + 6);
        }
        else
        {
            d = (s16)(d + ((x - y) << 2) + 10);
            --y;
        }
        ++x;
    }
}

/*
 *@ brief    : Draw a hollow or filled rounded rectangle according to style.
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1: outer bounds; r: corner radius; color: RGB565; style: Hollow/Filled.
 *@ argu out : Rounded rectangle rendered.
 *@ theme    : Unified hollow/filled rounded rectangle entry point.
 */
void gui_render_round_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 r, u16 color, GUI_Style style)
{
    if (style == GUI_Style_Filled)
    {
        gui_fill_round_rect(x0, y0, x1, y1, r, color);
    }
    else
    {
        gui_draw_round_rect(x0, y0, x1, y1, r, color);
    }
}

/*
 *@ brief    : Draw a hollow circle centered at (xc, yc) with radius r.
 *@ type     : public
 *@ argu in  : xc, yc: center coordinates; r: radius; color: RGB565 color.
 *@ argu out : Hollow circle drawn.
 *@ theme    : 8-way symmetric midpoint circle algorithm.
 */
void gui_draw_circle(s16 xc, s16 yc, u16 r, u16 color)
{
    gui_draw_quadrant_arc(xc, yc, r, GUI_QUAD_ALL, color);
}

/*
 *@ brief    : Fill a solid circle centered at (xc, yc) with radius r.
 *@ type     : public
 *@ argu in  : xc, yc: center coordinates; r: radius; color: RGB565 color.
 *@ argu out : Solid circle filled using fast horizontal spans.
 *@ theme    : Midpoint circle scanline fill.
 */
void gui_fill_circle(s16 xc, s16 yc, u16 r, u16 color)
{
    s16 x = 0;
    s16 y = (s16)r;
    s16 d = (s16)(3 - ((s16)r << 1));

    if (r == 0U)
    {
        gui_draw_point(xc, yc, color);
        return;
    }

    while (x <= y)
    {
        gui_draw_hline((s16)(xc - x), (s16)(xc + x), (s16)(yc - y), color);
        gui_draw_hline((s16)(xc - x), (s16)(xc + x), (s16)(yc + y), color);
        gui_draw_hline((s16)(xc - y), (s16)(xc + y), (s16)(yc - x), color);
        gui_draw_hline((s16)(xc - y), (s16)(xc + y), (s16)(yc + x), color);

        if (d < 0)
        {
            d = (s16)(d + (x << 2) + 6);
        }
        else
        {
            d = (s16)(d + ((x - y) << 2) + 10);
            --y;
        }
        ++x;
    }
}

/*
 *@ brief    : Draw a hollow or filled circle according to style.
 *@ type     : public
 *@ argu in  : xc, yc: center; r: radius; color: RGB565; style: Hollow or Filled.
 *@ argu out : Circle rendered.
 *@ theme    : Unified hollow/filled circle entry point.
 */
void gui_render_circle(s16 xc, s16 yc, u16 r, u16 color, GUI_Style style)
{
    if (style == GUI_Style_Filled)
    {
        gui_fill_circle(xc, yc, r, color);
    }
    else
    {
        gui_draw_circle(xc, yc, r, color);
    }
}

/*
 *@ brief    : Draw a filled ring (annulus) between r_inner and r_outer.
 *@ type     : public
 *@ argu in  : xc, yc: center; r_inner, r_outer: inner/outer radii; color: RGB565 color.
 *@ argu out : Ring area filled.
 *@ theme    : Scanline span difference between outer and inner circles.
 */
void gui_draw_ring(s16 xc, s16 yc, u16 r_inner, u16 r_outer, u16 color)
{
    s16 dy;
    s32 r_out2, r_in2;
    s16 x_out, x_in;

    if (r_inner > r_outer)
    {
        u16 tmp = r_inner;
        r_inner = r_outer;
        r_outer = tmp;
    }
    if (r_inner == 0U)
    {
        gui_fill_circle(xc, yc, r_outer, color);
        return;
    }
    if (r_inner == r_outer)
    {
        gui_draw_circle(xc, yc, r_outer, color);
        return;
    }

    r_out2 = (s32)r_outer * (s32)r_outer;
    r_in2 = (s32)r_inner * (s32)r_inner;
    x_out = (s16)r_outer;
    x_in = (s16)r_inner;

    for (dy = 0; dy <= (s16)r_outer; ++dy)
    {
        s32 dy2 = (s32)dy * (s32)dy;

        while (x_out > 0 && ((s32)x_out * (s32)x_out + dy2) > r_out2)
        {
            --x_out;
        }
        if (dy < (s16)r_inner)
        {
            while (x_in > 0 && ((s32)x_in * (s32)x_in + dy2) >= r_in2)
            {
                --x_in;
            }
            gui_draw_hline((s16)(xc - x_out), (s16)(xc - x_in - 1), (s16)(yc - dy), color);
            gui_draw_hline((s16)(xc + x_in + 1), (s16)(xc + x_out), (s16)(yc - dy), color);
            if (dy != 0)
            {
                gui_draw_hline((s16)(xc - x_out), (s16)(xc - x_in - 1), (s16)(yc + dy), color);
                gui_draw_hline((s16)(xc + x_in + 1), (s16)(xc + x_out), (s16)(yc + dy), color);
            }
        }
        else
        {
            gui_draw_hline((s16)(xc - x_out), (s16)(xc + x_out), (s16)(yc - dy), color);
            if (dy != 0)
            {
                gui_draw_hline((s16)(xc - x_out), (s16)(xc + x_out), (s16)(yc + dy), color);
            }
        }
    }
}

/*
 *@ brief    : Draw a circular arc from start_deg to end_deg (counter-clockwise, 0=right, 90=up).
 *@ type     : public
 *@ argu in  : xc, yc: center; r: radius; start_deg, end_deg: angles in degrees; color: RGB565.
 *@ argu out : Arc drawn.
 *@ theme    : Midpoint circle points filtered by integer half-plane cross products.
 */
void gui_draw_arc(s16 xc, s16 yc, u16 r, u16 start_deg, u16 end_deg, u16 color)
{
    s16 x = 0;
    s16 y = (s16)r;
    s16 d = (s16)(3 - ((s16)r << 1));

    if (r == 0U)
    {
        gui_draw_point(xc, yc, color);
        return;
    }

    while (x <= y)
    {
        if (_gui_point_in_angle(x, y, start_deg, end_deg))
            gui_draw_point((s16)(xc + x), (s16)(yc - y), color);
        if (_gui_point_in_angle(y, x, start_deg, end_deg))
            gui_draw_point((s16)(xc + y), (s16)(yc - x), color);
        if (_gui_point_in_angle((s16)-x, y, start_deg, end_deg))
            gui_draw_point((s16)(xc - x), (s16)(yc - y), color);
        if (_gui_point_in_angle((s16)-y, x, start_deg, end_deg))
            gui_draw_point((s16)(xc - y), (s16)(yc - x), color);
        if (_gui_point_in_angle((s16)-x, (s16)-y, start_deg, end_deg))
            gui_draw_point((s16)(xc - x), (s16)(yc + y), color);
        if (_gui_point_in_angle((s16)-y, (s16)-x, start_deg, end_deg))
            gui_draw_point((s16)(xc - y), (s16)(yc + x), color);
        if (_gui_point_in_angle(x, (s16)-y, start_deg, end_deg))
            gui_draw_point((s16)(xc + x), (s16)(yc + y), color);
        if (_gui_point_in_angle(y, (s16)-x, start_deg, end_deg))
            gui_draw_point((s16)(xc + y), (s16)(yc + x), color);

        if (d < 0)
        {
            d = (s16)(d + (x << 2) + 6);
        }
        else
        {
            d = (s16)(d + ((x - y) << 2) + 10);
            --y;
        }
        ++x;
    }
}

/*
 *@ brief    : Fill a circular sector (pie wedge) from start_deg to end_deg.
 *@ type     : public
 *@ argu in  : xc, yc: center; r: radius; start_deg, end_deg: angles in degrees; color: RGB565.
 *@ argu out : Sector filled.
 *@ theme    : Bounding-box disc test combined with integer half-plane angle test.
 */
void gui_fill_sector(s16 xc, s16 yc, u16 r, u16 start_deg, u16 end_deg, u16 color)
{
    s16 dx, dy;
    s32 r2 = (s32)r * (s32)r;
    s16 sr = (s16)r;

    if (r == 0U)
    {
        gui_draw_point(xc, yc, color);
        return;
    }

    for (dy = (s16)-sr; dy <= sr; ++dy)
    {
        s32 dy2 = (s32)dy * (s32)dy;
        for (dx = (s16)-sr; dx <= sr; ++dx)
        {
            if (((s32)dx * (s32)dx + dy2) <= r2 &&
                _gui_point_in_angle(dx, (s16)-dy, start_deg, end_deg))
            {
                gui_draw_point((s16)(xc + dx), (s16)(yc + dy), color);
            }
        }
        system_clear_watchdog();
    }
}

/*
 *@ brief    : Render a hollow arc or filled pie sector according to style.
 *@ type     : public
 *@ argu in  : xc, yc: center; r: radius; start_deg, end_deg: angles; color: RGB565; style.
 *@ argu out : Arc or sector rendered.
 *@ theme    : Unified hollow/filled arc entry point.
 */
void gui_render_arc(s16 xc, s16 yc, u16 r, u16 start_deg, u16 end_deg, u16 color, GUI_Style style)
{
    if (style == GUI_Style_Filled)
    {
        gui_fill_sector(xc, yc, r, start_deg, end_deg, color);
    }
    else
    {
        gui_draw_arc(xc, yc, r, start_deg, end_deg, color);
    }
}

/*
 *@ brief    : Draw a hollow ellipse centered at (xc, yc) with semi-axes rx and ry.
 *@ type     : public
 *@ argu in  : xc, yc: center; rx, ry: horizontal and vertical radii; color: RGB565 color.
 *@ argu out : Hollow ellipse drawn.
 *@ theme    : Two-region integer midpoint ellipse algorithm.
 */
void gui_draw_ellipse(s16 xc, s16 yc, u16 rx, u16 ry, u16 color)
{
    s32 rx2, ry2, two_rx2, two_ry2;
    s32 x, y, px, py, p;

    if (rx == 0U && ry == 0U)
    {
        gui_draw_point(xc, yc, color);
        return;
    }
    if (rx == 0U)
    {
        gui_draw_vline(xc, (s16)(yc - (s16)ry), (s16)(yc + (s16)ry), color);
        return;
    }
    if (ry == 0U)
    {
        gui_draw_hline((s16)(xc - (s16)rx), (s16)(xc + (s16)rx), yc, color);
        return;
    }

    rx2 = (s32)rx * (s32)rx;
    ry2 = (s32)ry * (s32)ry;
    two_rx2 = rx2 << 1;
    two_ry2 = ry2 << 1;

    x = 0L;
    y = (s32)ry;
    px = 0L;
    py = two_rx2 * y;

    /* Region 1 */
    p = ry2 - (rx2 * (s32)ry) + (rx2 >> 2);
    while (px < py)
    {
        gui_draw_point((s16)(xc + x), (s16)(yc + y), color);
        gui_draw_point((s16)(xc - x), (s16)(yc + y), color);
        gui_draw_point((s16)(xc + x), (s16)(yc - y), color);
        gui_draw_point((s16)(xc - x), (s16)(yc - y), color);

        ++x;
        px += two_ry2;
        if (p < 0L)
        {
            p += ry2 + px;
        }
        else
        {
            --y;
            py -= two_rx2;
            p += ry2 + px - py;
        }
    }

    /* Region 2 */
    p = ry2 * x * x + rx2 * (y - 1L) * (y - 1L) - rx2 * ry2;
    while (y >= 0L)
    {
        gui_draw_point((s16)(xc + x), (s16)(yc + y), color);
        gui_draw_point((s16)(xc - x), (s16)(yc + y), color);
        gui_draw_point((s16)(xc + x), (s16)(yc - y), color);
        gui_draw_point((s16)(xc - x), (s16)(yc - y), color);

        --y;
        py -= two_rx2;
        if (p > 0L)
        {
            p += rx2 - py;
        }
        else
        {
            ++x;
            px += two_ry2;
            p += rx2 - py + px;
        }
    }
}

/*
 *@ brief    : Fill a solid ellipse centered at (xc, yc) with semi-axes rx and ry.
 *@ type     : public
 *@ argu in  : xc, yc: center; rx, ry: horizontal and vertical radii; color: RGB565 color.
 *@ argu out : Solid ellipse filled.
 *@ theme    : Two-region midpoint ellipse algorithm using horizontal spans.
 */
void gui_fill_ellipse(s16 xc, s16 yc, u16 rx, u16 ry, u16 color)
{
    s32 rx2, ry2, two_rx2, two_ry2;
    s32 x, y, px, py, p;

    if (rx == 0U && ry == 0U)
    {
        gui_draw_point(xc, yc, color);
        return;
    }
    if (rx == 0U)
    {
        gui_draw_vline(xc, (s16)(yc - (s16)ry), (s16)(yc + (s16)ry), color);
        return;
    }
    if (ry == 0U)
    {
        gui_draw_hline((s16)(xc - (s16)rx), (s16)(xc + (s16)rx), yc, color);
        return;
    }

    rx2 = (s32)rx * (s32)rx;
    ry2 = (s32)ry * (s32)ry;
    two_rx2 = rx2 << 1;
    two_ry2 = ry2 << 1;

    x = 0L;
    y = (s32)ry;
    px = 0L;
    py = two_rx2 * y;

    p = ry2 - (rx2 * (s32)ry) + (rx2 >> 2);
    while (px < py)
    {
        gui_draw_hline((s16)(xc - x), (s16)(xc + x), (s16)(yc + y), color);
        gui_draw_hline((s16)(xc - x), (s16)(xc + x), (s16)(yc - y), color);

        ++x;
        px += two_ry2;
        if (p < 0L)
        {
            p += ry2 + px;
        }
        else
        {
            --y;
            py -= two_rx2;
            p += ry2 + px - py;
        }
    }

    p = ry2 * x * x + rx2 * (y - 1L) * (y - 1L) - rx2 * ry2;
    while (y >= 0L)
    {
        gui_draw_hline((s16)(xc - x), (s16)(xc + x), (s16)(yc + y), color);
        gui_draw_hline((s16)(xc - x), (s16)(xc + x), (s16)(yc - y), color);

        --y;
        py -= two_rx2;
        if (p > 0L)
        {
            p += rx2 - py;
        }
        else
        {
            ++x;
            px += two_ry2;
            p += rx2 - py + px;
        }
    }
}

/*
 *@ brief    : Draw a hollow or filled ellipse according to style.
 *@ type     : public
 *@ argu in  : xc, yc: center; rx, ry: semi-axes; color: RGB565; style: Hollow or Filled.
 *@ argu out : Ellipse rendered.
 *@ theme    : Unified hollow/filled ellipse entry point.
 */
void gui_render_ellipse(s16 xc, s16 yc, u16 rx, u16 ry, u16 color, GUI_Style style)
{
    if (style == GUI_Style_Filled)
    {
        gui_fill_ellipse(xc, yc, rx, ry, color);
    }
    else
    {
        gui_draw_ellipse(xc, yc, rx, ry, color);
    }
}

/*
 *@ brief    : Draw a hollow triangle with vertices (x0, y0), (x1, y1), (x2, y2).
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1, x2, y2: triangle vertices; color: RGB565 color.
 *@ argu out : Hollow triangle drawn.
 *@ theme    : Connect three edges with gui_draw_line.
 */
void gui_draw_triangle(s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, u16 color)
{
    gui_draw_line(x0, y0, x1, y1, color);
    gui_draw_line(x1, y1, x2, y2, color);
    gui_draw_line(x2, y2, x0, y0, color);
}

/*
 *@ brief    : Fill a solid triangle with vertices (x0, y0), (x1, y1), (x2, y2).
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1, x2, y2: triangle vertices; color: RGB565 color.
 *@ argu out : Solid triangle filled via horizontal scanlines.
 *@ theme    : Sort vertices by Y and rasterize upper/lower half-triangles with gui_draw_hline.
 */
void gui_fill_triangle(s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, u16 color)
{
    s16 a, b, y, last;
    s16 dx01, dy01, dx02, dy02, dx12, dy12;
    s32 sa = 0L;
    s32 sb = 0L;

    if (y0 > y1)
    {
        _gui_swap_s16(&y0, &y1);
        _gui_swap_s16(&x0, &x1);
    }
    if (y1 > y2)
    {
        _gui_swap_s16(&y2, &y1);
        _gui_swap_s16(&x2, &x1);
    }
    if (y0 > y1)
    {
        _gui_swap_s16(&y0, &y1);
        _gui_swap_s16(&x0, &x1);
    }

    if (y0 == y2)
    {
        a = x0;
        b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        gui_draw_hline(a, b, y0, color);
        return;
    }

    dx01 = x1 - x0;
    dy01 = y1 - y0;
    dx02 = x2 - x0;
    dy02 = y2 - y0;
    dx12 = x2 - x1;
    dy12 = y2 - y1;

    last = (y1 == y2) ? y1 : (s16)(y1 - 1);
    for (y = y0; y <= last; ++y)
    {
        a = (s16)(x0 + (s16)(sa / dy01));
        b = (s16)(x0 + (s16)(sb / dy02));
        sa += dx01;
        sb += dx02;
        gui_draw_hline(a, b, y, color);
    }

    sa = (s32)dx12 * (s32)(y - y1);
    sb = (s32)dx02 * (s32)(y - y0);
    for (; y <= y2; ++y)
    {
        a = (s16)(x1 + (s16)(sa / dy12));
        b = (s16)(x0 + (s16)(sb / dy02));
        sa += dx12;
        sb += dx02;
        gui_draw_hline(a, b, y, color);
    }
}

/*
 *@ brief    : Draw a hollow or filled triangle according to style.
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1, x2, y2: vertices; color: RGB565; style: Hollow or Filled.
 *@ argu out : Triangle rendered.
 *@ theme    : Unified hollow/filled triangle entry point.
 */
void gui_render_triangle(s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, u16 color,
                         GUI_Style style)
{
    if (style == GUI_Style_Filled)
    {
        gui_fill_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else
    {
        gui_draw_triangle(x0, y0, x1, y1, x2, y2, color);
    }
}

/*
 *@ brief    : Draw an open polyline connecting a sequence of points.
 *@ type     : public
 *@ argu in  : pts: vertex array; count: number of vertices; color: RGB565 color.
 *@ argu out : Connected segments drawn.
 *@ theme    : Sequential line segments without closing the last vertex.
 */
void gui_draw_polyline(const GUI_Point *pts, u16 count, u16 color)
{
    u16 i;

    if (pts == null || count == 0U)
    {
        return;
    }
    if (count == 1U)
    {
        gui_draw_point(pts[0].x, pts[0].y, color);
        return;
    }
    for (i = 0U; i + 1U < count; ++i)
    {
        gui_draw_line(pts[i].x, pts[i].y, pts[i + 1U].x, pts[i + 1U].y, color);
    }
}

/*
 *@ brief    : Draw a closed hollow polygon from a vertex array.
 *@ type     : public
 *@ argu in  : pts: vertex array; count: number of vertices; color: RGB565 color.
 *@ argu out : Closed polygon outline drawn.
 *@ theme    : Connect consecutive vertices and close last to first.
 */
void gui_draw_polygon(const GUI_Point *pts, u16 count, u16 color)
{
    if (pts == null || count == 0U)
    {
        return;
    }
    gui_draw_polyline(pts, count, color);
    if (count > 2U)
    {
        gui_draw_line(pts[count - 1U].x, pts[count - 1U].y, pts[0].x, pts[0].y, color);
    }
}

/*
 *@ brief    : Fill a polygon (fan triangulation from vertex 0).
 *@ type     : public
 *@ argu in  : pts: vertex array; count: number of vertices; color: RGB565 color.
 *@ argu out : Polygon interior filled.
 *@ theme    : Zero-RAM triangle fan decomposition suitable for convex/star polygons.
 */
void gui_fill_polygon(const GUI_Point *pts, u16 count, u16 color)
{
    u16 i;

    if (pts == null || count == 0U)
    {
        return;
    }
    if (count == 1U)
    {
        gui_draw_point(pts[0].x, pts[0].y, color);
        return;
    }
    if (count == 2U)
    {
        gui_draw_line(pts[0].x, pts[0].y, pts[1].x, pts[1].y, color);
        return;
    }
    for (i = 1U; i + 1U < count; ++i)
    {
        gui_fill_triangle(pts[0].x, pts[0].y, pts[i].x, pts[i].y, pts[i + 1U].x, pts[i + 1U].y,
                          color);
    }
}

/*
 *@ brief    : Draw a hollow or filled polygon according to style.
 *@ type     : public
 *@ argu in  : pts: vertex array; count: vertex count; color: RGB565; style: Hollow or Filled.
 *@ argu out : Polygon rendered.
 *@ theme    : Unified hollow/filled polygon entry point.
 */
void gui_render_polygon(const GUI_Point *pts, u16 count, u16 color, GUI_Style style)
{
    if (style == GUI_Style_Filled)
    {
        gui_fill_polygon(pts, count, color);
    }
    else
    {
        gui_draw_polygon(pts, count, color);
    }
}

/*
 *@ brief    : Draw a crosshair marker centered at (xc, yc).
 *@ type     : public
 *@ argu in  : xc, yc: center coordinates; size: half-arm length; color: RGB565 color.
 *@ argu out : Crosshair drawn.
 *@ theme    : Orthogonal horizontal and vertical spans.
 */
void gui_draw_cross(s16 xc, s16 yc, u16 size, u16 color)
{
    s16 s = (s16)size;
    gui_draw_hline((s16)(xc - s), (s16)(xc + s), yc, color);
    gui_draw_vline(xc, (s16)(yc - s), (s16)(yc + s), color);
}

/*
 *@ brief    : Draw a bordered horizontal progress bar (0..100%).
 *@ type     : public
 *@ argu in  : x0, y0, x1, y1: bounds; percent: 0..100; fg_color, bg_color, border_color: RGB565.
 *@ argu out : Progress bar rendered.
 *@ theme    : Common UI widget built on hollow/filled rectangle primitives.
 */
void gui_draw_progress_bar(s16 x0, s16 y0, s16 x1, s16 y1, u8 percent, u16 fg_color, u16 bg_color,
                           u16 border_color)
{
    s16 ix0, iy0, ix1, iy1, inner_w, fill_w;

    if (x0 > x1)
    {
        _gui_swap_s16(&x0, &x1);
    }
    if (y0 > y1)
    {
        _gui_swap_s16(&y0, &y1);
    }
    if (percent > 100U)
    {
        percent = 100U;
    }

    gui_draw_rect(x0, y0, x1, y1, border_color);
    ix0 = (s16)(x0 + 1);
    iy0 = (s16)(y0 + 1);
    ix1 = (s16)(x1 - 1);
    iy1 = (s16)(y1 - 1);
    if (ix0 > ix1 || iy0 > iy1)
    {
        return;
    }

    inner_w = (s16)(ix1 - ix0 + 1);
    fill_w = (s16)(((u32)(u16)inner_w * (u32)percent + 50UL) / 100UL);
    if (fill_w > 0)
    {
        gui_fill_rect(ix0, iy0, (s16)(ix0 + fill_w - 1), iy1, fg_color);
    }
    if (fill_w < inner_w)
    {
        gui_fill_rect((s16)(ix0 + fill_w), iy0, ix1, iy1, bg_color);
    }
}

/*
 *@ brief    : Draw an RGB565 bitmap image at (x, y) with width w and height h.
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; w, h: dimensions; bitmap: RGB565 pixel array.
 *@ argu out : Image streamed to GRAM.
 *@ theme    : Fast full-window SPI streaming when inside screen bounds, with clipped fallback.
 */
void gui_draw_bitmap_rgb565(s16 x, s16 y, u16 w, u16 h, const u16 *bitmap)
{
    u16 r, c;

    if (bitmap == null || w == 0U || h == 0U)
    {
        return;
    }

    if (x >= 0 && y >= 0 && (u16)x + w <= LCD_WIDTH && (u16)y + h <= LCD_HEIGHT)
    {
        lcd_draw_bitmap((u16)x, (u16)y, w, h, bitmap);
        return;
    }

    for (r = 0U; r < h; ++r)
    {
        for (c = 0U; c < w; ++c)
        {
            gui_draw_point((s16)(x + (s16)c), (s16)(y + (s16)r), bitmap[(u32)r * (u32)w + c]);
        }
        system_clear_watchdog();
    }
}

/*
 *@ brief    : Legacy compatible window setting API (x0, x1, y0, y1 order).
 *@ type     : public
 *@ argu in  : x0, x1: column range; y0, y1: row range.
 *@ argu out : ST7735_SUCCESS or ST7735_ERROR.
 *@ theme    : Preserve gui_set_window signature and behavior.
 */
uint8_t gui_set_window(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    if ((x0 > x1) || (x1 > SIZE_X) || (y0 > y1) || (y1 > SIZE_Y))
    {
        return ST7735_ERROR;
    }
    lcd_set_window(x0, y0, x1, y1);
    return ST7735_SUCCESS;
}

/*
 *@ brief    : Legacy compatible RGB565 pixel repeat helper.
 *@ type     : public
 *@ argu in  : color: RGB565 color; count: number of pixels.
 *@ argu out : Pixels written to GRAM.
 *@ theme    : Compatible with gui_send_rgb565.
 */
void gui_send_rgb565(uint16_t color, uint16_t count)
{
    const LCD_Device *dev = lcd_get_device();
    dev->write_cmd(LCD_CMD_RAMWR);
    dev->write_repeat_u16(color, count);
}

/*
 *@ brief    : Legacy compatible filled rectangle API (xs, xe, ys, ye order).
 *@ type     : public
 *@ argu in  : xs, xe: column bounds; ys, ye: row bounds; color: RGB565 color.
 *@ argu out : Filled rectangle drawn.
 *@ theme    : Compatible with gui_draw_rectangle.
 */
void gui_draw_rectangle(uint8_t xs, uint8_t xe, uint8_t ys, uint8_t ye, uint16_t color)
{
    gui_fill_rect((s16)xs, (s16)ys, (s16)xe, (s16)ye, color);
}
