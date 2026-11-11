#ifndef _HAL_GPIO_H__
#define _HAL_GPIO_H__

typedef struct
{
    volatile unsigned data; /* Read-only pad level in hardware. */
    volatile unsigned buffer;
    volatile unsigned dir;
    volatile unsigned attrib;
} handle_t;

#define GPIOA ((handle_t *)0x3000UL)
#define GPIOB ((handle_t *)0x3004UL)
#define GPIOC ((handle_t *)0x3008UL)

typedef enum
{
    GPIO_In_Float = 0,
    GPIO_In_PullLow = 1,
    GPIO_In_PullHigh = 2,
    GPIO_Out_Low = 3,
    GPIO_Out_Inv = 4,
    GPIO_Out_High = 5,
    GPIO_Out_Inv_High = 6
} GPIO_Pattern;

typedef struct
{
    handle_t *port;
    uint8_t pin;
} gpio_pin_s;

/*
 * port: GPIOA/GPIOB/GPIOC; pin is an index, not a mask.
 * A/B: 0..15; C: 0..7, with C0/C1 input-only. Pull-high is B0..B7 only.
 * Configure after system_init_module(), which resets A/B.
 * Outputs initialize LOW, except *_High patterns. The latch is set before enabling output.
 * Read/write levels refer to the physical pad, even when inverted.
 * Write/reverse require an output. Invalid requests halt in bsp_halt_fault before GPIO writes,
 * even with NDEBUG; set a debugger breakpoint there. No fault state or callback is stored.
 * Mux/peripheral ownership, wakeup and IOB's PL1M setting are not changed.
 * Register updates are read-modify-write: serialize access if a port is shared with an ISR.
 */
void gpio_init_pattern(handle_t *port, uint8_t pin, GPIO_Pattern pattern);
bool gpio_read_level(handle_t *port, uint8_t pin);
void gpio_write_level(handle_t *port, uint8_t pin, bool lev);
void gpio_reverse_level(handle_t *port, uint8_t pin);
#endif
