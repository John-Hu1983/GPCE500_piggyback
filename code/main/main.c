#include "System.h"
#include "..\audio\audio_spu.h"

unsigned Key = 0;
unsigned A34PE_Index = 0;
unsigned VolIndex_SPU = 9; ////0 ~ 15

//***************************************************************************************
// Main Function Area
//***************************************************************************************
int main(void)
{
    // add your code here
    Key = 0;

    System_Initial(); // System initial

    audio_set_pwm_dac(AUDIO_OUTMODE_PWM);

    spu_init_audio();
    spu_set_volume(AUDIO_VOLUME_DEFAULT);

    VolIndex_SPU = AUDIO_VOLUME_DEFAULT;
    while (1)
    {
        Key = SP_GetCh();
        switch (Key)
        {
        case 0x0001:       // IOA0 + Vcc
            A34PE_Index++; // index in ROM Table 0 ~ 1;  //in fileMerger rom bin  0 ~ 4 is
                           // A34PE_Index
            if (A34PE_Index >= 5)
                A34PE_Index = 0;
            spu_select_voice(SPU_Channel_0, SPU_A3400_4BIT, SPU_Freq_12K, A34PE_Index);
            spu_open_channel(SPU_Channel_0);
            break;

        case 0x0002:             // IOA1 + Vcc
            spu_stop_channel(SPU_Channel_0); ////Channel
            break;

        case 0x0004: // IOA2 + Vcc
            A34PE_Index = 0;
            spu_select_voice(SPU_Channel_1, SPU_A3400_4BIT, SPU_Freq_12K, A34PE_Index);
            spu_open_channel(SPU_Channel_1);
            break;

        case 0x0008:             // IOA3 + Vcc
            spu_stop_channel(SPU_Channel_1); ////Channel
            break;

        case 0x0010: // IOA4 + Vcc
            VolIndex_SPU++;
            if (VolIndex_SPU >= 16)
            {
                VolIndex_SPU = 0;
            }
            spu_set_volume(VolIndex_SPU);

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

        System_ServiceLoop();
    }

    return 0;
}
