#include "..\Include\spinmaster.h"

/*
 *@ brief    : Run the key-controlled SPU playback example.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Does not normally return.
 *@ theme    : Run button scanning through software timers and preserve watchdog service.
 */
int main(void)
{
    system_init_module();

    bsp_power_on();

    /* Release default GPIO pull-downs on IOA9/IOA10 early so 47K+0.1uF RC caps settle during boot */
    gpio_init_pattern(USB_DET_PIN.port, USB_DET_PIN.pin, GPIO_In_Float);
    gpio_init_pattern(BAT_DET_PIN.port, BAT_DET_PIN.pin, GPIO_In_Float);

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

    backend_open_service();
    gui_desktop_enter();

    for (;;)
    {
        swt_service_loop();

        sch_service_loop();

        system_clear_watchdog();
    }
    return 0;
}
