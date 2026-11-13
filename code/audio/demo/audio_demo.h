#ifndef _AUDIO_DEMO_H__
#define _AUDIO_DEMO_H__

// clang-format off
#define AUDIO_DEMO_BEAT_TICKS   256U    /* 256 RTC ticks @ 2048Hz = 125ms per 1/8 beat */
#define AUDIO_DEMO_MELODY_CH    SPU_Channel_2
#define AUDIO_DEMO_BASS_CH      SPU_Channel_3
// clang-format on

typedef enum
{
    AUDIO_Demo_OdeToJoy = 0,
    AUDIO_Demo_SuperMario = 1,
    AUDIO_Demo_Twinkle = 2,
    AUDIO_Demo_Count = 3
} AUDIO_Demo_Id;

typedef struct
{
    u16 melody_freq; /* Melody note frequency on SPU_Channel_2 (0 = rest) */
    u16 bass_freq;   /* Harmony/bass note frequency on SPU_Channel_3 (0 = rest) */
    u8 beats;        /* Duration in 125ms eighth-note units */
} AUDIO_Demo_Note;

void audio_demo_play(AUDIO_Demo_Id song_id);
void audio_demo_stop(void);
void audio_demo_cycle_next(void);
bool audio_demo_is_playing(void);

#endif /* _AUDIO_DEMO_H__ */
