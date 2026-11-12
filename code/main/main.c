#include "..\Include\spinmaster.h"

unsigned voice = 0;
unsigned vol = 9; ////0 ~ 15

static MSG_Sub main_button_sub = MSG_SUB_INIT;

/*
 *@ brief    : Handle one key-controlled SPU action.
 *@ type     : private
 *@ argu in  : message: generic header with a BTN_Message payload.
 *@ argu out : Updates the selected voice/channel or volume.
 *@ theme    : Keep one action per short click or hold; do not merge simultaneous keys.
 */
static void _main_service_button(const MSG_Message *message)
{
    const BTN_Message *button;

    if (message->type != MSG_Button)
    {
        return;
    }
    if (message->data == null)
    {
        bsp_halt_fault(BSP_Fault_MSG_Message);
    }
    button = message->data;
    if (button->event != BTN_Click && button->event != BTN_Hold)
    {
        return;
    }
    switch (button->key)
    {
    case BTN_Key_0:
        voice++;
        if (voice >= 5)
        {
            voice = 0;
        }
        spu_select_voice(SPU_Channel_0, SPU_A3400_4BIT, SPU_Freq_12K, voice);
        spu_open_channel(SPU_Channel_0);
        break;
    case BTN_Key_1:
        spu_stop_channel(SPU_Channel_0);
        break;
    case BTN_Key_2:
        voice = 0;
        spu_select_voice(SPU_Channel_1, SPU_A3400_4BIT, SPU_Freq_12K, voice);
        spu_open_channel(SPU_Channel_1);
        break;
    case BTN_Key_3:
        spu_stop_channel(SPU_Channel_1);
        break;
    case BTN_Key_4:
        vol++;
        if (vol >= 16)
        {
            vol = 0;
        }
        spu_set_volume(vol);
        break;
    case BTN_Key_5:
        if (led_get_mode() >= LED_Mode_2Hz)
        {
            led_set_mode(LED_Mode_Off);
        }
        else
        {
            led_set_mode((LED_Mode)(led_get_mode() + 1));
        }
        break;
    default:
        break;
    }
}

/*
 *@ brief    : Run the key-controlled SPU playback example.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Does not normally return.
 *@ theme    : Run button scanning through software timers and preserve watchdog service.
 */
int main(void)
{
    bsp_init_global();
    vol = AUDIO_VOLUME_DEFAULT;
    msg_subscribe_type(&main_button_sub, MSG_Button, null, _main_service_button);

    while (1)
    {
        swt_service_loop();
        sch_service_loop();
        system_clear_watchdog();
    }

    return 0;
}
