#include "..\..\Include\spinmaster.h"

const gpio_pin_s Lcd_Cs_Pin = LCD_CS_PIN;
const gpio_pin_s Lcd_A0_Pin = LCD_A0_PIN;
const gpio_pin_s Lcd_Reset_Pin = LCD_RESET_PIN;

static volatile ST7735_Msg_Bit st7735_msg_bit = {0};

/* Supplier initialization sequence: ST7735P3+BOE1.77(BT018QQQ)_2.2Gamma_V1.2 */
static const ST7735_Init_Cmd st7735_init_table[] = {
    {LCD_CMD_SWRESET, 0U, 50U, {0}},
    {LCD_CMD_SLPOUT, 0U, 120U, {0}},
    {0xF0U, 1U, 0U, {0x11U}},
    {0xD6U, 1U, 0U, {0xCBU}},
    {LCD_CMD_FRMCTR1, 3U, 0U, {0x01U, 0x04U, 0x01U}},
    {LCD_CMD_FRMCTR2, 3U, 0U, {0x05U, 0x3CU, 0x3CU}},
    {LCD_CMD_FRMCTR3, 6U, 0U, {0x05U, 0x3CU, 0x3CU, 0x05U, 0x3CU, 0x3CU}},
    {LCD_CMD_INVCTR, 2U, 0U, {0x03U, 0x02U}},
    {LCD_CMD_PWCTR1, 3U, 0U, {0xA9U, 0x09U, 0x84U}},
    {LCD_CMD_PWCTR2, 1U, 0U, {0xC4U}},
    {LCD_CMD_PWCTR3, 2U, 0U, {0x0DU, 0x00U}},
    {LCD_CMD_PWCTR4, 2U, 0U, {0x8DU, 0x2AU}},
    {LCD_CMD_PWCTR5, 2U, 0U, {0x8DU, 0xEEU}},
    {LCD_CMD_VMCTR1, 1U, 0U, {0x07U}},
    {LCD_CMD_GMCTRP1,
     16U,
     0U,
     {0x16U, 0x1FU, 0x16U, 0x1AU, 0x33U, 0x2DU, 0x26U, 0x28U, 0x27U, 0x22U, 0x2BU, 0x34U, 0x00U,
      0x11U, 0x00U, 0x0FU}},
    {LCD_CMD_GMCTRN1,
     16U,
     0U,
     {0x0DU, 0x1AU, 0x18U, 0x19U, 0x2FU, 0x28U, 0x22U, 0x26U, 0x26U, 0x26U, 0x2DU, 0x3EU, 0x00U,
      0x08U, 0x02U, 0x0FU}},
    {LCD_CMD_CASET, 4U, 0U, {0x00U, 0x00U, 0x00U, 0x7FU}},
    {LCD_CMD_RASET, 4U, 0U, {0x00U, 0x00U, 0x00U, 0x9FU}},
    {LCD_CMD_TEON, 1U, 0U, {0x00U}},
    {LCD_CMD_COLMOD, 1U, 5U, {0x05U}},
    {LCD_CMD_MADCTL, 1U, 5U, {0xC0U}},
    {LCD_CMD_INVON, 0U, 0U, {0}},
    {LCD_CMD_DISPON, 0U, 5U, {0}},
    {LCD_CMD_RAMWR, 0U, 0U, {0}}};

/*
 *@ brief    : Busy-wait delay in microseconds while servicing watchdog.
 *@ type     : private
 *@ argu in  : us: microsecond count.
 *@ argu out : None.
 *@ theme    : Safe before or after CPU interrupts are enabled.
 */
static void _st7735_delay_us(unsigned us)
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

/*
 *@ brief    : Busy-wait delay in milliseconds while servicing watchdog.
 *@ type     : private
 *@ argu in  : ms: millisecond count.
 *@ argu out : None.
 *@ theme    : Used for ST7735P3 hardware reset and sleep-out timings.
 */
