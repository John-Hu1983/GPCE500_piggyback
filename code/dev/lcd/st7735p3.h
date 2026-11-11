#ifndef _ST7735P3_H__
#define _ST7735P3_H__

// Success / Error
// -----------------------------------
#define ST7735_SUCCESS 0
#define ST7735_ERROR 1

// Command definition
// -----------------------------------
#define DELAY 0x80

#define NOP 0x00
#define SWRESET 0x01
#define RDDID 0x04
#define RDDST 0x09

#define SLPIN 0x10
#define SLPOUT 0x11
#define PTLON 0x12
#define NORON 0x13

#define INVOFF 0x20
#define INVON 0x21
#define DISPOFF 0x28
#define DISPON 0x29
#define RAMRD 0x2E
#define CASET 0x2A
#define RASET 0x2B
#define RAMWR 0x2C

#define PTLAR 0x30
#define TEON 0x35
#define MADCTL 0x36
#define COLMOD 0x3A

#define FRMCTR1 0xB1
#define FRMCTR2 0xB2
#define FRMCTR3 0xB3
#define INVCTR 0xB4
#define DISSET5 0xB6

#define PWCTR1 0xC0
#define PWCTR2 0xC1
#define PWCTR3 0xC2
#define PWCTR4 0xC3
#define PWCTR5 0xC4
#define VMCTR1 0xC5

#define RDID1 0xDA
#define RDID2 0xDB
#define RDID3 0xDC
#define RDID4 0xDD

#define GMCTRP1 0xE0
#define GMCTRN1 0xE1

#define PWCTR6 0xFC

// Colors
// -----------------------------------
#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 0xF800

//clang-format off
/* RGB565 standard colors */
#define LCD_COLOR_BLACK     0x0000U
#define LCD_COLOR_WHITE     0xFFFFU
#define LCD_COLOR_RED       0xF800U
#define LCD_COLOR_GREEN     0x07E0U
#define LCD_COLOR_BLUE      0x001FU
#define LCD_COLOR_YELLOW    0xFFE0U
#define LCD_COLOR_CYAN      0x07FFU
#define LCD_COLOR_MAGENTA   0xF81FU
//clang-format on

#ifndef LCD_BL_DEFAULT_PERCENT
#define LCD_BL_DEFAULT_PERCENT 65U
#endif

// AREA definition
// -----------------------------------
#define MAX_X 128                      // max columns
#define MAX_Y 160                      // max rows
#define SIZE_X (MAX_X - 1)             // columns max counter
#define SIZE_Y (MAX_Y - 1)             // rows max counter
#define CACHE_SIZE_MEM (MAX_X * MAX_Y) // whole pixels
#define CHARS_COLS_LEN 5               // number of columns for chars
#define CHARS_ROWS_LEN 8               // number of rows for chars

#define LCD_WIDTH MAX_X
#define LCD_HEIGHT MAX_Y

typedef struct
{
    gpio_pin_s *cs;
    gpio_pin_s *dat_cmd;
    gpio_pin_s *reset;
} lcd_st7735_dev;

typedef enum
{
    ST7735_CMD_E = 0x0,
    ST7735_DATA_E = 0x1,
} st7735_data_format_e;

typedef union
{
    u16 word;
    struct
    {
        unsigned te_flag : 1;
        unsigned ready : 1;
    } bits;
} LCD_Msg_Bit;

void st7735_init_interface(void);
void send_st7735_command(u8 cmd);
void send_st7735_data_u8(u8 data);
void send_st7735_data_u16(u16 data);
void st7735_set_data_format(st7735_data_format_e format);
void st7735_set_cs_level(bool level);
void st7735_set_rst_level(bool level);
void st7735_reset_chip(void);
void st7735_init_device(void);

uint8_t gui_set_window(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1);
void gui_send_rgb565(uint16_t color, uint16_t count);
void gui_draw_pixel(uint8_t x, uint8_t y, uint16_t color);
void gui_clear_screen(uint16_t color);
void gui_draw_rectangle(uint8_t xs, uint8_t xe, uint8_t ys, uint8_t ye, uint16_t color);

void lcd_init_module(void);
void lcd_write_cmd(u16 cmd);
void lcd_write_data(u16 data);
void lcd_fill_color(u16 color);
void lcd_set_backlight(u16 percent);
bool lcd_take_te_flag(void);
void lcd_te_irq_callback(void);

#endif /* _ST7735P3_H__ */

