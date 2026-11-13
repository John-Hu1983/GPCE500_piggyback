#ifndef _GUI_GEOMETRY_H__
#define _GUI_GEOMETRY_H__

/*
 * Common 2D graphics primitives over ST7735P3 (128x160 RGB565):
 * - Out-of-bounds pixels and scanlines are safely clipped to [0..LCD_WIDTH-1, 0..LCD_HEIGHT-1].
 * - Horizontal, vertical and filled spans use hardware window streaming (lcd_fill_rect).
 * - All shapes support both hollow (GUI_Style_Hollow) and solid/filled (GUI_Style_Filled) modes.
 */
#define GUI_RGB565(r, g, b)                                                                        \
    ((u16)((((u16)(r) & 0xF8U) << 8) | (((u16)(g) & 0xFCU) << 3) | (((u16)(b) & 0xF8U) >> 3)))

// clang-format off
/* Standard RGB565 GUI color palette */
#define GUI_COLOR_BLACK         LCD_COLOR_BLACK
#define GUI_COLOR_WHITE         LCD_COLOR_WHITE
#define GUI_COLOR_RED           LCD_COLOR_RED
#define GUI_COLOR_GREEN         LCD_COLOR_GREEN
#define GUI_COLOR_BLUE          LCD_COLOR_BLUE
#define GUI_COLOR_YELLOW        LCD_COLOR_YELLOW
#define GUI_COLOR_CYAN          LCD_COLOR_CYAN
#define GUI_COLOR_MAGENTA       LCD_COLOR_MAGENTA
#define GUI_COLOR_GRAY          0x8410U
#define GUI_COLOR_DARKGRAY      0x4208U
#define GUI_COLOR_LIGHTGRAY     0xC618U
#define GUI_COLOR_NAVY          0x000FU
#define GUI_COLOR_ORANGE        0xFD20U

/* Quadrant mask bits for quarter-circle arcs and rounded corners */
#define GUI_QUAD_TOP_RIGHT      0x01U   /* 0..90 deg    (x >= xc, y <= yc) */
#define GUI_QUAD_TOP_LEFT       0x02U   /* 90..180 deg  (x <= xc, y <= yc) */
#define GUI_QUAD_BOTTOM_LEFT    0x04U   /* 180..270 deg (x <= xc, y >= yc) */
#define GUI_QUAD_BOTTOM_RIGHT   0x08U   /* 270..360 deg (x >= xc, y >= yc) */
#define GUI_QUAD_ALL            0x0FU
// clang-format on

typedef enum
{
    GUI_Style_Hollow = 0,
    GUI_Style_Filled = 1
} GUI_Style;

typedef struct
{
    s16 x;
    s16 y;
} GUI_Point;

/* Color & screen helpers */
u16 gui_rgb565(u8 r, u8 g, u8 b);
void gui_clear_screen(u16 color);

/* Point & line primitives */
void gui_draw_point(s16 x, s16 y, u16 color);
void gui_draw_pixel(u8 x, u8 y, u16 color);
void gui_draw_point_thick(s16 x, s16 y, u8 radius, u16 color);
void gui_draw_hline(s16 x0, s16 x1, s16 y, u16 color);
void gui_draw_vline(s16 x, s16 y0, s16 y1, u16 color);
void gui_draw_line(s16 x0, s16 y0, s16 x1, s16 y1, u16 color);
void gui_draw_line_thick(s16 x0, s16 y0, s16 x1, s16 y1, u8 thickness, u16 color);
void gui_draw_dashed_line(s16 x0, s16 y0, s16 x1, s16 y1, u8 dash_len, u8 gap_len, u16 color);

/* Rectangle & rounded rectangle (hollow / filled) */
void gui_draw_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 color);
void gui_fill_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 color);
void gui_draw_rect_thick(s16 x0, s16 y0, s16 x1, s16 y1, u8 thickness, u16 color);
void gui_render_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 color, GUI_Style style);
void gui_draw_round_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 r, u16 color);
void gui_fill_round_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 r, u16 color);
void gui_render_round_rect(s16 x0, s16 y0, s16 x1, s16 y1, u16 r, u16 color, GUI_Style style);

/* Circle, quadrant arc, degree arc / sector, ellipse & ring (hollow / filled) */
void gui_draw_circle(s16 xc, s16 yc, u16 r, u16 color);
void gui_fill_circle(s16 xc, s16 yc, u16 r, u16 color);
void gui_render_circle(s16 xc, s16 yc, u16 r, u16 color, GUI_Style style);
void gui_draw_ring(s16 xc, s16 yc, u16 r_inner, u16 r_outer, u16 color);
void gui_draw_quadrant_arc(s16 xc, s16 yc, u16 r, u8 quad_mask, u16 color);
void gui_draw_arc(s16 xc, s16 yc, u16 r, u16 start_deg, u16 end_deg, u16 color);
void gui_fill_sector(s16 xc, s16 yc, u16 r, u16 start_deg, u16 end_deg, u16 color);
void gui_render_arc(s16 xc, s16 yc, u16 r, u16 start_deg, u16 end_deg, u16 color, GUI_Style style);
void gui_draw_ellipse(s16 xc, s16 yc, u16 rx, u16 ry, u16 color);
void gui_fill_ellipse(s16 xc, s16 yc, u16 rx, u16 ry, u16 color);
void gui_render_ellipse(s16 xc, s16 yc, u16 rx, u16 ry, u16 color, GUI_Style style);

/* Triangle, polyline & polygon (hollow / filled) */
void gui_draw_triangle(s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, u16 color);
void gui_fill_triangle(s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, u16 color);
void gui_render_triangle(s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, u16 color,
                         GUI_Style style);
void gui_draw_polyline(const GUI_Point *pts, u16 count, u16 color);
void gui_draw_polygon(const GUI_Point *pts, u16 count, u16 color);
void gui_fill_polygon(const GUI_Point *pts, u16 count, u16 color);
void gui_render_polygon(const GUI_Point *pts, u16 count, u16 color, GUI_Style style);

/* Crosshair, progress bar & RGB565 bitmap */
void gui_draw_cross(s16 xc, s16 yc, u16 size, u16 color);
void gui_draw_progress_bar(s16 x0, s16 y0, s16 x1, s16 y1, u8 percent, u16 fg_color, u16 bg_color,
                           u16 border_color);
void gui_draw_bitmap_rgb565(s16 x, s16 y, u16 w, u16 h, const u16 *bitmap);

/* Legacy ST7735 GUI compatibility helpers */
uint8_t gui_set_window(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1);
void gui_send_rgb565(uint16_t color, uint16_t count);
void gui_draw_rectangle(uint8_t xs, uint8_t xe, uint8_t ys, uint8_t ye, uint16_t color);

#endif /* _GUI_GEOMETRY_H__ */
