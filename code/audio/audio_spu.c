#include "audio_spu.h"
#include "GPCE36_CE5.h"
#include "Resource.h"
#include "System.h"

/*
 * SPU 初始化、输出模式、资源配置和播放控制集中在本文件。
 * CPU 中断控制由系统模块提供；汇编仅提供资源表地址。
 * 本芯片按 16 位字寻址；FileMerger 表内记录的偏移则以字节为单位。
 */
#define AUDIO_SPU_CHANNEL_STRIDE 0x10U

unsigned DPTR_H = 0;
unsigned DPTR_L = 0;

static const unsigned volumeLevels[16] = {0x0480, 0x0580, 0x0680, 0x0880, 0x0A80, 0x0D80,
                                          0x1080, 0x1480, 0x1980, 0x2080, 0x2880, 0x3380,
                                          0x4F80, 0x5180, 0x6680, 0x7F80};

static void _spu_set_addr_from_table(unsigned long tableAddress, unsigned inx)
{
    const volatile unsigned *entry;

    /* 每项由低、高两个字组成，不能把这个表误当成字节数组。 */
    entry = (const volatile unsigned *)(tableAddress + 2UL * inx);
    DPTR_L = entry[0];
    DPTR_H = entry[1];
}

unsigned audio_set_pwm_dac(unsigned outputMode)
{
    unsigned control;

    if (outputMode == AUDIO_OUTMODE_PWM)
    {
        *P_AUDIO_Ctrl1 =
            C_AUDIO_PWMIP_Enable | C_AUDIO_PWM_Enable | C_AUDIO_Gain_Sel | C_MuteControl_DATACHAGE;
        control = C_PWM_sData;
    }
    else
    {
        *P_AUDIO_Ctrl1 =
            C_AUDIO_PWMIP_Enable | C_AUDIO_PWM_Enable | C_AUDIO_Gain_Sel | C_MuteControl_DATACHAGE;
        control = C_PWM_sData | C_RDAC_Enable | C_PWM_Mute_50Duty | C_DRAC_Current;
    }

    *P_AUDIO_Ctrl3 = control;
    return control;
}

void spu_init_audio(void)
{
    *P_SPU_CLK_Ctrl = C_SPUCLK_DIV_2;
    *P_SPU_Volume_Ctrl = C_SPU_Mixer_MaxVolume | C_SPU_Mixer_FullSwing;
    *P_SPU_Mixer_Ctrl = C_SPU_Mixer_Ch0_En | C_SPU_Mixer_Ch1_En;
    *P_SPU_INT_Ctrl = C_SPU_Ch0_INT_En | C_SPU_Ch1_INT_En;
    *P_INT_Ctrl = C_IRQ5_SPU;
    System_EnableIRQ();
}

void spu_set_volume(unsigned volumeIndex)
{
    unsigned level = volumeLevels[volumeIndex];

    /* bit15 是原控制位，调整音量时保留它。 */
    *P_SPU_Volume_Ctrl = level | (*P_SPU_Volume_Ctrl & 0x8000);
}

void spu_open_channel(SPU_Channel channel) { *P_SPU_CH_Ctrl |= C_SPU_Ch0_En << channel; }

void spu_stop_channel(SPU_Channel channel) { *P_SPU_CH_Ctrl &= ~(C_SPU_Ch0_En << channel); }

void spu_read_PCM_addrs(unsigned inx)
{
    _spu_set_addr_from_table(Audio_SPU_GetPCMTableAddress(), inx);
}

void spu_read_A34PE_addrs(unsigned inx)
{
#if AUDIO_SPU_SPEECH_SOURCE_TYPE == AUDIO_SPU_SOURCE_ROM_TABLE
    _spu_set_addr_from_table(Audio_SPU_GetA34PETableAddress(), inx);
#else
    unsigned long baseAddress = RES_FILEMERGER_ROM_BIN_SA;
    const volatile unsigned *entry;
    unsigned long byteOffset;
    unsigned long wordAddress;

    /* 第一个 32 位字是文件数；之后每项是一个 32 位字节偏移。 */
    entry = (const volatile unsigned *)(baseAddress + 2UL + 2UL * inx);
    byteOffset = entry[0];
    byteOffset |= (unsigned long)entry[1] << 16;

    /* 等价于原汇编的高字右移、移位缓冲区旋转和带进位相加。 */
    wordAddress = baseAddress + (byteOffset >> 1);
    DPTR_L = (unsigned)wordAddress;
    DPTR_H = (unsigned)(wordAddress >> 16);
#endif
}

void spu_select_voice(SPU_Channel channel, SPU_Format format, SPU_Sample_Freq freq,
                      unsigned inx)
{
    unsigned pcmFormat = format == SPU_PCM_8BIT || format == SPU_PCM_16BIT;
    unsigned registerOffset = AUDIO_SPU_CHANNEL_STRIDE * channel;
    unsigned long byteAddress;

    if (pcmFormat)
    {
        spu_read_PCM_addrs(inx);
    }
    else
    {
        spu_read_A34PE_addrs(inx);
    }

    byteAddress = ((unsigned long)DPTR_H << 16) | DPTR_L;
    byteAddress <<= 1;

    /* A3400 跳过 9 字节头；PCM 的硬件起点是首字节地址减 1。 */
    if (pcmFormat)
    {
        byteAddress -= 1UL;
    }
    else
    {
        byteAddress += 9UL;
    }

    DPTR_L = (unsigned)byteAddress;
    /* 高字的 bit15..14 是格式选择位，不是资源地址的一部分。 */
    DPTR_H = (unsigned)(byteAddress >> 16) | ((unsigned)format << 14);

    *(P_SPU_Ch0_Volume_Ctrl + registerOffset) = 0xFFFF;
    *(P_SPU_Ch0_SampleFreq + registerOffset) = freq;
    *(P_SPU_Ch0_DPTR_L + registerOffset) = DPTR_L;
    *(P_SPU_Ch0_DPTR_H + registerOffset) = DPTR_H;
    *(P_SPU_Ch0_LoopLength + registerOffset) = 0;
    *(P_SPU_Ch0_NextDPTR_L + registerOffset) = 0;
    *(P_SPU_Ch0_NextDPTR_H + registerOffset) = 0;
    *(P_SPU_Ch0_Filter_S2 + registerOffset) = 0;
    *(P_SPU_Ch0_Filter_S1 + registerOffset) = 0;
}
