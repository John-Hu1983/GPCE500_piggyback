#include "..\Include\spinmaster.h"

/*
 * SPU 初始化、输出模式、资源配置和播放控制集中在本文件。
 * CPU 中断控制由系统模块提供；汇编仅提供资源表地址。
 * 本芯片按 16 位字寻址；FileMerger 表内记录的偏移则以字节为单位。
 */
unsigned spu_addr_hi = 0;
unsigned spu_addr_lo = 0;

static const unsigned vol_tab[16] = {0x0480, 0x0580, 0x0680, 0x0880, 0x0A80, 0x0D80,
                                     0x1080, 0x1480, 0x1980, 0x2080, 0x2880, 0x3380,
                                     0x4F80, 0x5180, 0x6680, 0x7F80};

/*
 *@ brief    : Load a resource address from a word-address table.
 *@ type     : private
 *@ argu in  : table: table word address; inx: resource index.
 *@ argu out : Updates spu_addr_lo and spu_addr_hi.
 *@ theme    : Each table entry contains a low and high word.
 */
static void _spu_read_table(unsigned long table, unsigned inx)
{
    const volatile unsigned *entry;

    /* 每项由低、高两个字组成，不能把这个表误当成字节数组。 */
    entry = (const volatile unsigned *)(table + 2UL * inx);
    spu_addr_lo = entry[0];
    spu_addr_hi = entry[1];
}

/*
 *@ brief    : Select PWM or DAC audio output.
 *@ type     : public
 *@ argu in  : mode: AUDIO_OUTMODE_PWM or AUDIO_OUTMODE_DAC.
 *@ argu out : Written audio control-register value.
 *@ theme    : Configure audio only; do not control amplifier GPIO.
 */
unsigned audio_set_pwm_dac(unsigned mode)
{
    unsigned ctrl;

    if (mode == AUDIO_OUTMODE_PWM)
    {
        *P_AUDIO_Ctrl1 = C_AUDIO_PWM_DS6 | C_AUDIO_PWMIP_Enable | C_AUDIO_PWM_Enable |
                         C_AUDIO_Gain_Sel | C_MuteControl_DATACHAGE;
        ctrl = C_PWM_sData;
    }
    else
    {
        *P_AUDIO_Ctrl1 = C_AUDIO_PWM_DS6 | C_AUDIO_PWMIP_Enable | C_AUDIO_PWM_Enable |
                         C_AUDIO_Gain_Sel | C_MuteControl_DATACHAGE;
        ctrl = C_PWM_sData | C_RDAC_Enable | C_PWM_Mute_50Duty | C_DRAC_Current;
    }

    *P_AUDIO_Ctrl3 = ctrl;
    return ctrl;
}

/*
 *@ brief    : Initialize SPU clock, mixing and interrupts.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : None.
 *@ theme    : Keep mixing channels 0/1; enable all channel IRQs without replacing IRQ masks.
 */
void spu_init_audio(void)
{
    *P_SPU_CLK_Ctrl = C_SPUCLK_DIV_2;
    *P_SPU_Volume_Ctrl = C_SPU_Mixer_MaxVolume | C_SPU_Mixer_FullSwing;
    *P_SPU_Mixer_Ctrl = C_SPU_Mixer_Ch0_En | C_SPU_Mixer_Ch1_En;
    *P_SPU_INT_Ctrl = 0;
    *P_SPU_INT_Status = AUDIO_SPU_CHANNEL_MASK;
    *P_INT_Status = C_IRQ5_SPU;
    *P_FIQ_Sel &= ~C_IRQ5_SPU;
    *P_SPU_INT_Ctrl = AUDIO_SPU_CHANNEL_MASK;
    *P_INT_Ctrl |= C_IRQ5_SPU;
    system_enable_irq();
}

/*
 *@ brief    : Service every pending enabled SPU channel interrupt.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Stops flagged channels and acknowledges their interrupt flags.
 *@ theme    : IRQ5 calls this; preserve other channels, concatenation and shared IRQ sources.
 */
