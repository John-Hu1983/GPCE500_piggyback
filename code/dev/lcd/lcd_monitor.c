#include "..\..\Include\spinmaster.h"

/* Default ST7735P3 panel driver bound via low-level hardware function pointers */
static const LCD_Device lcd_st7735p3_dev = {
    ST7735P3_WIDTH,       ST7735P3_HEIGHT,    ST7735P3_X_OFFSET,   ST7735P3_Y_OFFSET,
    st7735_init_device,   st7735_reset_chip,  send_st7735_command, send_st7735_data_u8,
    send_st7735_data_u16, send_st7735_buf_u8, send_st7735_buf_u16, send_st7735_repeat_u16,
    st7735_set_backlight, st7735_take_te_flag};

static const LCD_Device *lcd_active_dev = &lcd_st7735p3_dev;
static bool lcd_ready = false;

/*
 *@ brief    : Validate and return the currently registered LCD_Device.
 *@ type     : private
 *@ argu in  : require_ready: true to also verify lcd_ready.
 *@ argu out : Non-null pointer to active LCD_Device.
 *@ theme    : Centralize null-device and uninitialized-state checks.
 */
static const LCD_Device *_lcd_check_dev(bool require_ready)
{
    if (lcd_active_dev == null || (require_ready && !lcd_ready))
    {
        bsp_halt_fault(BSP_Fault_LCD_State);
    }
    return lcd_active_dev;
}

/*
 *@ brief    : Bind a custom LCD_Device driver table before or during initialization.
 *@ type     : public
 *@ argu in  : dev: pointer to a populated LCD_Device descriptor.
 *@ argu out : Active LCD device updated.
 *@ theme    : Allow seamless switching to another LCD controller IC.
 */
void lcd_register_device(const LCD_Device *dev)
{
    if (dev == null || dev->init == null || dev->write_cmd == null || dev->write_data_u8 == null ||
        dev->write_data_u16 == null || dev->write_buf_u8 == null || dev->write_buf_u16 == null ||
        dev->write_repeat_u16 == null)
    {
        bsp_halt_fault(BSP_Fault_LCD_Arg);
    }
    lcd_active_dev = dev;
}

/*
 *@ brief    : Get the active LCD_Device descriptor pointer.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Pointer to the registered LCD_Device.
 *@ theme    : Allow GUI or application code to inspect width/height or call driver hooks.
 */
const LCD_Device *lcd_get_device(void) { return _lcd_check_dev(false); }

/*
 *@ brief    : Set the active GRAM drawing window and issue RAMWR (0x2C).
 *@ type     : public
 *@ argu in  : x0, y0: top-left inclusive; x1, y1: bottom-right inclusive.
 *@ argu out : CASET, RASET and RAMWR transmitted through the registered LCD device.
 *@ theme    : Send 4-byte CASET/RASET coordinates in a single CS transaction per command.
 */
void lcd_set_window(u16 x0, u16 y0, u16 x1, u16 y1)
{
    const LCD_Device *dev = _lcd_check_dev(true);
    u16 coords[2];

    if (x0 > x1 || x1 >= dev->width || y0 > y1 || y1 >= dev->height)
    {
        bsp_halt_fault(BSP_Fault_LCD_Arg);
    }

    coords[0] = (u16)(x0 + dev->x_offset);
    coords[1] = (u16)(x1 + dev->x_offset);
    dev->write_cmd(LCD_CMD_CASET);
    dev->write_buf_u16(coords, 2UL);

    coords[0] = (u16)(y0 + dev->y_offset);
    coords[1] = (u16)(y1 + dev->y_offset);
    dev->write_cmd(LCD_CMD_RASET);
    dev->write_buf_u16(coords, 2UL);

    dev->write_cmd(LCD_CMD_RAMWR);
}

/*
 *@ brief    : Fill a rectangular region with a solid RGB565 color.
 *@ type     : public
 *@ argu in  : x0, y0: top-left coordinates; w, h: dimensions; color: RGB565 color.
 *@ argu out : Target GRAM region filled with color under a single CS frame.
 *@ theme    : Set active window and stream RGB565 pixels via dev->write_repeat_u16.
 */
