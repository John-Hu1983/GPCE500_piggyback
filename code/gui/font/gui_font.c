#include "..\..\Include\spinmaster.h"

/* Standard 5x8 ASCII column-major bitmap font (0x20..0x7F) */
const unsigned char FONTS[][CHARS_COLS_LENGTH] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, /* 20 space */
    {0x81, 0x81, 0x18, 0x81, 0x81}, /* 21 ! */
    {0x00, 0x07, 0x00, 0x07, 0x00}, /* 22 " */
    {0x14, 0x7f, 0x14, 0x7f, 0x14}, /* 23 # */
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, /* 24 $ */
    {0x23, 0x13, 0x08, 0x64, 0x62}, /* 25 % */
    {0x36, 0x49, 0x55, 0x22, 0x50}, /* 26 & */
    {0x00, 0x05, 0x03, 0x00, 0x00}, /* 27 ' */
    {0x00, 0x1c, 0x22, 0x41, 0x00}, /* 28 ( */
    {0x00, 0x41, 0x22, 0x1c, 0x00}, /* 29 ) */
    {0x14, 0x08, 0x3e, 0x08, 0x14}, /* 2a * */
    {0x08, 0x08, 0x3e, 0x08, 0x08}, /* 2b + */
    {0x00, 0x50, 0x30, 0x00, 0x00}, /* 2c , */
    {0x08, 0x08, 0x08, 0x08, 0x08}, /* 2d - */
    {0x00, 0x60, 0x60, 0x00, 0x00}, /* 2e . */
    {0x20, 0x10, 0x08, 0x04, 0x02}, /* 2f / */
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, /* 30 0 */
    {0x00, 0x42, 0x7f, 0x40, 0x00}, /* 31 1 */
    {0x42, 0x61, 0x51, 0x49, 0x46}, /* 32 2 */
    {0x21, 0x41, 0x45, 0x4b, 0x31}, /* 33 3 */
    {0x18, 0x14, 0x12, 0x7f, 0x10}, /* 34 4 */
    {0x27, 0x45, 0x45, 0x45, 0x39}, /* 35 5 */
    {0x3c, 0x4a, 0x49, 0x49, 0x30}, /* 36 6 */
    {0x01, 0x71, 0x09, 0x05, 0x03}, /* 37 7 */
    {0x36, 0x49, 0x49, 0x49, 0x36}, /* 38 8 */
    {0x06, 0x49, 0x49, 0x29, 0x1e}, /* 39 9 */
    {0x00, 0x36, 0x36, 0x00, 0x00}, /* 3a : */
    {0x00, 0x56, 0x36, 0x00, 0x00}, /* 3b ; */
    {0x08, 0x14, 0x22, 0x41, 0x00}, /* 3c < */
    {0x14, 0x14, 0x14, 0x14, 0x14}, /* 3d = */
    {0x00, 0x41, 0x22, 0x14, 0x08}, /* 3e > */
    {0x02, 0x01, 0x51, 0x09, 0x06}, /* 3f ? */
    {0x32, 0x49, 0x79, 0x41, 0x3e}, /* 40 @ */
    {0x7e, 0x11, 0x11, 0x11, 0x7e}, /* 41 A */
    {0x7f, 0x49, 0x49, 0x49, 0x36}, /* 42 B */
    {0x3e, 0x41, 0x41, 0x41, 0x22}, /* 43 C */
    {0x7f, 0x41, 0x41, 0x22, 0x1c}, /* 44 D */
    {0x7f, 0x49, 0x49, 0x49, 0x41}, /* 45 E */
    {0x7f, 0x09, 0x09, 0x09, 0x01}, /* 46 F */
    {0x3e, 0x41, 0x49, 0x49, 0x7a}, /* 47 G */
    {0x7f, 0x08, 0x08, 0x08, 0x7f}, /* 48 H */
    {0x00, 0x41, 0x7f, 0x41, 0x00}, /* 49 I */
    {0x20, 0x40, 0x41, 0x3f, 0x01}, /* 4a J */
    {0x7f, 0x08, 0x14, 0x22, 0x41}, /* 4b K */
    {0x7f, 0x40, 0x40, 0x40, 0x40}, /* 4c L */
    {0x7f, 0x02, 0x0c, 0x02, 0x7f}, /* 4d M */
    {0x7f, 0x04, 0x08, 0x10, 0x7f}, /* 4e N */
    {0x3e, 0x41, 0x41, 0x41, 0x3e}, /* 4f O */
    {0x7f, 0x09, 0x09, 0x09, 0x06}, /* 50 P */
    {0x3e, 0x41, 0x51, 0x21, 0x5e}, /* 51 Q */
    {0x7f, 0x09, 0x19, 0x29, 0x46}, /* 52 R */
    {0x46, 0x49, 0x49, 0x49, 0x31}, /* 53 S */
    {0x01, 0x01, 0x7f, 0x01, 0x01}, /* 54 T */
    {0x3f, 0x40, 0x40, 0x40, 0x3f}, /* 55 U */
    {0x1f, 0x20, 0x40, 0x20, 0x1f}, /* 56 V */
    {0x3f, 0x40, 0x38, 0x40, 0x3f}, /* 57 W */
    {0x63, 0x14, 0x08, 0x14, 0x63}, /* 58 X */
    {0x07, 0x08, 0x70, 0x08, 0x07}, /* 59 Y */
    {0x61, 0x51, 0x49, 0x45, 0x43}, /* 5a Z */
    {0x00, 0x7f, 0x41, 0x41, 0x00}, /* 5b [ */
    {0x02, 0x04, 0x08, 0x10, 0x20}, /* 5c backslash */
    {0x00, 0x41, 0x41, 0x7f, 0x00}, /* 5d ] */
    {0x04, 0x02, 0x01, 0x02, 0x04}, /* 5e ^ */
    {0x40, 0x40, 0x40, 0x40, 0x40}, /* 5f _ */
    {0x00, 0x01, 0x02, 0x04, 0x00}, /* 60 ` */
    {0x20, 0x54, 0x54, 0x54, 0x78}, /* 61 a */
    {0x7f, 0x48, 0x44, 0x44, 0x38}, /* 62 b */
    {0x38, 0x44, 0x44, 0x44, 0x20}, /* 63 c */
    {0x38, 0x44, 0x44, 0x48, 0x7f}, /* 64 d */
    {0x38, 0x54, 0x54, 0x54, 0x18}, /* 65 e */
    {0x08, 0x7e, 0x09, 0x01, 0x02}, /* 66 f */
    {0x0c, 0x52, 0x52, 0x52, 0x3e}, /* 67 g */
    {0x7f, 0x08, 0x04, 0x04, 0x78}, /* 68 h */
    {0x00, 0x44, 0x7d, 0x40, 0x00}, /* 69 i */
    {0x20, 0x40, 0x44, 0x3d, 0x00}, /* 6a j */
    {0x7f, 0x10, 0x28, 0x44, 0x00}, /* 6b k */
    {0x00, 0x41, 0x7f, 0x40, 0x00}, /* 6c l */
    {0x7c, 0x04, 0x18, 0x04, 0x78}, /* 6d m */
    {0x7c, 0x08, 0x04, 0x04, 0x78}, /* 6e n */
    {0x38, 0x44, 0x44, 0x44, 0x38}, /* 6f o */
    {0x7c, 0x14, 0x14, 0x14, 0x08}, /* 70 p */
    {0x08, 0x14, 0x14, 0x14, 0x7c}, /* 71 q */
    {0x7c, 0x08, 0x04, 0x04, 0x08}, /* 72 r */
    {0x48, 0x54, 0x54, 0x54, 0x20}, /* 73 s */
    {0x04, 0x3f, 0x44, 0x40, 0x20}, /* 74 t */
    {0x3c, 0x40, 0x40, 0x20, 0x7c}, /* 75 u */
    {0x1c, 0x20, 0x40, 0x20, 0x1c}, /* 76 v */
    {0x3c, 0x40, 0x30, 0x40, 0x3c}, /* 77 w */
    {0x44, 0x28, 0x10, 0x28, 0x44}, /* 78 x */
    {0x0c, 0x50, 0x50, 0x50, 0x3c}, /* 79 y */
    {0x44, 0x64, 0x54, 0x4c, 0x44}, /* 7a z */
    {0x00, 0x08, 0x36, 0x41, 0x00}, /* 7b { */
    {0x00, 0x00, 0x7f, 0x00, 0x00}, /* 7c | */
    {0x00, 0x41, 0x36, 0x08, 0x00}, /* 7d } */
    {0x10, 0x08, 0x08, 0x10, 0x08}, /* 7e ~ */
    {0x00, 0x00, 0x00, 0x00, 0x00}  /* 7f */
};

