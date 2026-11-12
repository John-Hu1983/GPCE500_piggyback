#include "..\..\Include\spinmaster.h"

/*
 *@ brief    : Initialize a lightweight GUI_Label widget.
 *@ type     : public
 *@ argu in  : lbl: label descriptor; x, y: top-left coordinates; w, h: bounding area (0 = auto);
 *             text: string pointer; font: GUI_Font_Size selector.
 *@ argu out : Label initialized with left alignment and white text.
 *@ theme    : Lightweight static/dynamic text label without textbox borders or padding.
 */
void gui_label_init(GUI_Label *lbl, s16 x, s16 y, u16 w, u16 h, const char *text,
                    GUI_Font_Size font)
{
    if (lbl == null)
    {
        return;
    }
    lbl->x = x;
    lbl->y = y;
    lbl->w = w;
    lbl->h = (h == 0U) ? gui_font_get_char_height(font) : h;
    lbl->text = text;
    lbl->font = font;
    lbl->align = GUI_Align_Left;
    lbl->text_color = GUI_COLOR_WHITE;
    lbl->bg_color = GUI_COLOR_BLACK;
    lbl->transparent = false;
}

/*
 *@ brief    : Render a GUI_Label widget onto the LCD.
 *@ type     : public
 *@ argu in  : lbl: pointer to label descriptor.
 *@ argu out : Label text rendered according to alignment and transparency mode.
 *@ theme    : When transparent is false and w > 0, clears the bounding area for flicker-free updates.
 */
void gui_label_draw(const GUI_Label *lbl)
{
    s16 tx, ty;
    u16 str_w, char_h;

    if (lbl == null)
    {
        return;
    }

    char_h = gui_font_get_char_height(lbl->font);

    if (!lbl->transparent && lbl->w > 0U && lbl->h > 0U)
    {
        gui_fill_rect(lbl->x, lbl->y, (s16)(lbl->x + (s16)lbl->w - 1),
                      (s16)(lbl->y + (s16)lbl->h - 1), lbl->bg_color);
    }

    if (lbl->text == null)
    {
        return;
    }

    str_w = gui_font_get_str_width(lbl->text, lbl->font);
    tx = lbl->x;
    ty = lbl->y;

    if (lbl->w > 0U)
    {
        if (lbl->align == GUI_Align_Center)
        {
            tx = (s16)(lbl->x + ((s16)lbl->w - (s16)str_w) / 2);
        }
        else if (lbl->align == GUI_Align_Right)
        {
            tx = (s16)(lbl->x + (s16)lbl->w - (s16)str_w);
        }
        if (tx < lbl->x)
        {
            tx = lbl->x;
        }
    }

    if (lbl->h > char_h)
    {
        ty = (s16)(lbl->y + ((s16)lbl->h - (s16)char_h) / 2);
    }

    if (lbl->transparent || lbl->w > 0U)
    {
        gui_draw_str_at(tx, ty, lbl->text, lbl->text_color, lbl->font);
    }
    else
    {
        gui_draw_str_bg(tx, ty, lbl->text, lbl->text_color, lbl->bg_color, lbl->font);
    }
}

/*
 *@ brief    : Update label string and immediately redraw.
 *@ type     : public
 *@ argu in  : lbl: label descriptor; text: new string pointer.
 *@ argu out : Label text updated and redrawn.
 *@ theme    : Dynamic label caption update.
 */
void gui_label_set_text(GUI_Label *lbl, const char *text)
{
    if (lbl == null)
    {
        return;
    }
    lbl->text = text;
    gui_label_draw(lbl);
}

/*
 *@ brief    : Format and render a signed 32-bit integer on the label.
 *@ type     : public
 *@ argu in  : lbl: label descriptor; value: signed integer value.
 *@ argu out : Formatted integer rendered on the label.
 *@ theme    : Dynamic numeric counter label without sprintf.
 */
void gui_label_set_int(GUI_Label *lbl, s32 value)
{
    char buf[12];
    u8 pos = 11U;
    u32 mag;
    const char *saved;

    if (lbl == null)
    {
        return;
    }

    buf[pos] = '\0';
    if (value < 0L)
    {
        mag = (u32)(-(value + 1L)) + 1UL;
    }
    else
    {
        mag = (u32)value;
    }

    do
    {
        buf[--pos] = (char)('0' + (mag % 10UL));
        mag /= 10UL;
    } while (mag != 0UL && pos > 1U);

    if (value < 0L)
    {
        buf[--pos] = '-';
    }

    saved = lbl->text;
    lbl->text = &buf[pos];
    gui_label_draw(lbl);
    lbl->text = saved;
}

/*
 *@ brief    : Update label text color and immediately redraw.
 *@ type     : public
 *@ argu in  : lbl: label descriptor; text_color: new RGB565 foreground color.
 *@ argu out : Color updated and label redrawn.
 *@ theme    : Dynamic highlight / status color change.
 */
void gui_label_set_color(GUI_Label *lbl, u16 text_color)
{
    if (lbl == null)
    {
        return;
    }
    lbl->text_color = text_color;
    gui_label_draw(lbl);
}
