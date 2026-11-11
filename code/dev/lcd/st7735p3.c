#include "..\Include\spinmaster.h"

const gpio_pin_s LCD_GPIO[] = {
    LCD_CS_PIN,
    LCD_A0_PIN,
    LCD_RESET_PIN,
};
lcd_st7735_dev lcd_equip;

static volatile LCD_Msg_Bit lcd_msg_bit = {0};

static void delay_us(unsigned us)
{
    unsigned i, s;
    for (i = 0; i < us; i++)
    {
        for (s = 0; s < 2; s++)
        {
            system_clear_watchdog();
        }
    }
}

static void delay_ms(unsigned ms)
{
    unsigned i;
    for (i = 0; i < ms; i++)
    {
        delay_us(1000);
    }
}

void ST7735_DelayMs(uint8_t time)
{
    while (time--)
    {
        delay_ms(1);
    }
}

void st7735_init_interface(void)
{
    lcd_equip.cs = (gpio_pin_s *)&LCD_GPIO[0];
    lcd_equip.dat_cmd = (gpio_pin_s *)&LCD_GPIO[1];
    lcd_equip.reset = (gpio_pin_s *)&LCD_GPIO[2];

    gpio_init_pattern(lcd_equip.cs->port, lcd_equip.cs->pin, GPIO_Out_High);
    gpio_init_pattern(lcd_equip.dat_cmd->port, lcd_equip.dat_cmd->pin, GPIO_Out_High);
    gpio_init_pattern(lcd_equip.reset->port, lcd_equip.reset->pin, GPIO_Out_High);

    spi_init_port();
}

void st7735_set_data_format(st7735_data_format_e format)
{
    gpio_write_level(lcd_equip.dat_cmd->port, lcd_equip.dat_cmd->pin, format);
}

void st7735_set_cs_level(bool level)
{
    gpio_write_level(lcd_equip.cs->port, lcd_equip.cs->pin, level);
}

void st7735_set_rst_level(bool level)
{
    gpio_write_level(lcd_equip.reset->port, lcd_equip.reset->pin, level);
}

void st7735_reset_chip(void)
{
    st7735_set_rst_level(0);
    delay_ms(100);
    st7735_set_rst_level(1);
    delay_ms(100);
}

void send_st7735_command(u8 cmd)
{
    st7735_set_cs_level(0);
    st7735_set_data_format(ST7735_CMD_E);
    *P_SPI2_TX_Data = cmd;
    SPI_SEND_BUSY;
    *P_SPI2_Misc = 0;
    st7735_set_cs_level(1);
}

void send_st7735_data_u8(u8 data)
{
    st7735_set_cs_level(0);
    st7735_set_data_format(ST7735_DATA_E);
    *P_SPI2_TX_Data = data;
    SPI_SEND_BUSY;
    *P_SPI2_Misc = 0;
    st7735_set_cs_level(1);
}

void send_st7735_data_u16(u16 data)
{
    u8 h, l;
    st7735_set_cs_level(0);
    st7735_set_data_format(ST7735_DATA_E);
    h = data >> 8;
    l = data & 0xff;
    *P_SPI2_TX_Data = h;
    *P_SPI2_TX_Data = l;
    SPI_SEND_BUSY;
    *P_SPI2_Misc = 0;
    st7735_set_cs_level(1);
}

void ST7735_RAM_Content_Show(void) { send_st7735_command(DISPON); }

void ST7735_RAM_Content_Hide(void) { send_st7735_command(DISPOFF); }

void st7735_init_device(void)
{
    st7735_init_interface();

    st7735_reset_chip();

    send_st7735_command(SWRESET);
    ST7735_DelayMs(50);

    send_st7735_command(SLPOUT);
    ST7735_DelayMs(120);

    send_st7735_command(0xF0);
    send_st7735_data_u8(0x11);

    send_st7735_command(0xD6);
    send_st7735_data_u8(0xCB);

    send_st7735_command(FRMCTR1);
    send_st7735_data_u8(0x01);
    send_st7735_data_u8(0x04);
    send_st7735_data_u8(0x01);

    send_st7735_command(FRMCTR2);
    send_st7735_data_u8(0x05);
    send_st7735_data_u8(0x3C);
    send_st7735_data_u8(0x3C);

    send_st7735_command(FRMCTR3);
    send_st7735_data_u8(0x05);
    send_st7735_data_u8(0x3C);
    send_st7735_data_u8(0x3C);
    send_st7735_data_u8(0x05);
    send_st7735_data_u8(0x3C);
    send_st7735_data_u8(0x3C);

    send_st7735_command(INVCTR);
    send_st7735_data_u8(0x03);
    send_st7735_data_u8(0x02);

    send_st7735_command(PWCTR1);
    send_st7735_data_u8(0xA9);
    send_st7735_data_u8(0x09);
    send_st7735_data_u8(0x84);

    send_st7735_command(PWCTR2);
    send_st7735_data_u8(0xC4);

    send_st7735_command(PWCTR3);
    send_st7735_data_u8(0x0D);
    send_st7735_data_u8(0x00);

    send_st7735_command(PWCTR4);
    send_st7735_data_u8(0x8D);
    send_st7735_data_u8(0x2A);

    send_st7735_command(PWCTR5);
    send_st7735_data_u8(0x8D);
    send_st7735_data_u8(0xEE);

    send_st7735_command(VMCTR1);
    send_st7735_data_u8(0x07);

    send_st7735_command(GMCTRP1);
    send_st7735_data_u8(0x16);
    send_st7735_data_u8(0x1F);
    send_st7735_data_u8(0x16);
    send_st7735_data_u8(0x1A);
    send_st7735_data_u8(0x33);
    send_st7735_data_u8(0x2D);
    send_st7735_data_u8(0x26);
    send_st7735_data_u8(0x28);
    send_st7735_data_u8(0x27);
    send_st7735_data_u8(0x22);
    send_st7735_data_u8(0x2B);
    send_st7735_data_u8(0x34);
    send_st7735_data_u8(0x00);
    send_st7735_data_u8(0x11);
    send_st7735_data_u8(0x00);
    send_st7735_data_u8(0x0F);

    send_st7735_command(GMCTRN1);
    send_st7735_data_u8(0x0D);
    send_st7735_data_u8(0x1A);
    send_st7735_data_u8(0x18);
    send_st7735_data_u8(0x19);
    send_st7735_data_u8(0x2F);
    send_st7735_data_u8(0x28);
    send_st7735_data_u8(0x22);
    send_st7735_data_u8(0x26);
    send_st7735_data_u8(0x26);
    send_st7735_data_u8(0x26);
    send_st7735_data_u8(0x2D);
    send_st7735_data_u8(0x3E);
    send_st7735_data_u8(0x00);
    send_st7735_data_u8(0x08);
    send_st7735_data_u8(0x02);
    send_st7735_data_u8(0x0F);

    send_st7735_command(TEON);
    send_st7735_data_u8(0x00);

    send_st7735_command(COLMOD);
    send_st7735_data_u8(0x05);
    ST7735_DelayMs(5);

    send_st7735_command(MADCTL);
    send_st7735_data_u8(0xC0);
    ST7735_DelayMs(5);

    send_st7735_command(INVON);

    send_st7735_command(DISPON);
    ST7735_DelayMs(5);
}