const GUI_Font_Desc gui_font_5x8 = {
    CHARS_COLS_LEN, CHARS_ROWS_LEN, 1U, 1U, 0x20U, 0x7FU, &FONTS[0][0]};

/* Legacy cursor state used by gui_set_position / gui_draw_character / gui_draw_string */
static u16 font_cursor_x = 0U;
static u16 font_cursor_y = 0U;

/*
 *@ brief    : Map GUI_Font_Size enum to horizontal and vertical scale factors.
 *@ type     : private
 *@ argu in  : size: font size selector; sx, sy: output scale pointers.
 *@ argu out : sx and sy populated with 1..4 multipliers.
 *@ theme    : Centralize font size scaling factors.
 */
static void _font_get_scale(GUI_Font_Size size, u8 *sx, u8 *sy)
{
    switch (size)
    {
    case GUI_Font_6x8:
        *sx = 1U;
        *sy = 1U;
        break;
    case GUI_Font_6x16:
        *sx = 1U;
        *sy = 2U;
        break;
    case GUI_Font_12x16:
        *sx = 2U;
        *sy = 2U;
        break;
    case GUI_Font_18x24:
        *sx = 3U;
        *sy = 3U;
        break;
    case GUI_Font_24x32:
        *sx = 4U;
        *sy = 4U;
        break;
    default:
        *sx = 1U;
        *sy = 1U;
        break;
    }
}

