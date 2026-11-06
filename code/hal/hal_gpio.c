#include "hal_gpio.h"

static void gpio_fail(void)
{
    for (;;)
    {
    }
}

static unsigned gpio_pin_mask(handle_t *port, uint8_t pin)
{
    if (port != GPIOA && port != GPIOB && port != GPIOC)
    {
        gpio_fail();
    }
    if (pin >= 16 || (port == GPIOC && pin >= 8))
    {
        gpio_fail();
    }
    return 1U << pin;
}

static unsigned gpio_output_mask(handle_t *port, uint8_t pin)
{
    unsigned mask = gpio_pin_mask(port, pin);

    if ((port == GPIOC && pin < 2) || (port->dir & mask) == 0)
    {
        gpio_fail();
    }
    return mask;
}

void gpio_init_pattern(handle_t *port, uint8_t pin, GPIO_Pattern pattern)
{
    unsigned mask = gpio_pin_mask(port, pin);

    if ((unsigned)pattern > GPIO_Out_Inv_High)
    {
        gpio_fail();
    }
    if (pattern == GPIO_In_PullHigh && (port != GPIOB || pin >= 8))
    {
        gpio_fail();
    }
    if (pattern >= GPIO_Out && port == GPIOC && pin < 2)
    {
        gpio_fail();
    }

    /* Release the driver before changing the latch/attribute; enable output last. */
    port->dir &= ~mask;
    switch (pattern)
    {
    case GPIO_In_Float:
        port->attrib |= mask;
        break;
    case GPIO_In_PullLow:
        port->buffer &= ~mask;
        port->attrib &= ~mask;
        break;
    case GPIO_In_PullHigh:
        port->buffer |= mask;
        port->attrib &= ~mask;
        break;
    case GPIO_Out:
        port->buffer &= ~mask;
        port->attrib |= mask;
        port->dir |= mask;
        break;
    case GPIO_Out_Inv:
        port->buffer |= mask;
        port->attrib &= ~mask;
        port->dir |= mask;
        break;
    case GPIO_Out_High:
        port->buffer |= mask;
        port->attrib |= mask;
        port->dir |= mask;
        break;
    case GPIO_Out_Inv_High:
        port->buffer &= ~mask;
        port->attrib &= ~mask;
        port->dir |= mask;
        break;
    }
}

bool gpio_read_level(handle_t *port, uint8_t pin)
{
    unsigned mask = gpio_pin_mask(port, pin);

    return (port->data & mask) != 0U;
}

void gpio_write_level(handle_t *port, uint8_t pin, bool lev)
{
    unsigned mask = gpio_output_mask(port, pin);
    bool bufferLevel = lev;

    if ((port->attrib & mask) == 0)
    {
        bufferLevel = !lev;
    }
    if (bufferLevel)
    {
        port->buffer |= mask;
    }
    else
    {
        port->buffer &= ~mask;
    }
}

void gpio_reverse_level(handle_t *port, uint8_t pin)
{
    unsigned mask = gpio_output_mask(port, pin);

    port->buffer ^= mask;
}
