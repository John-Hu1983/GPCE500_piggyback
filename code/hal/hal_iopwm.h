#ifndef _HAL_IOPWM_H__
#define _HAL_IOPWM_H__

#include "GPCE36_CE5.h"

#define IOPWM_CONFIG (C_IOPWM_Enable | C_IOPWM_Mask | C_IOPWM_CLK_SRC | C_IOPWM_Duty)

typedef enum
{
    IOPWM_Ch_0 = 0,
    IOPWM_Ch_1 = 1,
    IOPWM_Ch_2 = 2,
    IOPWM_Ch_3 = 3,
    IOPWM_Ch_4 = 4,
    IOPWM_Ch_5 = 5,
    IOPWM_Ch_6 = 6,
    IOPWM_Ch_7 = 7,
    IOPWM_Ch_8 = 8
} IOPWM_Channel;

typedef enum
{
    IOPWM_Clk_32K2 = 0x0000,
    IOPWM_Clk_TMA = 0x0200,
    IOPWM_Clk_TMB = 0x0400,
    IOPWM_Clk_TMC = 0x0600,
    IOPWM_Clk_Sys = 0x0800,
    IOPWM_Clk_Sys8 = 0x0A00,
    IOPWM_Clk_Sys64 = 0x0C00,
    IOPWM_Clk_Sys1024 = 0x0E00
} IOPWM_Clock;

/*
 * Channels 0..8 map directly to IOA0..IOA8. Call iopwm_init_channel after system_init_module.
 * init resets only the selected channel: disabled, GPIO LOW, Sys64 clock, 50% duty.
 * enable unmasks PWM; disable releases PWM and drives the pin LOW, keeping clock/duty.
 * Caller must release conflicting pin peripherals before use.
 *
 * Frequency = selected clock / 256; set_freq selects a clock, not a value in Hz.
 * At Fsys=81.92MHz: Sys=320kHz, Sys8=40kHz, Sys64=5kHz, Sys1024=312.5Hz.
 * 32K2 gives F32K/512 (~64Hz). TMA/B/C give the timer timeout rate /256.
 * Timers are caller-owned: this driver never configures them. Channels sharing
 * a timer share its frequency; changing Fsys also changes system-derived outputs.
 *
 * duty is 0..256: 0=0%, 128=50%, 255=255/256, 256=100%. All other values are invalid.
 * Updates are immediate, not guaranteed glitch-free or synchronized.
 * Serialize access to PWM/GPIO registers if shared with an ISR. Disable before sleep.
 * Invalid arguments halt in _iopwm_halt_fault before register writes; no fault RAM is stored.
 */
void iopwm_init_channel(IOPWM_Channel channel);
void iopwm_enable_channel(IOPWM_Channel channel);
void iopwm_disable_channel(IOPWM_Channel channel);
void iopwm_set_freq(IOPWM_Channel channel, IOPWM_Clock clock);
void iopwm_set_duty(IOPWM_Channel channel, unsigned duty);

#endif /* _HAL_IOPWM_H__ */