/*
 *@ brief    : Return character cell width (including inter-character spacing) for a font size.
 *@ type     : public
 *@ argu in  : size: GUI_Font_Size enum value.
 *@ argu out : Cell width in pixels.
 *@ theme    : Font metrics helper for layout alignment.
 */
u16 gui_font_get_char_width(GUI_Font_Size size)
{
    u8 sx, sy;
    _font_get_scale(size, &sx, &sy);
    return (u16)((gui_font_5x8.width + gui_font_5x8.spacing_x) * sx);
}

/*
 *@ brief    : Return character cell height for a font size.
 *@ type     : public
 *@ argu in  : size: GUI_Font_Size enum value.
 *@ argu out : Cell height in pixels.
 *@ theme    : Font metrics helper for layout alignment.
 */
u16 gui_font_get_char_height(GUI_Font_Size size)
{
    u8 sx, sy;
    _font_get_scale(size, &sx, &sy);
    return (u16)(gui_font_5x8.height * sy);
}

/*
 *@ brief    : Calculate the pixel width of a single-line string for a given font size.
 *@ type     : public
 *@ argu in  : str: null-terminated string; size: GUI_Font_Size enum value.
 *@ argu out : Total pixel width.
 *@ theme    : Enables centered or right-aligned text positioning.
 */
u16 gui_font_get_str_width(const char *str, GUI_Font_Size size)
{
    u16 len = 0U;
    u8 sx, sy;

    if (str == null)
    {
        return 0U;
    }
    while (str[len] != '\0' && str[len] != '\n' && str[len] != '\r')
    {
        len++;
    }
    if (len == 0U)
    {
        return 0U;
    }
    _font_get_scale(size, &sx, &sy);
    return (u16)((len * (gui_font_5x8.width + gui_font_5x8.spacing_x) - gui_font_5x8.spacing_x) *
                 sx);
}

/*
 *@ brief    : Draw one character with arbitrary horizontal/vertical scaling.
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; ch: ASCII character; fg_color, bg_color: RGB565 colors;
 *             has_bg: true to fill background cell, false for transparent background;
 *             scale_x, scale_y: horizontal and vertical pixel multipliers (>= 1).
 *@ argu out : Character rendered to LCD.
 *@ theme    : Core multi-size glyph renderer supporting both transparent and opaque modes.
 */
