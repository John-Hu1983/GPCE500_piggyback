#include "..\Include\spinmaster.h"

static u16 Bsp_phy_state = 0;
const gpio_pin_s Power_Lock_Pin = POWER_LOCK_PIN;

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

/*
 *@ brief    : Check whether a hardware peripheral (PHY) has been initialized.
 *@ type     : public
 *@ argu in  : phy: peripheral identifier from BSP_phy_name.
 *@ argu out : true if marked initialized, false otherwise.
 *@ theme    : Track peripheral initialization state in a single 16-bit bitmask word.
 */
bool bsp_is_phy_init(BSP_phy_name phy) { return (Bsp_phy_state & (1 << phy)) != 0; }

/*
 *@ brief    : Mark a hardware peripheral (PHY) as initialized.
 *@ type     : public
 *@ argu in  : phy: peripheral identifier from BSP_phy_name.
 *@ argu out : Corresponding bit set in Bsp_phy_state.
 *@ theme    : Record peripheral readiness for dependent software modules.
 */
void bsp_set_phy_init(BSP_phy_name phy) { Bsp_phy_state |= (1 << phy); }

/*
 *@ brief    : Clear the initialized state flag of a hardware peripheral (PHY).
 *@ type     : public
 *@ argu in  : phy: peripheral identifier from BSP_phy_name.
 *@ argu out : Corresponding bit cleared in Bsp_phy_state.
 *@ theme    : Reset peripheral state flag on shutdown or re-initialization.
 */
void bsp_clear_phy_init(BSP_phy_name phy) { Bsp_phy_state &= ~(1 << phy); }

/*
 *@ brief    : Initialize all board-level hardware peripherals, runtime services, and default UI.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : System clock, power lock, heap, timers, scheduler, message bus, audio, buttons,
 *             RTC, LCD monitor, and desktop UI initialized.
 *@ theme    : Single board bring-up entry point called at the start of main().
 */
void bsp_init_global(void)
{
    system_init_module();

    gpio_init_pattern(Power_Lock_Pin.port, Power_Lock_Pin.pin, GPIO_Out_High);

    heap_init_module();
    swt_init_module();
    sch_init_module();
    msg_init_module();

    audio_set_pwm_dac(AUDIO_OUTMODE_PWM);
    spu_init_audio();
    spu_set_volume(AUDIO_VOLUME_DEFAULT);

    btn_init_module();

    rtc_init_module(RTC_Freq_2K);
    lcd_init_module();

    gui_desktop_enter();
}

/*
 *@ brief    : Run the main foreground application loop servicing timers, scheduler, and watchdog.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Does not return; continuously dispatches ready software timers and tasks.
 *@ theme    : Non-preemptive foreground super-loop.
 */
void bsp_run_app(void)
{
    for (;;)
    {
        swt_service_loop();

        sch_service_loop();

        system_clear_watchdog();
    }
}
