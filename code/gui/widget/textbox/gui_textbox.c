#include "..\..\..\Include\spinmaster.h"

/*
 *@ brief    : Initialize a GUI_Textbox widget with default colors and alignment.
 *@ type     : public
 *@ argu in  : box: textbox descriptor; x, y: top-left coordinates; w, h: dimensions;
 *             text: initial text string; font: GUI_Font_Size selector.
 *@ argu out : Textbox initialized with bordered dark background and left alignment.
 *@ theme    : Standard text box / label initialization.
 */
void gui_textbox_init(GUI_Textbox *box, s16 x, s16 y, u16 w, u16 h, const char *text,
                      GUI_Font_Size font)
{
    if (box == null)
    {
        return;
    }
    box->x = x;
    box->y = y;
    box->w = w;
    box->h = h;
    box->padding = 3U;
    box->text = text;
    box->font = font;
    box->align = GUI_Align_Left;
    box->bg_color = GUI_COLOR_BLACK;
    box->border_color = GUI_COLOR_GRAY;
    box->text_color = GUI_COLOR_WHITE;
    box->has_border = true;
    box->has_bg = true;
}

/*
 *@ brief    : Render a GUI_Textbox widget onto the LCD without blanking the text band first.
 *@ type     : public
 *@ argu in  : box: pointer to textbox descriptor.
 *@ argu out : Border, non-overlapping margin strips, and opaque text rendered flicker-free.
 *@ theme    : Stream characters directly via gui_draw_str_bg and fill only surrounding margins.
 */
void gui_textbox_draw(const GUI_Textbox *box)
{
    s16 x1, y1, ix0, iy0, ix1, iy1, tx, ty, tx1, ty1;
    u16 str_w, char_h;

    if (box == null || box->w == 0U || box->h == 0U)
    {
        return;
    }

    x1 = (s16)(box->x + (s16)box->w - 1);
    y1 = (s16)(box->y + (s16)box->h - 1);

    if (box->has_border)
    {
        gui_draw_rect(box->x, box->y, x1, y1, box->border_color);
        ix0 = (s16)(box->x + 1);
        iy0 = (s16)(box->y + 1);
        ix1 = (s16)(x1 - 1);
        iy1 = (s16)(y1 - 1);
    }
    else
    {
        ix0 = box->x;
        iy0 = box->y;
        ix1 = x1;
        iy1 = y1;
    }

    if (box->text == null || box->text[0] == '\0')
    {
        if (box->has_bg && ix0 <= ix1 && iy0 <= iy1)
        {
            gui_fill_rect(ix0, iy0, ix1, iy1, box->bg_color);
        }
        return;
    }

    str_w = gui_font_get_str_width(box->text, box->font);
    char_h = gui_font_get_char_height(box->font);

    switch (box->align)
    {
    case GUI_Align_Center:
        tx = (s16)(box->x + ((s16)box->w - (s16)str_w) / 2);
        ty = (s16)(box->y + ((s16)box->h - (s16)char_h) / 2);
        break;

    case GUI_Align_Right:
        tx = (s16)(x1 - (s16)box->padding - (s16)str_w + 1);
        ty = (s16)(box->y + ((s16)box->h - (s16)char_h) / 2);
        break;

    case GUI_Align_Left:
    default:
        tx = (s16)(box->x + (s16)box->padding);
        ty = (s16)(box->y + (s16)box->padding);
        break;
    }

    if (tx < (s16)(box->x + (s16)box->padding))
    {
        tx = (s16)(box->x + (s16)box->padding);
    }
    if (ty < iy0)
    {
        ty = iy0;
    }

    if (box->has_bg)
    {
        /* Draw opaque text first so glyphs never flash blank, then fill surrounding margins */
        gui_draw_str_bg(tx, ty, box->text, box->text_color, box->bg_color, box->font);

        tx1 = (s16)(tx + (s16)str_w - 1);
        ty1 = (s16)(ty + (s16)char_h - 1);
        if (tx1 > ix1)
        {
            tx1 = ix1;
        }
        if (ty1 > iy1)
        {
            ty1 = iy1;
        }

        if (iy0 < ty)
        {
            gui_fill_rect(ix0, iy0, ix1, (s16)(ty - 1), box->bg_color);
        }
        if (ty1 < iy1)
        {
            gui_fill_rect(ix0, (s16)(ty1 + 1), ix1, iy1, box->bg_color);
        }
        if (ix0 < tx)
        {
            gui_fill_rect(ix0, ty, (s16)(tx - 1), ty1, box->bg_color);
        }
        if (tx1 < ix1)
        {
            gui_fill_rect((s16)(tx1 + 1), ty, ix1, ty1, box->bg_color);
        }
    }
    else
    {
        gui_draw_str_at(tx, ty, box->text, box->text_color, box->font);
    }
}

/*
 *@ brief    : Update textbox string and immediately redraw.
 *@ type     : public
 *@ argu in  : box: textbox descriptor; text: new string pointer.
 *@ argu out : Textbox updated and redrawn.
 *@ theme    : Dynamic text content update.
 */
void gui_textbox_set_text(GUI_Textbox *box, const char *text)
{
    if (box == null)
    {
        return;
    }
    box->text = text;
    gui_textbox_draw(box);
}

/*
 *@ brief    : Render a signed 32-bit integer inside the textbox using its alignment.
 *@ type     : public
 *@ argu in  : box: textbox descriptor; value: integer to display.
 *@ argu out : Box cleared and formatted integer drawn.
 *@ theme    : Convenient numeric readout box.
 */
void gui_textbox_set_int(GUI_Textbox *box, s32 value)
{
    char buf[12];
    u8 pos = 11U;
    u32 mag;
    const char *saved;

    if (box == null)
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

    saved = box->text;
    box->text = &buf[pos];
    gui_textbox_draw(box);
    box->text = saved;
}