void lcd_fill_rect(u16 x0, u16 y0, u16 w, u16 h, u16 color)
{
    const LCD_Device *dev = _lcd_check_dev(true);

    if (w == 0U || h == 0U || x0 >= dev->width || y0 >= dev->height || w > (dev->width - x0) ||
        h > (dev->height - y0))
    {
        bsp_halt_fault(BSP_Fault_LCD_Arg);
    }

    lcd_set_window(x0, y0, (u16)(x0 + w - 1U), (u16)(y0 + h - 1U));
    dev->write_repeat_u16(color, (u32)w * (u32)h);
}

/*
 *@ brief    : Fill the entire screen with a solid RGB565 color.
 *@ type     : public
 *@ argu in  : color: RGB565 color.
 *@ argu out : Entire screen filled.
 *@ theme    : Full-screen wrapper around lcd_fill_rect using active device dimensions.
 */
void lcd_fill_color(u16 color)
{
    const LCD_Device *dev = _lcd_check_dev(true);
    lcd_fill_rect(0U, 0U, dev->width, dev->height, color);
}

/*
 *@ brief    : Draw an in-bounds RGB565 bitmap to GRAM.
 *@ type     : public
 *@ argu in  : x, y: top-left coordinates; w, h: dimensions; bitmap: RGB565 pixel array.
 *@ argu out : Bitmap rendered to GRAM under a single CS frame.
 *@ theme    : Set active window and stream pixel array via dev->write_buf_u16.
 */
void lcd_draw_bitmap(u16 x, u16 y, u16 w, u16 h, const u16 *bitmap)
{
    const LCD_Device *dev = _lcd_check_dev(true);

    if (bitmap == null || w == 0U || h == 0U || x >= dev->width || y >= dev->height ||
        w > (dev->width - x) || h > (dev->height - y))
    {
        bsp_halt_fault(BSP_Fault_LCD_Arg);
    }

    lcd_set_window(x, y, (u16)(x + w - 1U), (u16)(y + h - 1U));
    dev->write_buf_u16(bitmap, (u32)w * (u32)h);
}

/*
 *@ brief    : Set LCD backlight brightness percentage through the registered LCD device.
 *@ type     : public
 *@ argu in  : percent: 0..100 brightness percentage.
 *@ argu out : Backlight updated via LCD_Device->set_backlight.
 *@ theme    : Dispatch backlight control to the underlying panel driver.
 */
void lcd_set_backlight(u16 percent)
{
    const LCD_Device *dev = _lcd_check_dev(false);
    if (dev->set_backlight != null)
    {
        dev->set_backlight(percent);
    }
}

/*
 *@ brief    : Atomically read and clear the TE rising-edge flag through the registered LCD device.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if a TE rising edge occurred since the last check.
 *@ theme    : Dispatch TE synchronization check to the underlying panel driver.
 */
bool lcd_take_te_flag(void)
{
    const LCD_Device *dev = _lcd_check_dev(false);
    if (dev->take_te_flag != null)
    {
        return dev->take_te_flag();
    }
    return false;
}

/*
 *@ brief    : Initialize the active LCD_Device, show welcome screen, and turn on backlight.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Active LCD initialized, "wellcom spinmaster" displayed, and backlight set to 65%.
 *@ theme    : Panel-agnostic monitor startup entry called from bsp_init_global.
 */
void lcd_init_module(void)
{
    const LCD_Device *dev = _lcd_check_dev(false);

    dev->init();
    lcd_ready = true;

    lcd_fill_color(LCD_COLOR_BLACK);
    gui_draw_str_at(10, 60, "wellcom", LCD_COLOR_WHITE, GUI_Font_12x16);
    gui_draw_str_at(4, 84, "spinmaster", LCD_COLOR_YELLOW, GUI_Font_12x16);
    lcd_set_backlight(LCD_BL_DEFAULT_PERCENT);
}