void gui_draw_char_scaled(s16 x, s16 y, char ch, u16 fg_color, u16 bg_color, bool has_bg,
                          u8 scale_x, u8 scale_y)
{
    u8 code = (u8)ch;
    u8 col, row, dy;
    u8 col_bits;
    u16 cell_w, cell_h;

    if (scale_x == 0U || scale_y == 0U)
    {
        return;
    }
    if (code < gui_font_5x8.first_char || code > gui_font_5x8.last_char)
    {
        code = (u8)' ';
    }

    cell_w = (u16)((gui_font_5x8.width + gui_font_5x8.spacing_x) * scale_x);
    cell_h = (u16)(gui_font_5x8.height * scale_y);

    /* Fast window streaming when opaque and completely within screen bounds */
    if (has_bg && x >= 0 && y >= 0 && (u16)x + cell_w <= LCD_WIDTH && (u16)y + cell_h <= LCD_HEIGHT)
    {
        const LCD_Device *dev = lcd_get_device();
        lcd_set_window((u16)x, (u16)y, (u16)((u16)x + cell_w - 1U), (u16)((u16)y + cell_h - 1U));
        for (row = 0U; row < gui_font_5x8.height; ++row)
        {
            for (dy = 0U; dy < scale_y; ++dy)
            {
                for (col = 0U; col < gui_font_5x8.width; ++col)
                {
                    u16 px;
                    col_bits = FONTS[code - gui_font_5x8.first_char][col];
                    px = ((col_bits & (u8)(1U << row)) != 0U) ? fg_color : bg_color;
                    dev->write_repeat_u16(px, scale_x);
                }
                dev->write_repeat_u16(bg_color, (u32)(gui_font_5x8.spacing_x * scale_x));
            }
        }
        return;
    }

    if (has_bg)
    {
        gui_fill_rect(x, y, (s16)(x + (s16)cell_w - 1), (s16)(y + (s16)cell_h - 1), bg_color);
    }

    for (col = 0U; col < gui_font_5x8.width; ++col)
    {
        col_bits = FONTS[code - gui_font_5x8.first_char][col];
        if (col_bits == 0U)
        {
            continue;
        }
        for (row = 0U; row < gui_font_5x8.height; ++row)
        {
            if ((col_bits & (u8)(1U << row)) != 0U)
            {
                s16 px_x = (s16)(x + (s16)(col * scale_x));
                s16 px_y = (s16)(y + (s16)(row * scale_y));
                if (scale_x == 1U && scale_y == 1U)
                {
                    gui_draw_point(px_x, px_y, fg_color);
                }
                else
                {
                    gui_fill_rect(px_x, px_y, (s16)(px_x + (s16)scale_x - 1),
                                  (s16)(px_y + (s16)scale_y - 1), fg_color);
                }
            }
        }
    }
}

/*
 *@ brief    : Draw a null-terminated string with arbitrary horizontal/vertical scaling and line wrap.
 *@ type     : public
 *@ argu in  : x, y: starting top-left coordinates; str: text string;
 *             fg_color, bg_color: RGB565 colors; has_bg: opaque if true;
 *             scale_x, scale_y: horizontal and vertical pixel multipliers.
 *@ argu out : String rendered with automatic line wrapping and '\n' support.
 *@ theme    : Core multi-size string renderer.
 */
void gui_draw_str_scaled(s16 x, s16 y, const char *str, u16 fg_color, u16 bg_color, bool has_bg,
                         u8 scale_x, u8 scale_y)
{
    s16 cur_x = x;
    s16 cur_y = y;
    s16 adv_x, adv_y;

    if (str == null || scale_x == 0U || scale_y == 0U)
    {
        return;
    }

    adv_x = (s16)((gui_font_5x8.width + gui_font_5x8.spacing_x) * scale_x);
    adv_y = (s16)((gui_font_5x8.height + gui_font_5x8.spacing_y) * scale_y);

    while (*str != '\0')
    {
        char ch = *str++;
        if (ch == '\r')
        {
            continue;
        }
        if (ch == '\n')
        {
            cur_x = x;
            cur_y = (s16)(cur_y + adv_y);
            continue;
        }
        if (cur_x + adv_x > (s16)LCD_WIDTH)
        {
            cur_x = x;
            cur_y = (s16)(cur_y + adv_y);
        }
        if (cur_y >= (s16)LCD_HEIGHT)
        {
            break;
        }
        gui_draw_char_scaled(cur_x, cur_y, ch, fg_color, bg_color, has_bg, scale_x, scale_y);
        cur_x = (s16)(cur_x + adv_x);
        system_clear_watchdog();
    }
}

/*
 *@ brief    : Draw one character with transparent background at (x, y) in the selected font size.
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; ch: ASCII char; color: RGB565 color; size: font size.
 *@ argu out : Character drawn.
 *@ theme    : Convenient preset wrapper around gui_draw_char_scaled.
 */
void gui_draw_char_at(s16 x, s16 y, char ch, u16 color, GUI_Font_Size size)
{
    u8 sx, sy;
    _font_get_scale(size, &sx, &sy);
    gui_draw_char_scaled(x, y, ch, color, 0U, false, sx, sy);
}

