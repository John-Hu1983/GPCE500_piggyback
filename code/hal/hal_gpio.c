#include "hal_gpio.h"
#include "..\bsp\bsp_global.h"

/*
 *@ brief    : Validate a port and pin, then get its bit mask.
 *@ type     : private
 *@ argu in  : port: GPIOA/B/C; pin: pin index.
 *@ argu out : Validated pin mask.
 *@ theme    : Reject invalid pointers and shifts.
 */
static unsigned _gpio_get_mask(handle_t *port, uint8_t pin)
{
    if (port != GPIOA && port != GPIOB && port != GPIOC)
    {
        bsp_halt_fault(BSP_Fault_GPIO_Port);
    }
    if (pin >= 16 || (port == GPIOC && pin >= 8))
    {
        bsp_halt_fault(BSP_Fault_GPIO_Pin);
    }
    return 1U << pin;
}

/*
 *@ brief    : Get the mask of a writable output pin.
 *@ type     : private
 *@ argu in  : port: GPIOA/B/C; pin: pin index.
 *@ argu out : Output pin mask.
 *@ theme    : Reject input-only pins and input mode.
 */
static unsigned _gpio_get_outmask(handle_t *port, uint8_t pin)
{
    unsigned mask = _gpio_get_mask(port, pin);

    if ((port == GPIOC && pin < 2) || (port->dir & mask) == 0)
    {
        bsp_halt_fault(BSP_Fault_GPIO_Out);
    }
    return mask;
}

/*
 *@ brief    : Configure one GPIO pin's input or output mode.
 *@ type     : public
 *@ argu in  : port: GPIOA/B/C; pin: index; pattern: supported mode.
 *@ argu out : None.
 *@ theme    : Preserve other pins; set the latch before enabling output.
 */
void gpio_init_pattern(handle_t *port, uint8_t pin, GPIO_Pattern pattern)
{
    unsigned mask = _gpio_get_mask(port, pin);

    if ((unsigned)pattern > GPIO_Out_Inv_High)
    {
        bsp_halt_fault(BSP_Fault_GPIO_Pattern);
    }
    if (pattern == GPIO_In_PullHigh && (port != GPIOB || pin >= 8))
    {
        bsp_halt_fault(BSP_Fault_GPIO_Pull);
    }
    if (pattern >= GPIO_Out && port == GPIOC && pin < 2)
    {
        bsp_halt_fault(BSP_Fault_GPIO_Input);
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

/*
 *@ brief    : Read the physical pin level.
 *@ type     : public
 *@ argu in  : port: GPIOA/B/C; pin: pin index.
 *@ argu out : true: HIGH; false: LOW.
 *@ theme    : Read the pad register, not the output latch.
 */
bool gpio_read_level(handle_t *port, uint8_t pin)
{
    unsigned mask = _gpio_get_mask(port, pin);

    return (port->data & mask) != 0U;
}

/*
 *@ brief    : Set an output pin's physical level.
 *@ type     : public
 *@ argu in  : port: GPIOA/B/C; pin: index; lev: HIGH/LOW.
 *@ argu out : None.
 *@ theme    : Account for inverted output and preserve other latch bits.
 */
void gpio_write_level(handle_t *port, uint8_t pin, bool lev)
{
    unsigned mask = _gpio_get_outmask(port, pin);
    bool latch = lev;

    if ((port->attrib & mask) == 0)
    {
        latch = !lev;
    }
    if (latch)
    {
        port->buffer |= mask;
    }
    else
    {
        port->buffer &= ~mask;
    }
}

/*
 *@ brief    : Toggle an output pin.
 *@ type     : public
 *@ argu in  : port: GPIOA/B/C; pin: pin index.
 *@ argu out : None.
 *@ theme    : Toggle the latch without relying on the pad level.
 */
void gpio_reverse_level(handle_t *port, uint8_t pin)
{
    unsigned mask = _gpio_get_outmask(port, pin);

    port->buffer ^= mask;
}
