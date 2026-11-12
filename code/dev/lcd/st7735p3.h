#ifndef _ST7735P3_H__
#define _ST7735P3_H__

/*
 * ST7735P3 (HFB0177ATFT 1.77" 128x160 TFT, 4-line SPI) controller-specific definitions:
 * - A0 (D/C)       : IOB8  (0 = Command, 1 = Parameter/Data)
 * - RESET          : IOB9  (Active Low)
 * - SPI_CS         : IOB12 (Active Low)
 * - SPI_CLK / SCK  : IOB13 (SPI2 Port B SCK)
 * - SPI_MOSI / SDA : IOB15 (SPI2 Port B MOSI)
 */
// clang-format off
/*
 * ST7735P3 (HFB0177ATFT 1.77" 128x160 TFT, 4-line SPI) display orientation & MADCTL (0x36) macros:
 * - Bit 7 (MY) : Row Address Order
 * - Bit 6 (MX) : Column Address Order
 * - Bit 5 (MV) : Row/Column Exchange (0 = Portrait 128x160, 1 = Landscape 160x128)
 * - Bit 3 (RGB): RGB/BGR Order (0 = RGB, 1 = BGR)
 */
#define ST7735P3_MADCTL_0       0xC0U   /* 0 deg   : Portrait          (128x160) */
#define ST7735P3_MADCTL_90      0xA0U   /* 90 deg  : Landscape         (160x128) */
#define ST7735P3_MADCTL_180     0x00U   /* 180 deg : Portrait Inverted (128x160) */
#define ST7735P3_MADCTL_270     0x60U   /* 270 deg : Landscape Flipped (160x128) */

/* Select default display orientation here (one of ST7735P3_MADCTL_0 / 90 / 180 / 270) */
#define ST7735P3_MADCTL_DEFAULT ST7735P3_MADCTL_90

#if (ST7735P3_MADCTL_DEFAULT & 0x20U)
#define ST7735P3_WIDTH          160U
#define ST7735P3_HEIGHT         128U
#else
#define ST7735P3_WIDTH          128U
#define ST7735P3_HEIGHT         160U
#endif
#define ST7735P3_X_OFFSET       0U
#define ST7735P3_Y_OFFSET       0U

#define ST7735_SUCCESS          0
#define ST7735_ERROR            1

/* ST7735P3 active command definitions */
#define LCD_CMD_SWRESET         0x01U
#define LCD_CMD_SLPIN           0x10U
#define LCD_CMD_SLPOUT          0x11U
#define LCD_CMD_INVOFF          0x20U
#define LCD_CMD_INVON           0x21U
#define LCD_CMD_DISPOFF         0x28U
#define LCD_CMD_DISPON          0x29U
#define LCD_CMD_CASET           0x2AU
#define LCD_CMD_RASET           0x2BU
#define LCD_CMD_RAMWR           0x2CU
#define LCD_CMD_TEON            0x35U
#define LCD_CMD_MADCTL          0x36U
#define LCD_CMD_COLMOD          0x3AU
#define LCD_CMD_FRMCTR1         0xB1U
#define LCD_CMD_FRMCTR2         0xB2U
#define LCD_CMD_FRMCTR3         0xB3U
#define LCD_CMD_INVCTR          0xB4U
#define LCD_CMD_PWCTR1          0xC0U
#define LCD_CMD_PWCTR2          0xC1U
#define LCD_CMD_PWCTR3          0xC2U
#define LCD_CMD_PWCTR4          0xC3U
#define LCD_CMD_PWCTR5          0xC4U
#define LCD_CMD_VMCTR1          0xC5U
#define LCD_CMD_GMCTRP1         0xE0U
#define LCD_CMD_GMCTRN1         0xE1U
// clang-format on

typedef union
{
    u16 word;
    struct
    {
        unsigned te_flag : 1;
        unsigned ready : 1;
    } bits;
} ST7735_Msg_Bit;

typedef enum
{
    ST7735_CMD_E = 0x0,
    ST7735_DATA_E = 0x1,
} st7735_data_format_e;

typedef struct
{
    u8 cmd;
    u8 len;
    u8 delay_ms;
    u8 data[16];
} ST7735_Init_Cmd;

void st7735_init_interface(void);
void st7735_reset_chip(void);
void st7735_init_device(void);
void st7735_set_data_format(st7735_data_format_e format);
void st7735_set_cs_level(bool level);
void st7735_set_rst_level(bool level);
void send_st7735_command(u8 cmd);
void send_st7735_data_u8(u8 data);
void send_st7735_data_u16(u16 data);
void send_st7735_buf_u8(const u8 *buf, u16 count);
void send_st7735_buf_u16(const u16 *buf, u32 count);
void send_st7735_repeat_u16(u16 data, u32 count);
void st7735_set_backlight(u16 percent);
bool st7735_take_te_flag(void);
void st7735_te_irq_callback(void);

#endif /* _ST7735P3_H__ */



