#ifndef _LCD_MONITOR_H__
#define _LCD_MONITOR_H__

/*
 * Generic LCD monitor abstraction layer:
 * - Decouples upper-layer GUI / application code from specific panel controllers (ST7735P3, etc.).
 * - Binds panel driver operations into a function-pointer device structure (LCD_Device).
 * - Owns panel-agnostic TE synchronization (IOB10 / EXT2) and backlight PWM control (IOA8).
 */
// clang-format off
#define LCD_WIDTH               ST7735P3_WIDTH
#define LCD_HEIGHT              ST7735P3_HEIGHT
#define LCD_X_OFFSET            ST7735P3_X_OFFSET
#define LCD_Y_OFFSET            ST7735P3_Y_OFFSET

#define LCD_BL_DEFAULT_PERCENT  65U

#define MAX_X               LCD_WIDTH
#define MAX_Y               LCD_HEIGHT
#define SIZE_X              (MAX_X - 1U)
#define SIZE_Y              (MAX_Y - 1U)
#define CACHE_SIZE_MEM      (MAX_X * MAX_Y)

/* RGB565 standard colors */
#define LCD_COLOR_BLACK     0x0000U
#define LCD_COLOR_WHITE     0xFFFFU
#define LCD_COLOR_RED       0xF800U
#define LCD_COLOR_GREEN     0x07E0U
#define LCD_COLOR_BLUE      0x001FU
#define LCD_COLOR_YELLOW    0xFFE0U
#define LCD_COLOR_CYAN      0x07FFU
#define LCD_COLOR_MAGENTA   0xF81FU

#define BLACK               LCD_COLOR_BLACK
#define WHITE               LCD_COLOR_WHITE
#define RED                 LCD_COLOR_RED
// clang-format on

typedef struct
{
    u16 width;
    u16 height;
    u16 x_offset;
    u16 y_offset;
    void (*init)(void);
    void (*reset)(void);
    void (*write_cmd)(u8 cmd);
    void (*write_data_u8)(u8 data);
    void (*write_data_u16)(u16 data);
    void (*write_buf_u8)(const u8 *buf, u16 count);
    void (*write_buf_u16)(const u16 *buf, u32 count);
    void (*write_repeat_u16)(u16 data, u32 count);
    void (*set_backlight)(u16 percent);
    bool (*take_te_flag)(void);
} LCD_Device;

void lcd_register_device(const LCD_Device *dev);
const LCD_Device *lcd_get_device(void);

void lcd_init_module(void);
void lcd_set_window(u16 x0, u16 y0, u16 x1, u16 y1);
void lcd_fill_rect(u16 x0, u16 y0, u16 w, u16 h, u16 color);
void lcd_fill_color(u16 color);
void lcd_draw_bitmap(u16 x, u16 y, u16 w, u16 h, const u16 *bitmap);
void lcd_set_backlight(u16 percent);
bool lcd_take_te_flag(void);

#endif /* _LCD_MONITOR_H__ */