void spu_service_irq(void)
{
    unsigned flags;

    if ((*P_INT_Status & *P_INT_Ctrl & ~*P_FIQ_Sel & C_IRQ5_SPU) == 0)
    {
        return;
    }

    /* Ack the parent first so a later channel event is not erased on exit. */
    *P_INT_Status = C_IRQ5_SPU;
    flags = *P_SPU_INT_Status & *P_SPU_INT_Ctrl & AUDIO_SPU_CHANNEL_MASK;
    if (flags != 0)
    {
        *P_SPU_CH_Ctrl &= ~flags;
        *P_SPU_INT_Status = flags;
    }
}

/*
 *@ brief    : Set the SPU mixer volume.
 *@ type     : public
 *@ argu in  : vol: volume table index, 0..15.
 *@ argu out : None.
 *@ theme    : Preserve the existing bit15 control flag.
 */
void spu_set_volume(unsigned vol)
{
    unsigned level = vol_tab[vol];

    /* bit15 是原控制位，调整音量时保留它。 */
    *P_SPU_Volume_Ctrl = level | (*P_SPU_Volume_Ctrl & 0x8000);
}

/*
 *@ brief    : Enable one SPU playback channel.
 *@ type     : public
 *@ argu in  : channel: hardware index, 0..7.
 *@ argu out : None.
 *@ theme    : Preserve other channel-enable bits.
 */
void spu_open_channel(SPU_Channel channel) { *P_SPU_CH_Ctrl |= C_SPU_Ch0_En << channel; }

/*
 *@ brief    : Stop one SPU playback channel.
 *@ type     : public
 *@ argu in  : channel: hardware index, 0..7.
 *@ argu out : None.
 *@ theme    : Preserve other channel-enable bits.
 */
void spu_stop_channel(SPU_Channel channel) { *P_SPU_CH_Ctrl &= ~(C_SPU_Ch0_En << channel); }

/*
 *@ brief    : Load a PCM resource address.
 *@ type     : public
 *@ argu in  : inx: resource index.
 *@ argu out : Updates spu_addr_lo and spu_addr_hi.
 *@ theme    : Read the assembly-provided PCM address table.
 */
void spu_read_pcm(unsigned inx) { _spu_read_table(Audio_SPU_GetPCMTableAddress(), inx); }

/*
 *@ brief    : Load an A3400 resource address.
 *@ type     : public
 *@ argu in  : inx: resource index.
 *@ argu out : Updates spu_addr_lo and spu_addr_hi.
 *@ theme    : Support ROM tables and FileMerger byte offsets.
 */
void spu_read_a34pe(unsigned inx)
{
#if AUDIO_SPU_SPEECH_SOURCE_TYPE == AUDIO_SPU_SOURCE_ROM_TABLE
    _spu_read_table(Audio_SPU_GetA34PETableAddress(), inx);
#else
    unsigned long base = RES_FILEMERGER_ROM_BIN_SA;
    const volatile unsigned *entry;
    unsigned long offset;
    unsigned long addr;

    /* 第一个 32 位字是文件数；之后每项是一个 32 位字节偏移。 */
    entry = (const volatile unsigned *)(base + 2UL + 2UL * inx);
    offset = entry[0];
    offset |= (unsigned long)entry[1] << 16;

    /* 等价于原汇编的高字右移、移位缓冲区旋转和带进位相加。 */
    addr = base + (offset >> 1);
    spu_addr_lo = (unsigned)addr;
    spu_addr_hi = (unsigned)(addr >> 16);
#endif
}

/*
 *@ brief    : Configure the resource and format for one SPU channel.
 *@ type     : public
 *@ argu in  : channel: 0..7; format: codec; freq: sample rate; inx: resource index.
 *@ argu out : Channel playback registers and current resource address.
 *@ theme    : Convert word addresses to byte addresses and reset filter state.
 */
