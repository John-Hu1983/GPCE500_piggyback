#ifndef AUDIO_SPU_H
#define AUDIO_SPU_H

#define AUDIO_SPU_SOURCE_ROM_TABLE           0
#define AUDIO_SPU_SOURCE_FILEMERGER_VERSION1 1

/* Preserve the original default; select ROM_TABLE here or with a compiler define. */
#ifndef AUDIO_SPU_SPEECH_SOURCE_TYPE
#define AUDIO_SPU_SPEECH_SOURCE_TYPE AUDIO_SPU_SOURCE_FILEMERGER_VERSION1
#endif

#if AUDIO_SPU_SPEECH_SOURCE_TYPE != AUDIO_SPU_SOURCE_ROM_TABLE && \
    AUDIO_SPU_SPEECH_SOURCE_TYPE != AUDIO_SPU_SOURCE_FILEMERGER_VERSION1
#error Unsupported AUDIO_SPU_SPEECH_SOURCE_TYPE
#endif

typedef enum {
    SPU_FORMAT_A3400_4BIT = 0,
    SPU_FORMAT_A3400_5BIT = 1,
    SPU_FORMAT_PCM_8BIT = 2,
    SPU_FORMAT_PCM_16BIT = 3
} SPU_Format;

/* (sampleRate / 40960000) * 2^24, with C_SPUCLK_DIV_2. */
#define SPU_SAMPLE_FREQ_12K 0x1333U
#define SPU_SAMPLE_FREQ_48K 0x4CCCU

extern unsigned long Audio_SPU_GetPCMTableAddress(void);
extern unsigned long Audio_SPU_GetA34PETableAddress(void);

/* Initializes channel 0/1 mixing and interrupts, then enables CPU IRQs. */
void SPU_Init(void);

/* Caller supplies a valid volume index (0..15), channel (0..7), or speech index. */
/* format must be one of SPU_Format; speechIndex must exist in its resource table. */
void SPU_SetResAddr(unsigned channel, SPU_Format format,
                    unsigned sampleFrequency, unsigned speechIndex);
void SPU_Play(unsigned channel);
void USER_SPU_fptr_Volume(unsigned volumeIndex);
void SPU_StopPlay(unsigned channel);
void USER_SPU_SetPCM_sppAddr(unsigned speechIndex);
void USER_SPU_SetA34PE_sppAddr(unsigned speechIndex);

/* Legacy resource-address scratch words, owned by the audio module. */
extern unsigned DPTR_H;
extern unsigned DPTR_L;

#endif
