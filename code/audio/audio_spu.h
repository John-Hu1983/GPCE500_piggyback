#ifndef AUDIO_SPU_H
#define AUDIO_SPU_H

#include "user_config.h"

#define AUDIO_SPU_SOURCE_ROM_TABLE 0
#define AUDIO_SPU_SOURCE_FILEMERGER_VERSION1 1

/* Preserve the original default; select ROM_TABLE here or with a compiler define. */
#ifndef AUDIO_SPU_SPEECH_SOURCE_TYPE
#define AUDIO_SPU_SPEECH_SOURCE_TYPE AUDIO_SPU_SOURCE_FILEMERGER_VERSION1
#endif

#if AUDIO_SPU_SPEECH_SOURCE_TYPE != AUDIO_SPU_SOURCE_ROM_TABLE &&                                  \
    AUDIO_SPU_SPEECH_SOURCE_TYPE != AUDIO_SPU_SOURCE_FILEMERGER_VERSION1
#error Unsupported AUDIO_SPU_SPEECH_SOURCE_TYPE
#endif

typedef enum
{
    SPU_A3400_4BIT = 0,
    SPU_A3400_5BIT = 1,
    SPU_PCM_8BIT = 2,
    SPU_PCM_16BIT = 3
} SPU_Format;

/*
 * Register values: floor(sampleRate * 2^24 / 40960000).
 * Requires an 81.92 MHz system clock and C_SPUCLK_DIV_2.
 * Match the resource's sample rate; another rate changes speed and pitch.
 */
typedef enum
{
    SPU_Freq_8K = 0x0CCCU,
    SPU_Freq_11K025 = 0x11A3U,
    SPU_Freq_12K = 0x1333U,
    SPU_Freq_16K = 0x1999U,
    SPU_Freq_22K050 = 0x2347U,
    SPU_Freq_24K = 0x2666U,
    SPU_Freq_32K = 0x3333U,
    SPU_Freq_44K100 = 0x468FU,
    SPU_Freq_48K = 0x4CCCU
} SPU_Sample_Freq;

/* Hardware channel indices, not enable-bit masks. */
typedef enum
{
    SPU_Channel_0 = 0,
    SPU_Channel_1 = 1,
    SPU_Channel_2 = 2,
    SPU_Channel_3 = 3,
    SPU_Channel_4 = 4,
    SPU_Channel_5 = 5,
    SPU_Channel_6 = 6,
    SPU_Channel_7 = 7
} SPU_Channel;

extern unsigned long Audio_SPU_GetPCMTableAddress(void);
extern unsigned long Audio_SPU_GetA34PETableAddress(void);

unsigned audio_set_pwm_dac(unsigned outputMode);
void spu_init_audio(void);
void spu_select_voice(SPU_Channel channel, SPU_Format format, SPU_Sample_Freq freq, unsigned inx);
void spu_open_channel(SPU_Channel channel);
void spu_set_volume(unsigned volumeIndex);
void spu_stop_channel(SPU_Channel channel);
void spu_read_PCM_addrs(unsigned inx);
void spu_read_A34PE_addrs(unsigned inx);

#endif
