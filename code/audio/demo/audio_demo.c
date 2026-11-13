#include "..\..\Include\spinmaster.h"

/*
 * Song 0: Beethoven's "Ode to Joy" (Two-channel polyphonic: Melody + Bass harmony)
 */
static const AUDIO_Demo_Note demo_song_ode[] = {
    {TONE_NOTE_E5, TONE_NOTE_C4, 2U}, {TONE_NOTE_E5, 0U, 2U},
    {TONE_NOTE_F5, TONE_NOTE_A4, 2U}, {TONE_NOTE_G5, 0U, 2U},
    {TONE_NOTE_G5, TONE_NOTE_E4, 2U}, {TONE_NOTE_F5, 0U, 2U},
    {TONE_NOTE_E5, TONE_NOTE_G4, 2U}, {TONE_NOTE_D5, 0U, 2U},
    {TONE_NOTE_C5, TONE_NOTE_C4, 2U}, {TONE_NOTE_C5, 0U, 2U},
    {TONE_NOTE_D5, TONE_NOTE_G4, 2U}, {TONE_NOTE_E5, 0U, 2U},
    {TONE_NOTE_E5, TONE_NOTE_C4, 3U}, {TONE_NOTE_D5, 0U, 1U},
    {TONE_NOTE_D5, TONE_NOTE_G4, 4U},

    {TONE_NOTE_E5, TONE_NOTE_C4, 2U}, {TONE_NOTE_E5, 0U, 2U},
    {TONE_NOTE_F5, TONE_NOTE_A4, 2U}, {TONE_NOTE_G5, 0U, 2U},
    {TONE_NOTE_G5, TONE_NOTE_E4, 2U}, {TONE_NOTE_F5, 0U, 2U},
    {TONE_NOTE_E5, TONE_NOTE_G4, 2U}, {TONE_NOTE_D5, 0U, 2U},
    {TONE_NOTE_C5, TONE_NOTE_F4, 2U}, {TONE_NOTE_C5, 0U, 2U},
    {TONE_NOTE_D5, TONE_NOTE_G4, 2U}, {TONE_NOTE_E5, 0U, 2U},
    {TONE_NOTE_D5, TONE_NOTE_G4, 3U}, {TONE_NOTE_C5, 0U, 1U},
    {TONE_NOTE_C5, TONE_NOTE_C4, 4U},

    {TONE_NOTE_D5, TONE_NOTE_G4, 2U}, {TONE_NOTE_D5, 0U, 2U},
    {TONE_NOTE_E5, TONE_NOTE_C4, 2U}, {TONE_NOTE_C5, 0U, 2U},
    {TONE_NOTE_D5, TONE_NOTE_G4, 2U}, {TONE_NOTE_E5, 0U, 1U},
    {TONE_NOTE_F5, 0U, 1U},           {TONE_NOTE_E5, TONE_NOTE_C4, 2U},
    {TONE_NOTE_C5, 0U, 2U},           {TONE_NOTE_D5, TONE_NOTE_G4, 2U},
    {TONE_NOTE_E5, 0U, 1U},           {TONE_NOTE_F5, 0U, 1U},
    {TONE_NOTE_E5, TONE_NOTE_A4, 2U}, {TONE_NOTE_D5, 0U, 2U},
    {TONE_NOTE_C5, TONE_NOTE_F4, 2U}, {TONE_NOTE_D5, TONE_NOTE_G4, 2U},
    {TONE_NOTE_G4, TONE_NOTE_G4, 4U},

    {TONE_NOTE_E5, TONE_NOTE_C4, 2U}, {TONE_NOTE_E5, 0U, 2U},
    {TONE_NOTE_F5, TONE_NOTE_A4, 2U}, {TONE_NOTE_G5, 0U, 2U},
    {TONE_NOTE_G5, TONE_NOTE_E4, 2U}, {TONE_NOTE_F5, 0U, 2U},
    {TONE_NOTE_E5, TONE_NOTE_G4, 2U}, {TONE_NOTE_D5, 0U, 2U},
    {TONE_NOTE_C5, TONE_NOTE_F4, 2U}, {TONE_NOTE_C5, 0U, 2U},
    {TONE_NOTE_D5, TONE_NOTE_G4, 2U}, {TONE_NOTE_E5, 0U, 2U},
    {TONE_NOTE_D5, TONE_NOTE_G4, 3U}, {TONE_NOTE_C5, 0U, 1U},
    {TONE_NOTE_C5, TONE_NOTE_C4, 4U}};

