#ifndef _GUI_LABEL_H__
#define _GUI_LABEL_H__

typedef struct
{
    s16 x;
    s16 y;
    u16 w;
    u16 h;
    const char *text;
    GUI_Font_Size font;
    GUI_Text_Align align;
    u16 text_color;
    u16 bg_color;
    bool transparent;
} GUI_Label;

void gui_label_init(GUI_Label *lbl, s16 x, s16 y, u16 w, u16 h, const char *text,
                    GUI_Font_Size font);
void gui_label_draw(const GUI_Label *lbl);
void gui_label_set_text(GUI_Label *lbl, const char *text);
void gui_label_set_int(GUI_Label *lbl, s32 value);
void gui_label_set_color(GUI_Label *lbl, u16 text_color);

#endif /* _GUI_LABEL_H__ */