void spu_select_voice(SPU_Channel channel, SPU_Format format, SPU_Sample_Freq freq, unsigned inx)
{
    unsigned pcm = format == SPU_PCM_8BIT || format == SPU_PCM_16BIT;
    unsigned offset = AUDIO_SPU_CHANNEL_STRIDE * channel;
    unsigned long addr;

    if (pcm)
    {
        spu_read_pcm(inx);
    }
    else
    {
        spu_read_a34pe(inx);
    }

    addr = ((unsigned long)spu_addr_hi << 16) | spu_addr_lo;
    addr <<= 1;

    /* A3400 跳过 9 字节头；PCM 的硬件起点是首字节地址减 1。 */
    if (pcm)
    {
        addr -= 1UL;
    }
    else
    {
        addr += 9UL;
    }

    spu_addr_lo = (unsigned)addr;
    /* 高字的 bit15..14 是格式选择位，不是资源地址的一部分。 */
    spu_addr_hi = (unsigned)(addr >> 16) | ((unsigned)format << 14);

    *(P_SPU_Ch0_Volume_Ctrl + offset) = 0xFFFF;
    *(P_SPU_Ch0_SampleFreq + offset) = freq;
    *(P_SPU_Ch0_DPTR_L + offset) = spu_addr_lo;
    *(P_SPU_Ch0_DPTR_H + offset) = spu_addr_hi;
    *(P_SPU_Ch0_LoopLength + offset) = 0;
    *(P_SPU_Ch0_NextDPTR_L + offset) = 0;
    *(P_SPU_Ch0_NextDPTR_H + offset) = 0;
    *(P_SPU_Ch0_Filter_S2 + offset) = 0;
    *(P_SPU_Ch0_Filter_S1 + offset) = 0;
}

/*
 *@ brief    : Select a voice resource and immediately start playback on the specified SPU channel.
 *@ type     : public
 *@ argu in  : channel: SPU_Channel_0..7; format: SPU_Format; freq: SPU_Sample_Freq; inx: voice
 * index.
 *@ argu out : Channel configured and opened for playback.
 *@ theme    : Convenience wrapper combining spu_select_voice and spu_open_channel.
 */
void spu_play_voice(SPU_Channel channel, SPU_Format format, SPU_Sample_Freq freq, unsigned inx)
{
    spu_select_voice(channel, format, freq, inx);
    spu_open_channel(channel);
}

/*
 * 16-sample-per-cycle chiptune pulse/triangle waveform with high-amplitude decay envelope,
 * terminated by 0x8000 (16-bit PCM end marker). Used by spu_play_tone() on SPU_Channel_1.
 */