/*
 * Song 1: Classic "Super Mario Bros." Overworld Theme (Melody + Bass)
 */
static const AUDIO_Demo_Note demo_song_mario[] = {
    {TONE_NOTE_E6, TONE_NOTE_D4, 1U}, {TONE_NOTE_E6, TONE_NOTE_D4, 2U},
    {TONE_NOTE_E6, TONE_NOTE_D4, 2U}, {TONE_NOTE_C6, TONE_NOTE_D4, 1U},
    {TONE_NOTE_E6, TONE_NOTE_D4, 2U}, {TONE_NOTE_G6, TONE_NOTE_G4, 4U},
    {TONE_NOTE_G5, TONE_NOTE_G4, 4U},

    {TONE_NOTE_C6, TONE_NOTE_G4, 3U}, {TONE_NOTE_G5, TONE_NOTE_E4, 3U},
    {TONE_NOTE_E5, TONE_NOTE_C4, 3U}, {TONE_NOTE_A5, TONE_NOTE_F4, 2U},
    {TONE_NOTE_B5, TONE_NOTE_G4, 2U}, {TONE_NOTE_A5S, TONE_NOTE_F4, 1U},
    {TONE_NOTE_A5, TONE_NOTE_F4, 2U}, {TONE_NOTE_G5, TONE_NOTE_E4, 2U},
    {TONE_NOTE_E6, TONE_NOTE_C5, 2U}, {TONE_NOTE_G6, TONE_NOTE_E5, 2U},
    {TONE_NOTE_A6, TONE_NOTE_F5, 2U}, {TONE_NOTE_F6, TONE_NOTE_D5, 1U},
    {TONE_NOTE_G6, TONE_NOTE_E5, 2U}, {TONE_NOTE_E6, TONE_NOTE_C5, 2U},
    {TONE_NOTE_C6, TONE_NOTE_A4, 1U}, {TONE_NOTE_D6, TONE_NOTE_B4, 1U},
    {TONE_NOTE_B5, TONE_NOTE_G4, 3U}};

/*
 * Song 2: "Twinkle Twinkle Little Star" Music-Box Duet
 */
static const AUDIO_Demo_Note demo_song_twinkle[] = {
    {TONE_NOTE_C5, TONE_NOTE_C4, 2U}, {TONE_NOTE_C5, TONE_NOTE_E4, 2U},
    {TONE_NOTE_G5, TONE_NOTE_G4, 2U}, {TONE_NOTE_G5, TONE_NOTE_E4, 2U},
    {TONE_NOTE_A5, TONE_NOTE_F4, 2U}, {TONE_NOTE_A5, TONE_NOTE_C4, 2U},
    {TONE_NOTE_G5, TONE_NOTE_E4, 4U},

    {TONE_NOTE_F5, TONE_NOTE_D4, 2U}, {TONE_NOTE_F5, TONE_NOTE_G4, 2U},
    {TONE_NOTE_E5, TONE_NOTE_C4, 2U}, {TONE_NOTE_E5, TONE_NOTE_G4, 2U},
    {TONE_NOTE_D5, TONE_NOTE_F4, 2U}, {TONE_NOTE_D5, TONE_NOTE_G4, 2U},
    {TONE_NOTE_C5, TONE_NOTE_C4, 4U}};

static SWT_Timer demo_timer;
static const AUDIO_Demo_Note *demo_cur_notes = null;
static u16 demo_note_count = 0U;
static u16 demo_note_idx = 0U;
static u8 demo_beats_left = 0U;
static u8 demo_cycle_state = 0U;
static bool demo_playing = false;

/*
 *@ brief    : Trigger the current note pair (melody + bass) on SPU_Channel_2 and SPU_Channel_3.
 *@ type     : private
 *@ argu in  : note: pointer to the AUDIO_Demo_Note to play.
 *@ argu out : Melody played with TONE_Wave_Piano; bass played with TONE_Wave_Bell.
 *@ theme    : Two-channel polyphonic note dispatch.
 */