uint8_t gui_set_window(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
#define ST7735_X_OFFSET (0)
    if ((x0 > x1) || (x1 > SIZE_X) || (y0 > y1) || (y1 > SIZE_Y))
    {
        return ST7735_ERROR;
    }

    send_st7735_command(CASET);
    send_st7735_data_u16(0x0000 | (x0 + ST7735_X_OFFSET));
    send_st7735_data_u16(0x0000 | (x1 + ST7735_X_OFFSET));
    send_st7735_command(RASET);
    send_st7735_data_u16(0x0000 | y0);
    send_st7735_data_u16(0x0000 | y1);
    return ST7735_SUCCESS;
}

void gui_send_rgb565(uint16_t color, uint16_t count)
{
    send_st7735_command(RAMWR);
    while (count--)
    {
        send_st7735_data_u16(color);
    }
}

void gui_draw_pixel(uint8_t x, uint8_t y, uint16_t color)
{
    gui_set_window(x, x, y, y);
    gui_send_rgb565(color, 1);
}

void gui_clear_screen(uint16_t color)
{
    u8 h, l;
    u16 i;
    gui_set_window(0, SIZE_X, 0, SIZE_Y);
    h = color >> 8;
    l = color & 0xff;
    send_st7735_command(RAMWR);
    st7735_set_cs_level(0);
    st7735_set_data_format(ST7735_DATA_E);
    for (i = 0; i < CACHE_SIZE_MEM; i++)
    {
        *P_SPI2_TX_Data = h;
        *P_SPI2_TX_Data = l;
        while ((*P_SPI2_TX_Status & 0xf) > 6)
            ;
    }
    SPI_SEND_BUSY;
    *P_SPI2_Misc = 0;
    st7735_set_cs_level(1);
}

void gui_draw_rectangle(uint8_t xs, uint8_t xe, uint8_t ys, uint8_t ye, uint16_t color)
{
    uint8_t temp;
    if (xs > xe)
    {
        temp = xe;
        xe = xs;
        xs = temp;
    }
    if (ys > ye)
    {
        temp = ye;
        ye = ys;
        ys = temp;
    }
    gui_set_window(xs, xe, ys, ye);
    gui_send_rgb565(color, (xe - xs + 1) * (ye - ys + 1));
}

void lcd_write_cmd(u16 cmd) { send_st7735_command((u8)cmd); }

void lcd_write_data(u16 data) { send_st7735_data_u8((u8)data); }

void lcd_fill_color(u16 color) { gui_clear_screen(color); }

void lcd_set_backlight(u16 percent)
{
    unsigned duty;

    if (percent > 100U)
    {
        bsp_halt_fault(BSP_Fault_LCD_Arg);
    }
    if (percent == 0U)
    {
        iopwm_set_duty(LCD_BL_PWM_CH, 0U);
        iopwm_disable_channel(LCD_BL_PWM_CH);
        return;
    }

    duty = (unsigned)(((u32)percent * 256UL + 50UL) / 100UL);
    iopwm_set_duty(LCD_BL_PWM_CH, duty);
    iopwm_enable_channel(LCD_BL_PWM_CH);
}

bool lcd_take_te_flag(void)
{
    if (!lcd_msg_bit.bits.te_flag)
    {
        return false;
    }
    lcd_msg_bit.bits.te_flag = 0;
    return true;
}

void lcd_te_irq_callback(void) { lcd_msg_bit.bits.te_flag = 1; }

void lcd_init_module(void)
{
    iopwm_init_channel(LCD_BL_PWM_CH);

    lcd_msg_bit.bits.te_flag = 0;
    exisr_init_channel(EXISR_Ch_2, EXISR_Edge_Rising, lcd_te_irq_callback);

    st7735_init_device();

    exisr_enable_channel(EXISR_Ch_2);
    lcd_msg_bit.bits.ready = 1;
    gui_clear_screen(LCD_COLOR_RED);
    lcd_set_backlight(LCD_BL_DEFAULT_PERCENT);
}

