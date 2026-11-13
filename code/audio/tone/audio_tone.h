#ifndef _AUDIO_TONE_H__
#define _AUDIO_TONE_H__

// clang-format off
/* Standard channel volume masks for spu_play_tone() */
#define TONE_VOL_MUTE           0x0000U
#define TONE_VOL_LOW            0x3FFFU
#define TONE_VOL_MID            0x7FFFU
#define TONE_VOL_HIGH           0xBFFFU
#define TONE_VOL_MAX            0xFFFFU

/*
 * Musical note frequency register values for 16-sample-per-cycle waveforms:
 * SampleFreq = round(f_note_Hz * 16 * 2^24 / 40960000) = round(f_note_Hz * 6.5536)
 */
#define TONE_NOTE_REST          0x0000U

/* Octave 4 (Middle C = C4) */
#define TONE_NOTE_C4            0x06B3U /* 261.6 Hz */
#define TONE_NOTE_D4            0x0785U /* 293.7 Hz */
#define TONE_NOTE_E4            0x0870U /* 329.6 Hz */
#define TONE_NOTE_F4            0x08F1U /* 349.2 Hz */
#define TONE_NOTE_G4            0x0A09U /* 392.0 Hz */
#define TONE_NOTE_G4S           0x0AA2U /* 415.3 Hz */
#define TONE_NOTE_A4            0x0B44U /* 440.0 Hz */
#define TONE_NOTE_A4S           0x0BEFU /* 466.2 Hz */
#define TONE_NOTE_B4            0x0CA5U /* 493.9 Hz */

/* Octave 5 (Treble C = C5) */
#define TONE_NOTE_C5            0x0D65U /* 523.3 Hz */
#define TONE_NOTE_C5S           0x0E31U /* 554.4 Hz */
#define TONE_NOTE_D5            0x0F09U /* 587.3 Hz */
#define TONE_NOTE_D5S           0x0FEFU /* 622.3 Hz */
#define TONE_NOTE_E5            0x10E0U /* 659.3 Hz */
#define TONE_NOTE_F5            0x11E1U /* 698.5 Hz */
#define TONE_NOTE_F5S           0x12F2U /* 740.0 Hz */
#define TONE_NOTE_G5            0x1412U /* 784.0 Hz */
#define TONE_NOTE_G5S           0x1543U /* 830.6 Hz */
#define TONE_NOTE_A5            0x1687U /* 880.0 Hz */
#define TONE_NOTE_A5S           0x17DEU /* 932.3 Hz */
#define TONE_NOTE_B5            0x1949U /* 987.8 Hz */

/* Octave 6 (High C = C6) */
#define TONE_NOTE_C6            0x1ACAU /* 1046.5 Hz */
#define TONE_NOTE_D6            0x1E12U /* 1174.7 Hz */
#define TONE_NOTE_E6            0x21C1U /* 1318.5 Hz */
#define TONE_NOTE_F6            0x23C3U /* 1396.9 Hz */
#define TONE_NOTE_G6            0x2824U /* 1568.0 Hz */
#define TONE_NOTE_A6            0x2D0EU /* 1760.0 Hz */
// clang-format on

/* Built-in 16-bit PCM wave timbre resources in audio_tone.c */
typedef enum
{
    TONE_Wave_Pulse = 0, /* Crisp 12-cycle pulse wave for UI/game SFX */
    TONE_Wave_Piano = 1, /* Sustained 36-cycle music-box/piano wave for melodies */
    TONE_Wave_Bell = 2,  /* Warm 24-cycle triangle-bell wave for chords/chimes */
    TONE_Wave_Count = 3
} TONE_Wave_Id;

const unsigned *tone_get_wave_resource(TONE_Wave_Id wave_id);

#endif /* _AUDIO_TONE_H__ */