static void _audio_demo_trigger_note(const AUDIO_Demo_Note *note)
{
    if (note->melody_freq != 0U)
    {
        spu_play_tone(AUDIO_DEMO_MELODY_CH, TONE_Wave_Piano, note->melody_freq, TONE_VOL_MAX);
    }
    if (note->bass_freq != 0U)
    {
        spu_play_tone(AUDIO_DEMO_BASS_CH, TONE_Wave_Bell, note->bass_freq, TONE_VOL_HIGH);
    }
}

/*
 *@ brief    : Software timer callback advancing the demo melody beat by beat.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Next note triggered when current note's beat counter expires; loops at end of song.
 *@ theme    : Non-blocking background music sequencer.
 */
static void _audio_demo_on_tick(void)
{
    if (!demo_playing || demo_cur_notes == null || demo_note_count == 0U)
    {
        return;
    }

    if (demo_beats_left > 1U)
    {
        demo_beats_left--;
        return;
    }

    demo_note_idx++;
    if (demo_note_idx >= demo_note_count)
    {
        demo_note_idx = 0U;
    }

    demo_beats_left = demo_cur_notes[demo_note_idx].beats;
    _audio_demo_trigger_note(&demo_cur_notes[demo_note_idx]);
}

/*
 *@ brief    : Start playing the specified polyphonic demo melody in the background.
 *@ type     : public
 *@ argu in  : song_id: AUDIO_Demo_OdeToJoy, AUDIO_Demo_SuperMario, or AUDIO_Demo_Twinkle.
 *@ argu out : First note triggered immediately and beat timer started.
 *@ theme    : Public entry point for demo song playback.
 */
void audio_demo_play(AUDIO_Demo_Id song_id)
{
    audio_demo_stop();

    switch (song_id)
    {
    case AUDIO_Demo_SuperMario:
        demo_cur_notes = demo_song_mario;
        demo_note_count = (u16)(sizeof(demo_song_mario) / sizeof(demo_song_mario[0]));
        break;
    case AUDIO_Demo_Twinkle:
        demo_cur_notes = demo_song_twinkle;
        demo_note_count = (u16)(sizeof(demo_song_twinkle) / sizeof(demo_song_twinkle[0]));
        break;
    case AUDIO_Demo_OdeToJoy:
    default:
        demo_cur_notes = demo_song_ode;
        demo_note_count = (u16)(sizeof(demo_song_ode) / sizeof(demo_song_ode[0]));
        break;
    }

    demo_note_idx = 0U;
    demo_beats_left = demo_cur_notes[0].beats;
    demo_playing = true;

    _audio_demo_trigger_note(&demo_cur_notes[0]);
    swt_start_timer(&demo_timer, AUDIO_DEMO_BEAT_TICKS, SWT_Repeat, _audio_demo_on_tick);
}

/*
 *@ brief    : Stop the currently playing demo melody and silence its SPU channels.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Beat timer stopped and SPU_Channel_2 / SPU_Channel_3 silenced.
 *@ theme    : Clean stop for demo sequencer.
 */
void audio_demo_stop(void)
{
    swt_stop_timer(&demo_timer);
    spu_stop_channel(AUDIO_DEMO_MELODY_CH);
    spu_stop_channel(AUDIO_DEMO_BASS_CH);
    demo_playing = false;
}

/*
 *@ brief    : Cycle through the 3 demo songs and stop on the 4th press (Ode -> Mario -> Twinkle -> Off).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Next demo song started or playback stopped.
 *@ theme    : One-button music demo control for Desktop Music card.
 */
void audio_demo_cycle_next(void)
{
    if (demo_cycle_state < (u8)AUDIO_Demo_Count)
    {
        audio_demo_play((AUDIO_Demo_Id)demo_cycle_state);
        demo_cycle_state++;
    }
    else
    {
        audio_demo_stop();
        demo_cycle_state = 0U;
    }
}

/*
 *@ brief    : Check whether a demo song is currently playing.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : true if playing, false if stopped.
 *@ theme    : State query helper.
 */
bool audio_demo_is_playing(void) { return demo_playing; }