static const unsigned spu_tone_wave[193] = {
    0x6800U, 0x7800U, 0x7800U, 0x7800U, 0x6800U, 0x4800U, 0x2400U, 0x0000U, 0x9800U, 0x8800U,
    0x8800U, 0x8800U, 0x9800U, 0xB800U, 0xDC00U, 0x0000U, 0x6800U, 0x7800U, 0x7800U, 0x7800U,
    0x6800U, 0x4800U, 0x2400U, 0x0000U, 0x9800U, 0x8800U, 0x8800U, 0x8800U, 0x9800U, 0xB800U,
    0xDC00U, 0x0000U, 0x6000U, 0x7000U, 0x7000U, 0x7000U, 0x6000U, 0x4000U, 0x2000U, 0x0000U,
    0xA000U, 0x9000U, 0x9000U, 0x9000U, 0xA000U, 0xC000U, 0xE000U, 0x0000U, 0x5800U, 0x6800U,
    0x6800U, 0x6800U, 0x5800U, 0x3C00U, 0x1E00U, 0x0000U, 0xA800U, 0x9800U, 0x9800U, 0x9800U,
    0xA800U, 0xC400U, 0xE200U, 0x0000U, 0x5000U, 0x6000U, 0x6000U, 0x6000U, 0x5000U, 0x3400U,
    0x1A00U, 0x0000U, 0xB000U, 0xA000U, 0xA000U, 0xA000U, 0xB000U, 0xCC00U, 0xE600U, 0x0000U,
    0x4800U, 0x5400U, 0x5400U, 0x5400U, 0x4800U, 0x3000U, 0x1800U, 0x0000U, 0xB800U, 0xAC00U,
    0xAC00U, 0xAC00U, 0xB800U, 0xD000U, 0xE800U, 0x0000U, 0x3C00U, 0x4800U, 0x4800U, 0x4800U,
    0x3C00U, 0x2800U, 0x1400U, 0x0000U, 0xC400U, 0xB800U, 0xB800U, 0xB800U, 0xC400U, 0xD800U,
    0xEC00U, 0x0000U, 0x3000U, 0x3C00U, 0x3C00U, 0x3C00U, 0x3000U, 0x2000U, 0x1000U, 0x0000U,
    0xD000U, 0xC400U, 0xC400U, 0xC400U, 0xD000U, 0xE000U, 0xF000U, 0x0000U, 0x2400U, 0x3000U,
    0x3000U, 0x3000U, 0x2400U, 0x1800U, 0x0C00U, 0x0000U, 0xDC00U, 0xD000U, 0xD000U, 0xD000U,
    0xDC00U, 0xE800U, 0xF400U, 0x0000U, 0x1C00U, 0x2400U, 0x2400U, 0x2400U, 0x1C00U, 0x1200U,
    0x0800U, 0x0000U, 0xE400U, 0xDC00U, 0xDC00U, 0xDC00U, 0xE400U, 0xEE00U, 0xF800U, 0x0000U,
    0x1200U, 0x1800U, 0x1800U, 0x1800U, 0x1200U, 0x0C00U, 0x0600U, 0x0000U, 0xEE00U, 0xE800U,
    0xE800U, 0xE800U, 0xEE00U, 0xF400U, 0xFA00U, 0x0000U, 0x0800U, 0x0C00U, 0x0C00U, 0x0C00U,
    0x0800U, 0x0600U, 0x0200U, 0x0000U, 0xF800U, 0xF400U, 0xF400U, 0xF400U, 0xF800U, 0xFA00U,
    0xFE00U, 0x0000U, 0x8000U};

/*
 *@ brief    : Synthesize a retro chiptune tone on SPU_Channel_1 at the given sample frequency.
 *@ type     : public
 *@ argu in  : sample_freq: SPU_Sample_Freq or raw phase-adder value (e.g., 0x0800..0x4800);
 *             vol_mask: channel volume register value (e.g., 0x7FFF).
 *@ argu out : SPU_Channel_1 plays the decaying pulse wave and auto-stops at the 0x8000 end tag.
 *@ theme    : Zero-asset procedural game sound effects using hardware SPU 16-bit PCM.
 */
void spu_play_tone(unsigned sample_freq, unsigned vol_mask)
{
    unsigned offset = AUDIO_SPU_CHANNEL_STRIDE * SPU_Channel_1;
    unsigned long addr = ((unsigned long)spu_tone_wave << 1) - 1UL;
    unsigned lo = (unsigned)addr;
    unsigned hi = (unsigned)(addr >> 16) | ((unsigned)SPU_PCM_16BIT << 14);

    spu_stop_channel(SPU_Channel_1);
    *(P_SPU_Ch0_Volume_Ctrl + offset) = vol_mask;
    *(P_SPU_Ch0_SampleFreq + offset) = sample_freq;
    *(P_SPU_Ch0_DPTR_L + offset) = lo;
    *(P_SPU_Ch0_DPTR_H + offset) = hi;
    *(P_SPU_Ch0_LoopLength + offset) = 0;
    *(P_SPU_Ch0_NextDPTR_L + offset) = 0;
    *(P_SPU_Ch0_NextDPTR_H + offset) = 0;
    *(P_SPU_Ch0_Filter_S2 + offset) = 0;
    *(P_SPU_Ch0_Filter_S1 + offset) = 0;
    spu_open_channel(SPU_Channel_1);
}
