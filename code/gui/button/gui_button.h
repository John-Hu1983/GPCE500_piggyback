#ifndef _GUI_BUTTON_H__
#define _GUI_BUTTON_H__

typedef enum
{
    GUI_Btn_Normal = 0,
    GUI_Btn_Focused = 1,
    GUI_Btn_Pressed = 2,
    GUI_Btn_Disabled = 3
} GUI_Btn_State;

typedef struct
{
    s16 x;
    s16 y;
    u16 w;
    u16 h;
    u8 radius;
    const char *label;
    GUI_Font_Size font;
    u16 bg_normal;
    u16 bg_pressed;
    u16 border_color;
    u16 focus_color;
    u16 text_color;
    GUI_Btn_State state;
} GUI_Button;

void gui_button_init(GUI_Button *btn, s16 x, s16 y, u16 w, u16 h, const char *label,
                     GUI_Font_Size font);
void gui_button_draw(const GUI_Button *btn);
void gui_button_set_state(GUI_Button *btn, GUI_Btn_State state);
void gui_button_set_label(GUI_Button *btn, const char *label);

#endif /* _GUI_BUTTON_H__ */