static void _st7735_delay_ms(unsigned ms)
{
    unsigned i;

    for (i = 0; i < ms; i++)
    {
        _st7735_delay_us(1000U);
    }
}

/*
 *@ brief    : Drive ST7735P3 CS (IOB12) pin level.
 *@ type     : public
 *@ argu in  : level: false = active low, true = deselected high.
 *@ argu out : None.
 *@ theme    : Direct GPIO wrapper for CS framing.
 */
void st7735_set_cs_level(bool level) { gpio_write_level(Lcd_Cs_Pin.port, Lcd_Cs_Pin.pin, level); }

/*
 *@ brief    : Drive ST7735P3 A0/DC (IOB8) pin level.
 *@ type     : public
 *@ argu in  : format: ST7735_CMD_E (0) for command, ST7735_DATA_E (1) for data.
 *@ argu out : None.
 *@ theme    : Select command or parameter register mode.
 */
void st7735_set_data_format(st7735_data_format_e format)
{
    gpio_write_level(Lcd_A0_Pin.port, Lcd_A0_Pin.pin, format != ST7735_CMD_E);
}

/*
 *@ brief    : Drive ST7735P3 RESET (IOB9) pin level.
 *@ type     : public
 *@ argu in  : level: false = active reset low, true = normal high.
 *@ argu out : None.
 *@ theme    : Direct GPIO wrapper for hardware reset pin.
 */
void st7735_set_rst_level(bool level)
{
    gpio_write_level(Lcd_Reset_Pin.port, Lcd_Reset_Pin.pin, level);
}

/*
 *@ brief    : Send one 8-bit command byte to ST7735P3.
 *@ type     : public
 *@ argu in  : cmd: command byte.
 *@ argu out : Command transmitted with CS=0 and A0=0.
 *@ theme    : Single-byte command transmission over SPI2.
 */
void send_st7735_command(u8 cmd)
{
    st7735_set_cs_level(false);
    st7735_set_data_format(ST7735_CMD_E);
    spi_write_byte(cmd);
    st7735_set_cs_level(true);
}

/*
 *@ brief    : Send one 8-bit parameter/data byte to ST7735P3.
 *@ type     : public
 *@ argu in  : data: 8-bit parameter byte.
 *@ argu out : Byte transmitted with CS=0 and A0=1.
 *@ theme    : Single-byte parameter transmission over SPI2.
 */
void send_st7735_data_u8(u8 data)
{
    st7735_set_cs_level(false);
    st7735_set_data_format(ST7735_DATA_E);
    spi_write_byte(data);
    st7735_set_cs_level(true);
}

/*
 *@ brief    : Send one 16-bit parameter/pixel word (big-endian) to ST7735P3.
 *@ type     : public
 *@ argu in  : data: 16-bit value (high byte sent first, then low byte).
 *@ argu out : Two bytes transmitted under a single CS frame with A0=1.
 *@ theme    : 16-bit coordinate and RGB565 pixel transmission over SPI2.
 */
void send_st7735_data_u16(u16 data)
{
    u8 h = (u8)(data >> 8);
    u8 l = (u8)(data & 0x00FFU);

    st7735_set_cs_level(false);
    st7735_set_data_format(ST7735_DATA_E);
    *P_SPI2_TX_Data = h;
    *P_SPI2_TX_Data = l;
    SPI_SEND_BUSY;
    *P_SPI2_Misc = 0;
    st7735_set_cs_level(true);
}

/*
 *@ brief    : Send a buffer of 8-bit parameter bytes to ST7735P3.
 *@ type     : public
 *@ argu in  : buf: parameter byte array; count: number of bytes.
 *@ argu out : All parameter bytes transmitted.
 *@ theme    : Sequential parameter byte write using send_st7735_data_u8.
 */
