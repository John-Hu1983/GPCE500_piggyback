#ifndef _GUI_GAME_H__
#define _GUI_GAME_H__

// clang-format off
#define GAME_TICK_PERIOD        64U     /* 64 RTC ticks @ 2048Hz = 31.25ms (32 FPS) */
#define GAME_HUD_H              14U
#define GAME_PLAY_Y0            16U
#define GAME_BG_COLOR           GUI_RGB565(12, 16, 26)
#define GAME_HUD_BG             GUI_RGB565(24, 34, 56)
#define GAME_BORDER_COLOR       GUI_RGB565(56, 88, 136)

/* SPU_Channel_1 chiptune tone frequency presets */
#define GAME_TONE_LOW           0x0700U
#define GAME_TONE_MOVE          0x1100U
#define GAME_TONE_HIT           0x1800U
#define GAME_TONE_SCORE         0x2200U
#define GAME_TONE_HIGH          0x2C00U
// clang-format on

typedef enum
{
    GUI_Game_Snake = 0,
    GUI_Game_Tetris = 1,
    GUI_Game_Brick = 2,
    GUI_Game_Flappy = 3,
    GUI_Game_Shooter = 4,
    GUI_Game_2048 = 5,
    GUI_Game_Count = 6
} GUI_Game_Id;

typedef struct
{
    GUI_Label top_bar;
    GUI_Label bottom_bar;
    char heap_info_str[32];
    GUI_Button buttons[GUI_Game_Count];
    u8 selected_idx;
} GUI_Game_Menu_Page;

/* 1st-level Game Submenu APIs */
void gui_game_menu_enter(void);
void gui_game_menu_exit(void);
bool gui_game_menu_is_active(void);

/* Active game lifecycle APIs */
void gui_game_enter(GUI_Game_Id id);
void gui_game_exit(void);
bool gui_game_is_active(void);

/* Shared game engine utilities used by individual game modules */
u16 gui_game_rand(void);
void gui_game_sfx(unsigned freq);
void gui_game_draw_hud(u16 score);
void gui_game_update_score(u16 score);
void gui_game_draw_over_banner(const char *msg);

#endif /* _GUI_GAME_H__ */
