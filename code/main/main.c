#include "System.h"
#include "..\audio\audio_spu.h"
#include "..\hal\hal_rtc.h"
#include "..\hal\hal_gpio.h"

#include "../bsp/bsp_global.h"

unsigned key = 0;
unsigned voice = 0;
unsigned vol = 9; ////0 ~ 15

void rtc_cb(void) { gpio_reverse_level(GPIOB, 0); }

/*
 *@ brief    : Run the key-controlled SPU playback example.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Does not normally return.
 *@ theme    : Initialize the system/audio and service key events continuously.
 */
int main(void)
{
    // add your code here
    bsp_init_global();
    key = 0;

    vol = AUDIO_VOLUME_DEFAULT;

    gpio_init_pattern(GPIOB, 0, GPIO_Out);
    rtc_init_module(RTC_Freq_2K, rtc_cb);
    while (1)
    {
        key = key_read_event();
        switch (key)
        {
        case 0x0001: // IOA0 + Vcc
            voice++; // FileMerger resource index, 0..4.
            if (voice >= 5)
                voice = 0;
            spu_select_voice(SPU_Channel_0, SPU_A3400_4BIT, SPU_Freq_12K, voice);
            spu_open_channel(SPU_Channel_0);
            break;

        case 0x0002:                         // IOA1 + Vcc
            spu_stop_channel(SPU_Channel_0); ////Channel
            break;

        case 0x0004: // IOA2 + Vcc
            voice = 0;
            spu_select_voice(SPU_Channel_1, SPU_A3400_4BIT, SPU_Freq_12K, voice);
            spu_open_channel(SPU_Channel_1);
            break;

        case 0x0008:                         // IOA3 + Vcc
            spu_stop_channel(SPU_Channel_1); ////Channel
            break;

        case 0x0010: // IOA4 + Vcc
            vol++;
            if (vol >= 16)
            {
                vol = 0;
            }
            spu_set_volume(vol);

            break;

        case 0x0020: // IOA5 + Vcc

            break;

        case 0x0040: // IOA6 + Vcc

            break;

        case 0x0080: // IOA7 + Vcc

            break;

        default:
            break;
        } // end of switch

        system_service_loop();
    }

    return 0;
}