void send_st7735_buf_u8(const u8 *buf, u16 count)
{
    u16 i;

    if (buf == null || count == 0U)
    {
        bsp_halt_fault(BSP_Fault_LCD_Arg);
    }
    for (i = 0U; i < count; ++i)
    {
        send_st7735_data_u8(buf[i]);
    }
}

/*
 *@ brief    : Set ST7735P3 backlight brightness percentage on IOA8 (IOPWM_Ch_8).
 *@ type     : public
 *@ argu in  : percent: 0..100 brightness percentage.
 *@ argu out : IOA8 IOPWM duty updated and channel enabled (or disabled when 0%).
 *@ theme    : Map 0..100% linearly onto IOPWM duty 0..256 (65% -> 166/256).
 */
void st7735_set_backlight(u16 percent)
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

/*
 *@ brief    : Atomically read and clear the ST7735P3 TE rising-edge flag.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if a TE rising edge occurred since the last check.
 *@ theme    : Allow tearing-synchronized frame updates in foreground tasks.
 */
bool st7735_take_te_flag(void)
{
    if (!st7735_msg_bit.bits.te_flag)
    {
        return false;
    }
    st7735_msg_bit.bits.te_flag = 0;
    return true;
}

/*
 *@ brief    : TE rising-edge interrupt callback registered with hal_exisr (EXT2 / IOB10).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Latches st7735_msg_bit.bits.te_flag.
 *@ theme    : Invoked by exisr_service_irq when EXT2 triggers on IRQ5.
 */
void st7735_te_irq_callback(void) { st7735_msg_bit.bits.te_flag = 1; }

/*
 *@ brief    : Initialize ST7735P3 GPIO control pins, backlight PWM, TE interrupt, and SPI2 interface.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : CS, A0, RESET initialized high, IOA8 PWM & IOB10 EXT2 configured, SPI2 at Div2.
 *@ theme    : Hardware interface setup for ST7735P3.
 */
void st7735_init_interface(void)
{
    gpio_init_pattern(Lcd_Cs_Pin.port, Lcd_Cs_Pin.pin, GPIO_Out_High);
    gpio_init_pattern(Lcd_A0_Pin.port, Lcd_A0_Pin.pin, GPIO_Out_High);
    gpio_init_pattern(Lcd_Reset_Pin.port, Lcd_Reset_Pin.pin, GPIO_Out_High);

    iopwm_init_channel(LCD_BL_PWM_CH);

    st7735_msg_bit.bits.te_flag = 0;
    exisr_init_channel(EXISR_Ch_2, EXISR_Edge_Rising, st7735_te_irq_callback);

    spi_init_serial(SPI_Rate_Div2);
}

/*
 *@ brief    : Toggle IOB9 hardware reset pin and wait for ST7735P3 recovery.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : ST7735P3 hardware reset completed.
 *@ theme    : Follow supplier INI reset pulse and 120ms recovery delay.
 */
void st7735_reset_chip(void)
{
    st7735_set_rst_level(false);
    _st7735_delay_ms(100U);
    st7735_set_rst_level(true);
    _st7735_delay_ms(120U);
}

/*
 *@ brief    : Initialize ST7735P3 controller registers from the compact init table.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : All supplier initialization commands and parameters sent and TE IRQ enabled.
 *@ theme    : Table-driven ST7735P3 initialization.
 */
void st7735_init_device(void)
{
    u16 i;

    st7735_init_interface();
    st7735_reset_chip();

    for (i = 0U; i < sizeof(st7735_init_table) / sizeof(st7735_init_table[0]); ++i)
    {
        send_st7735_command(st7735_init_table[i].cmd);
        if (st7735_init_table[i].len > 0U)
        {
            send_st7735_buf_u8(st7735_init_table[i].data, st7735_init_table[i].len);
        }
        if (st7735_init_table[i].delay_ms > 0U)
        {
            _st7735_delay_ms(st7735_init_table[i].delay_ms);
        }
    }

    exisr_enable_channel(EXISR_Ch_2);
    st7735_msg_bit.bits.ready = 1;
}

