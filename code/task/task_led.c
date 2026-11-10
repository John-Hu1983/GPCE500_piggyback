#include "task_led.h"
#include "..\bsp\bsp_global.h"
#include "..\schedule\schedule.h"

static SCH_Task led_task = SCH_TASK_INIT;
static LED_Mode led_mode = LED_Mode_Off;
static bool led_ready = false;

/*
 *@ brief    : Toggle the LED output on each scheduled half-period slice.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : PB2 output level inverted.
 *@ theme    : Invoked by the cooperative scheduler for 0.5Hz, 1Hz and 2Hz modes.
 */
static void _led_service_task(void) { gpio_reverse_level(LED_PORT, LED_PIN); }

/*
 *@ brief    : Initialize the PB2 LED GPIO and reset the LED task to OFF.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : PB2 configured as output in OFF state; blink task stopped.
 *@ theme    : Call after system_init_module() and sch_init_module().
 */
void led_init_task(void)
{
    sch_stop_task(&led_task);
    gpio_init_pattern(LED_PORT, LED_PIN, (LED_ACTIVE_LEVEL != 0) ? GPIO_Out_Low : GPIO_Out_High);
    led_mode = LED_Mode_Off;
    led_ready = true;
    led_set_mode(led_mode);
}

/*
 *@ brief    : Set the LED display mode (OFF, ON, 0.5Hz, 1Hz, or 2Hz).
 *@ type     : public
 *@ argu in  : mode: target LED_Mode value.
 *@ argu out : PB2 updated and scheduler task started or stopped accordingly.
 *@ theme    : Steady modes stop the scheduler task; blink modes toggle every half-period.
 */
void led_set_mode(LED_Mode mode)
{
    u16 slice = 0;

    if (!led_ready)
    {
        bsp_halt_fault(BSP_Fault_LED_State);
    }

    switch (mode)
    {
    case LED_Mode_Off:
        sch_stop_task(&led_task);
        gpio_write_level(LED_PORT, LED_PIN, LED_ACTIVE_LEVEL == 0);
        break;
    case LED_Mode_On:
        sch_stop_task(&led_task);
        gpio_write_level(LED_PORT, LED_PIN, LED_ACTIVE_LEVEL != 0);
        break;
    case LED_Mode_0_5Hz:
        slice = LED_SLICE_0_5HZ;
        break;
    case LED_Mode_1Hz:
        slice = LED_SLICE_1HZ;
        break;
    case LED_Mode_2Hz:
        slice = LED_SLICE_2HZ;
        break;
    default:
        bsp_halt_fault(BSP_Fault_LED_Mode);
    }

    if (slice != 0)
    {
        gpio_write_level(LED_PORT, LED_PIN, LED_ACTIVE_LEVEL != 0);
        sch_start_task(&led_task, slice, 0, _led_service_task);
    }
    led_mode = mode;
}

/*
 *@ brief    : Read the current LED display mode.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Active LED_Mode.
 *@ theme    : Allow callers to inspect or cycle display modes.
 */
LED_Mode led_get_mode(void) { return led_mode; }