/*
 *@ brief    : Draw one character with solid background at (x, y) in the selected font size.
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; ch: ASCII char; fg_color, bg_color: RGB565 colors; size: font size.
 *@ argu out : Character drawn with background fill.
 *@ theme    : Flicker-free character update via window streaming.
 */
void gui_draw_char_bg(s16 x, s16 y, char ch, u16 fg_color, u16 bg_color, GUI_Font_Size size)
{
    u8 sx, sy;
    _font_get_scale(size, &sx, &sy);
    gui_draw_char_scaled(x, y, ch, fg_color, bg_color, true, sx, sy);
}

/*
 *@ brief    : Draw a string with transparent background at (x, y) in the selected font size.
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; str: string; color: RGB565 color; size: font size.
 *@ argu out : String drawn.
 *@ theme    : Convenient preset wrapper around gui_draw_str_scaled.
 */
void gui_draw_str_at(s16 x, s16 y, const char *str, u16 color, GUI_Font_Size size)
{
    u8 sx, sy;
    _font_get_scale(size, &sx, &sy);
    gui_draw_str_scaled(x, y, str, color, 0U, false, sx, sy);
}

/*
 *@ brief    : Draw a string with solid background at (x, y) in the selected font size.
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; str: string; fg_color, bg_color: RGB565 colors; size: font size.
 *@ argu out : String drawn with background fill.
 *@ theme    : Flicker-free text update without clearing the whole screen.
 */
void gui_draw_str_bg(s16 x, s16 y, const char *str, u16 fg_color, u16 bg_color, GUI_Font_Size size)
{
    u8 sx, sy;
    _font_get_scale(size, &sx, &sy);
    gui_draw_str_scaled(x, y, str, fg_color, bg_color, true, sx, sy);
}

/*
 *@ brief    : Format and draw an unsigned 32-bit decimal number at (x, y).
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; num: unsigned value; width: minimum field width (0 = auto);
 *             zero_pad: pad with '0' if true, ' ' if false; color: RGB565 color; size: font size.
 *@ argu out : Formatted number drawn.
 *@ theme    : Lightweight integer rendering without sprintf overhead.
 */
void gui_draw_uint_at(s16 x, s16 y, u32 num, u8 width, bool zero_pad, u16 color,
                      GUI_Font_Size size)
{
    char buf[12];
    u8 pos = 11U;
    u8 digits = 0U;
    char pad = zero_pad ? '0' : ' ';

    buf[pos] = '\0';
    do
    {
        buf[--pos] = (char)('0' + (num % 10UL));
        num /= 10UL;
        digits++;
    } while (num != 0UL && pos > 0U);

    if (width > 10U)
    {
        width = 10U;
    }
    while (digits < width && pos > 0U)
    {
        buf[--pos] = pad;
        digits++;
    }

    gui_draw_str_at(x, y, &buf[pos], color, size);
}

/*
 *@ brief    : Draw a signed 32-bit integer at (x, y) with transparent background.
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; num: signed integer; color: RGB565 color; size: font size.
 *@ argu out : Signed integer drawn.
 *@ theme    : Lightweight signed integer display.
 */
void gui_draw_int_at(s16 x, s16 y, s32 num, u16 color, GUI_Font_Size size)
{
    u32 mag;

    if (num < 0L)
    {
        gui_draw_char_at(x, y, '-', color, size);
        x = (s16)(x + (s16)gui_font_get_char_width(size));
        mag = (u32)(-(num + 1L)) + 1UL;
    }
    else
    {
        mag = (u32)num;
    }
    gui_draw_uint_at(x, y, mag, 0U, false, color, size);
}

/*
 *@ brief    : Draw a signed 32-bit integer at (x, y) with solid background.
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; num: signed integer; fg_color, bg_color: RGB565 colors; size: font size.
 *@ argu out : Signed integer drawn with background fill.
 *@ theme    : Flicker-free numeric counter display.
 */
