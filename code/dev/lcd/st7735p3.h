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
#define ST7735P3_WIDTH      128U
#define ST7735P3_HEIGHT     160U
#define ST7735P3_X_OFFSET   0U
#define ST7735P3_Y_OFFSET   0U

// clang-format off
#define ST7735_SUCCESS      0
#define ST7735_ERROR        1

/* ST7735P3 command definitions */
#define LCD_CMD_NOP         0x00U
#define LCD_CMD_SWRESET     0x01U
#define LCD_CMD_RDDID       0x04U
#define LCD_CMD_RDDST       0x09U
#define LCD_CMD_SLPIN       0x10U
#define LCD_CMD_SLPOUT      0x11U
#define LCD_CMD_PTLON       0x12U
#define LCD_CMD_NORON       0x13U
#define LCD_CMD_INVOFF      0x20U
#define LCD_CMD_INVON       0x21U
#define LCD_CMD_DISPOFF     0x28U
#define LCD_CMD_DISPON      0x29U
#define LCD_CMD_CASET       0x2AU
#define LCD_CMD_RASET       0x2BU
#define LCD_CMD_RAMWR       0x2CU
#define LCD_CMD_RAMRD       0x2EU
#define LCD_CMD_PTLAR       0x30U
#define LCD_CMD_TEON        0x35U
#define LCD_CMD_MADCTL      0x36U
#define LCD_CMD_COLMOD      0x3AU
#define LCD_CMD_FRMCTR1     0xB1U
#define LCD_CMD_FRMCTR2     0xB2U
#define LCD_CMD_FRMCTR3     0xB3U
#define LCD_CMD_INVCTR      0xB4U
#define LCD_CMD_DISSET5     0xB6U
#define LCD_CMD_PWCTR1      0xC0U
#define LCD_CMD_PWCTR2      0xC1U
#define LCD_CMD_PWCTR3      0xC2U
#define LCD_CMD_PWCTR4      0xC3U
#define LCD_CMD_PWCTR5      0xC4U
#define LCD_CMD_VMCTR1      0xC5U
#define LCD_CMD_RDID1       0xDAU
#define LCD_CMD_RDID2       0xDBU
#define LCD_CMD_RDID3       0xDCU
#define LCD_CMD_RDID4       0xDDU
#define LCD_CMD_GMCTRP1     0xE0U
#define LCD_CMD_GMCTRN1     0xE1U
#define LCD_CMD_PWCTR6      0xFCU

/* Short command aliases */
#define NOP                 LCD_CMD_NOP
#define SWRESET             LCD_CMD_SWRESET
#define SLPOUT              LCD_CMD_SLPOUT
#define INVOFF              LCD_CMD_INVOFF
#define INVON               LCD_CMD_INVON
#define DISPOFF             LCD_CMD_DISPOFF
#define DISPON              LCD_CMD_DISPON
#define CASET               LCD_CMD_CASET
#define RASET               LCD_CMD_RASET
#define RAMWR               LCD_CMD_RAMWR
#define TEON                LCD_CMD_TEON
#define MADCTL              LCD_CMD_MADCTL
#define COLMOD              LCD_CMD_COLMOD
#define FRMCTR1             LCD_CMD_FRMCTR1
#define FRMCTR2             LCD_CMD_FRMCTR2
#define FRMCTR3             LCD_CMD_FRMCTR3
#define INVCTR              LCD_CMD_INVCTR
#define PWCTR1              LCD_CMD_PWCTR1
#define PWCTR2              LCD_CMD_PWCTR2
#define PWCTR3              LCD_CMD_PWCTR3
#define PWCTR4              LCD_CMD_PWCTR4
#define PWCTR5              LCD_CMD_PWCTR5
#define VMCTR1              LCD_CMD_VMCTR1
#define GMCTRP1             LCD_CMD_GMCTRP1
#define GMCTRN1             LCD_CMD_GMCTRN1
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
void st7735_set_backlight(u16 percent);
bool st7735_take_te_flag(void);
void st7735_te_irq_callback(void);

#endif /* _ST7735P3_H__ */



