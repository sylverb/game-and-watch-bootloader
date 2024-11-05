#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "lcd.h"
#include "font_basic.h"
#include "overlay.h"

static uint16_t overlay_buffer[GW_LCD_WIDTH * 32];
static short font_size = 8;

void overlay_init()
{
    //    overlay_buffer = (uint16_t *)malloc(ODROID_SCREEN_WIDTH * 32 * 2, MEM_SLOW);
    //    odroid_overlay_set_font_size(odroid_settings_FontSize_get());
}

void overlay_fill(pixel_t color)
{
    pixel_t *dst = framebuffer;
    for (int x = 0; x < GW_LCD_WIDTH; x++)
    {
        for (int y = 0; y < GW_LCD_HEIGHT; y++)
        {
            dst[y * GW_LCD_WIDTH + x] = color;
        }
    }
}

void display_write_rect(short left, short top, short width, short height, short stride, const uint16_t *buffer)
{
    pixel_t *dest = framebuffer;

    for (short y = 0; y < height; y++)
    {
        if ((y + top) >= GW_LCD_WIDTH)
            return;
        pixel_t *dest_row = &dest[(y + top) * GW_LCD_WIDTH + left];
        memcpy(dest_row, &buffer[y * stride], width * sizeof(pixel_t));
    }
}

void display_write(short left, short top, short width, short height, const uint16_t *buffer)
{
    display_write_rect(left, top, width, height, width, buffer);
}

int overlay_get_font_size()
{
    return font_size;
}

int overlay_get_font_width()
{
    return 8;
}

int overlay_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int font_height = overlay_get_font_size();
    int font_width = overlay_get_font_width();
    int x_offset = 0;
    float scale = (float)font_height / 8;
    int text_len = strlen(text);

    for (int i = 0; i < (width / font_width); i++)
    {
        const char *glyph = font8x8_basic[(i < text_len) ? text[i] : ' '];
        for (int y = 0; y < font_height; y++)
        {
            int offset = x_offset + (width * y);
            for (int x = 0; x < 8; x++)
            {
                overlay_buffer[offset + x] = (glyph[(int)(y / scale)] & 1 << x) ? color : color_bg;
            }
        }
        x_offset += font_width;
    }

    display_write(x_pos, y_pos, width, font_height, overlay_buffer);

    return font_height;
}

int overlay_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int text_len = 1;
    int height = 0;

    if (text == NULL || text[0] == 0)
    {
        text = " ";
    }

    text_len = strlen(text);

    if (width < 1)
    {
        width = text_len * overlay_get_font_width();
    }

    if (width > (GW_LCD_WIDTH - x_pos))
    {
        width = (GW_LCD_WIDTH - x_pos);
    }

    int line_len = width / overlay_get_font_width();
    char buffer[line_len + 1];

    for (int pos = 0; pos < text_len;)
    {
        sprintf(buffer, "%.*s", line_len, text + pos);
        if (strchr(buffer, '\n'))
            *(strchr(buffer, '\n')) = 0;
        height += overlay_draw_text_line(x_pos, y_pos + height, width, buffer, color, color_bg);
        pos += strlen(buffer);
        if (*(text + pos) == 0 || *(text + pos) == '\n')
            pos++;
    }

    return height;
}