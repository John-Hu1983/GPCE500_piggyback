#include "bsp_global.h"
#include "System.h"
#include "..\audio\audio_spu.h"
#include "..\dev\soft_timer.h"
#include "..\hal\hal_gpio.h"
#include "..\hal\hal_rtc.h"
#include "..\dev\dev_button.h"
#include "..\message\message.h"
#include "..\schedule\schedule.h"
#include "..\task\task_led.h"

static u16 Bsp_phy_state = 0;
/*
 *@ brief    : Halt on a fatal driver fault.
 *@ type     : public
 *@ argu in  : fault: compact driver/check code for debugger inspection.
 *@ argu out : Does not return.
 *@ theme    : Retain the volatile stack argument without allocating persistent RAM.
 */
void bsp_halt_fault(volatile BSP_Fault fault)
{
    for (;;)
    {
        /* Keep the stack argument materialized in the unSP optimizer. */
        fault = fault;
    }
}

bool bsp_is_phy_init(BSP_phy_name phy) { return (Bsp_phy_state & (1 << phy)) != 0; }

void bsp_set_phy_init(BSP_phy_name phy) { Bsp_phy_state |= (1 << phy); }

void bsp_clear_phy_init(BSP_phy_name phy) { Bsp_phy_state &= ~(1 << phy); }

void bsp_init_global(void)
{
    system_init_module();
    swt_init_module();
    sch_init_module();
    msg_init_module();

    audio_set_pwm_dac(AUDIO_OUTMODE_PWM);
    spu_init_audio();
    spu_set_volume(AUDIO_VOLUME_DEFAULT);

    btn_init_module();
    led_init_task();

    rtc_init_module(RTC_Freq_2K);
}
