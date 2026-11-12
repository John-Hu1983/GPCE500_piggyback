#ifndef _GUI_FONT_H__
#define _GUI_FONT_H__

// clang-format off
#define CHARS_COLS_LENGTH   5U
#define CHARS_COLS_LEN      5U
#define CHARS_ROWS_LEN      8U
// clang-format on

/* Legacy font size enum compatible with DEMO_bitzee_aquarium_asia */
enum Size
{
    X1 = 0x00, /* 1x width, 1x height (6x8 cell) */
    X2 = 0x80, /* 1x width, 2x height (6x16 cell) */
    X3 = 0x81  /* 2x width, 2x height (12x16 cell) */
};

/* Extended multi-size font selector */
typedef enum
{
    GUI_Font_6x8 = 0,   /* 1x width, 1x height (5x8 glyph, 6x8 cell) */
    GUI_Font_6x16 = 1,  /* 1x width, 2x height (5x16 glyph, 6x16 cell) */
    GUI_Font_12x16 = 2, /* 2x width, 2x height (10x16 glyph, 12x16 cell) */
    GUI_Font_18x24 = 3, /* 3x width, 3x height (15x24 glyph, 18x24 cell) */
    GUI_Font_24x32 = 4  /* 4x width, 4x height (20x32 glyph, 24x32 cell) */
} GUI_Font_Size;

/* Extensible bitmap font descriptor */
typedef struct
{
    u8 width;      /* Glyph bitmap columns (e.g., 5) */
    u8 height;     /* Glyph bitmap rows (e.g., 8) */
    u8 spacing_x;  /* Inter-character horizontal spacing in base pixels (e.g., 1) */
    u8 spacing_y;  /* Inter-line vertical spacing in base pixels (e.g., 1) */
    u8 first_char; /* First supported ASCII code (0x20) */
    u8 last_char;  /* Last supported ASCII code (0x7F) */
    const u8 *table;
} GUI_Font_Desc;

extern const unsigned char FONTS[][CHARS_COLS_LENGTH];
extern const GUI_Font_Desc gui_font_5x8;

/* Cursor-based legacy APIs */
char gui_set_position(u8 x, u8 y);
char gui_draw_character(char character, u16 color, enum Size size);
void gui_draw_string(const char *str, u16 color, enum Size size);

/* Coordinate-based multi-size transparent text APIs */
void gui_draw_char_at(s16 x, s16 y, char ch, u16 color, GUI_Font_Size size);
void gui_draw_str_at(s16 x, s16 y, const char *str, u16 color, GUI_Font_Size size);
void gui_draw_int_at(s16 x, s16 y, s32 num, u16 color, GUI_Font_Size size);
void gui_draw_uint_at(s16 x, s16 y, u32 num, u8 width, bool zero_pad, u16 color,
                      GUI_Font_Size size);
void gui_draw_hex_at(s16 x, s16 y, u32 val, u8 digits, u16 color, GUI_Font_Size size);

/* Coordinate-based multi-size opaque (foreground + background) text APIs */
void gui_draw_char_bg(s16 x, s16 y, char ch, u16 fg_color, u16 bg_color, GUI_Font_Size size);
void gui_draw_str_bg(s16 x, s16 y, const char *str, u16 fg_color, u16 bg_color, GUI_Font_Size size);
void gui_draw_int_bg(s16 x, s16 y, s32 num, u16 fg_color, u16 bg_color, GUI_Font_Size size);

/* Arbitrary (scale_x, scale_y) custom font rendering APIs */
void gui_draw_char_scaled(s16 x, s16 y, char ch, u16 fg_color, u16 bg_color, bool has_bg,
                          u8 scale_x, u8 scale_y);
void gui_draw_str_scaled(s16 x, s16 y, const char *str, u16 fg_color, u16 bg_color, bool has_bg,
                         u8 scale_x, u8 scale_y);

/* Font metrics helpers */
u16 gui_font_get_char_width(GUI_Font_Size size);
u16 gui_font_get_char_height(GUI_Font_Size size);
u16 gui_font_get_str_width(const char *str, GUI_Font_Size size);

#endif /* _GUI_FONT_H__ */