void gui_draw_int_bg(s16 x, s16 y, s32 num, u16 fg_color, u16 bg_color, GUI_Font_Size size)
{
    char buf[12];
    u8 pos = 11U;
    u32 mag;

    buf[pos] = '\0';
    if (num < 0L)
    {
        mag = (u32)(-(num + 1L)) + 1UL;
    }
    else
    {
        mag = (u32)num;
    }

    do
    {
        buf[--pos] = (char)('0' + (mag % 10UL));
        mag /= 10UL;
    } while (mag != 0UL && pos > 1U);

    if (num < 0L)
    {
        buf[--pos] = '-';
    }

    gui_draw_str_bg(x, y, &buf[pos], fg_color, bg_color, size);
}

/*
 *@ brief    : Draw an uppercase hexadecimal value (with "0x" prefix) at (x, y).
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; val: value; digits: hex digits (1..8); color: RGB565; size: font size.
 *@ argu out : Hex string drawn.
 *@ theme    : Useful for register/debug inspection on LCD.
 */
void gui_draw_hex_at(s16 x, s16 y, u32 val, u8 digits, u16 color, GUI_Font_Size size)
{
    static const char hex_lut[] = "0123456789ABCDEF";
    char buf[11];
    u8 i;

    if (digits == 0U || digits > 8U)
    {
        digits = 4U;
    }
    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0U; i < digits; ++i)
    {
        u8 shift = (u8)((digits - 1U - i) * 4U);
        buf[2U + i] = hex_lut[(val >> shift) & 0x0FUL];
    }
    buf[2U + digits] = '\0';

    gui_draw_str_at(x, y, buf, color, size);
}

/*
 *@ brief    : Legacy compatible cursor setter (gui_set_position).
 *@ type     : public
 *@ argu in  : x, y: cursor coordinates.
 *@ argu out : ST7735_SUCCESS or ST7735_ERROR.
 *@ theme    : Preserve compatibility with DEMO_bitzee_aquarium_asia gui_set_position.
 */
char gui_set_position(u8 x, u8 y)
{
    if (x > MAX_X && y > MAX_Y)
    {
        return ST7735_ERROR;
    }
    if (x > MAX_X)
    {
        font_cursor_y = y;
        font_cursor_x = 2U;
    }
    else
    {
        font_cursor_x = x;
        font_cursor_y = y;
    }
    return ST7735_SUCCESS;
}

/*
 *@ brief    : Legacy compatible character drawer at current cursor (X1, X2, X3).
 *@ type     : public
 *@ argu in  : character: ASCII char; color: RGB565 color; size: enum Size (X1, X2, X3).
 *@ argu out : ST7735_SUCCESS or 0 if out of range.
 *@ theme    : Preserve compatibility with DEMO_bitzee_aquarium_asia gui_draw_character.
 */
char gui_draw_character(char character, u16 color, enum Size size)
{
    GUI_Font_Size fsize;

    if ((u8)character < 0x20U || (u8)character > 0x7FU)
    {
        return 0;
    }

    if (size == X2)
    {
        fsize = GUI_Font_6x16;
    }
    else if (size == X3)
    {
        fsize = GUI_Font_12x16;
    }
    else
    {
        fsize = GUI_Font_6x8;
    }

    gui_draw_char_at((s16)font_cursor_x, (s16)font_cursor_y, character, color, fsize);
    font_cursor_x = (u16)(font_cursor_x + gui_font_get_char_width(fsize));
    return ST7735_SUCCESS;
}

/*
 *@ brief    : Legacy compatible string drawer at current cursor (X1, X2, X3).
 *@ type     : public
 *@ argu in  : str: null-terminated string; color: RGB565 color; size: enum Size (X1, X2, X3).
 *@ argu out : String drawn and cursor updated.
 *@ theme    : Preserve compatibility with DEMO_bitzee_aquarium_asia gui_draw_string.
 */
void gui_draw_string(const char *str, u16 color, enum Size size)
{
    u16 char_w, char_h;
    GUI_Font_Size fsize;

    if (str == null)
    {
        return;
    }

    if (size == X2)
    {
        fsize = GUI_Font_6x16;
    }
    else if (size == X3)
    {
        fsize = GUI_Font_12x16;
    }
    else
    {
        fsize = GUI_Font_6x8;
    }

    char_w = gui_font_get_char_width(fsize);
    char_h = gui_font_get_char_height(fsize);

    while (*str != '\0')
    {
        if (font_cursor_x + char_w > MAX_X)
        {
            font_cursor_x = 2U;
            font_cursor_y = (u16)(font_cursor_y + char_h);
        }
        if (font_cursor_y + char_h > MAX_Y)
        {
            break;
        }
        gui_draw_character(*str++, color, size);
    }
}
