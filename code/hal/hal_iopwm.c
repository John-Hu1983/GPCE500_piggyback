#include "hal_iopwm.h"
#include "hal_gpio.h"
#include "GPCE36_CE5.h"
#include "..\bsp\bsp_global.h"

/*
 *@ brief    : Get the selected IOPWM control register.
 *@ type     : private
 *@ argu in  : channel: 0..8.
 *@ argu out : Validated control register pointer.
 *@ theme    : Validate the index before word-addressed pointer arithmetic.
 */
static volatile unsigned *_iopwm_get_reg(IOPWM_Channel channel)
{
    if ((unsigned)channel > IOPWM_Ch_8)
    {
        bsp_halt_fault(BSP_Fault_IOPWM_Ch);
    }
    return P_PWM0_Ctrl + (unsigned)channel;
}

/*
 *@ brief    : Initialize one IOPWM channel.
 *@ type     : public
 *@ argu in  : channel: 0..8.
 *@ argu out : None.
 *@ theme    : Disabled LOW output, Sys64 clock and 50% duty.
 */
void iopwm_init_channel(IOPWM_Channel channel)
{
    volatile unsigned *ctrl = _iopwm_get_reg(channel);

    *ctrl = 0;
    gpio_init_pattern(GPIOA, (uint8_t)channel, GPIO_Out);
    *ctrl = C_IOPWM_CNT_Clear | IOPWM_Clk_Sys64 | 128U;
}

/*
 *@ brief    : Enable and unmask one IOPWM channel.
 *@ type     : public
 *@ argu in  : channel: 0..8.
 *@ argu out : None.
 *@ theme    : Keep clock/duty and prepare the GPIO before enabling output.
 */
void iopwm_enable_channel(IOPWM_Channel channel)
{
    volatile unsigned *ctrl = _iopwm_get_reg(channel);

    if ((*ctrl & C_IOPWM_Enable) == 0)
    {
        gpio_init_pattern(GPIOA, (uint8_t)channel, GPIO_Out);
    }
    *ctrl = (*ctrl & (IOPWM_CONFIG & ~C_IOPWM_Mask)) | C_IOPWM_Enable;
}

/*
 *@ brief    : Disable one IOPWM channel.
 *@ type     : public
 *@ argu in  : channel: 0..8.
 *@ argu out : None.
 *@ theme    : Release PWM, drive GPIO LOW and keep clock/duty.
 */
void iopwm_disable_channel(IOPWM_Channel channel)
{
    volatile unsigned *ctrl = _iopwm_get_reg(channel);

    *ctrl &= C_IOPWM_CLK_SRC | C_IOPWM_Duty;
    gpio_init_pattern(GPIOA, (uint8_t)channel, GPIO_Out);
}

/*
 *@ brief    : Select one channel's PWM clock source.
 *@ type     : public
 *@ argu in  : channel: 0..8; clock: supported source, not Hz.
 *@ argu out : None.
 *@ theme    : Frequency is source/256; do not reconfigure shared timers.
 */
void iopwm_set_freq(IOPWM_Channel channel, IOPWM_Clock clock)
{
    volatile unsigned *ctrl = _iopwm_get_reg(channel);

    if (((unsigned)clock & ~C_IOPWM_CLK_SRC) != 0)
    {
        bsp_halt_fault(BSP_Fault_IOPWM_Clk);
    }
    *ctrl = (*ctrl & (IOPWM_CONFIG & ~C_IOPWM_CLK_SRC)) | (unsigned)clock;
}

/*
 *@ brief    : Set one channel's duty cycle.
 *@ type     : public
 *@ argu in  : channel: 0..8; duty: 0..256 (128 is 50%).
 *@ argu out : None.
 *@ theme    : Preserve clock/enable/mask; never replay counter-clear commands.
 */
void iopwm_set_duty(IOPWM_Channel channel, unsigned duty)
{
    volatile unsigned *ctrl = _iopwm_get_reg(channel);

    if (duty > 256U)
    {
        bsp_halt_fault(BSP_Fault_IOPWM_Duty);
    }
    *ctrl = (*ctrl & (IOPWM_CONFIG & ~C_IOPWM_Duty)) | duty;
}
