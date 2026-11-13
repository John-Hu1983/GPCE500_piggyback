#ifndef _GUI_CHECKBOX_H__
#define _GUI_CHECKBOX_H__

typedef enum
{
    GUI_Check_Box = 0,  /* Square checkbox with checkmark */
    GUI_Check_Radio = 1 /* Circular radio button with inner dot */
} GUI_Check_Style;

typedef struct
{
    s16 x;
    s16 y;
    u8 box_size;
    bool checked;
    bool focused;
    GUI_Check_Style style;
    const char *label;
    GUI_Font_Size font;
    u16 box_bg_color;
    u16 border_color;
    u16 check_color;
    u16 text_color;
    u16 page_bg_color;
} GUI_Checkbox;

void gui_checkbox_init(GUI_Checkbox *cb, s16 x, s16 y, const char *label, GUI_Font_Size font,
                       bool checked);
void gui_checkbox_draw(const GUI_Checkbox *cb);
void gui_checkbox_set_checked(GUI_Checkbox *cb, bool checked);
void gui_checkbox_toggle(GUI_Checkbox *cb);

#endif /* _GUI_CHECKBOX_H__ */
