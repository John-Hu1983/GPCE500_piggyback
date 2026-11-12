#ifndef _GUI_TEXTBOX_H__
#define _GUI_TEXTBOX_H__

typedef enum
{
    GUI_Align_Left = 0,
    GUI_Align_Center = 1,
    GUI_Align_Right = 2
} GUI_Text_Align;

typedef struct
{
    s16 x;
    s16 y;
    u16 w;
    u16 h;
    u8 padding;
    const char *text;
    GUI_Font_Size font;
    GUI_Text_Align align;
    u16 bg_color;
    u16 border_color;
    u16 text_color;
    bool has_border;
    bool has_bg;
} GUI_Textbox;

void gui_textbox_init(GUI_Textbox *box, s16 x, s16 y, u16 w, u16 h, const char *text,
                      GUI_Font_Size font);
void gui_textbox_draw(const GUI_Textbox *box);
void gui_textbox_set_text(GUI_Textbox *box, const char *text);
void gui_textbox_set_int(GUI_Textbox *box, s32 value);

#endif /* _GUI_TEXTBOX_H__ */
