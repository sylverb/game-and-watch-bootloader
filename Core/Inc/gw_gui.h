#pragma once

#include "gw_lcd.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t width;
    uint16_t height;
    char logo[];
} logo_image;

#define RED_COMPONENT(c)     (((c) & 0xF800) >> 11)
#define GREEN_COMPONENT(c)   (((c) & 0x07E0) >> 5)
#define BLUE_COMPONENT(c)    ((c) & 0x001F)

#define RGB24_TO_RGB565(r, g, b) ((pixel_t)( ((r) >> 3) << 11 ) | ( ((g) >> 2) << 5 ) | ( (b) >> 3 ))
#define DARKEN(c, multiplier)  (pixel_t)((((pixel_t)(RED_COMPONENT(c) * multiplier) & 0x1F) << 11) | \
                                     (((pixel_t)(GREEN_COMPONENT(c) * multiplier) & 0x3F) << 5) | \
                                     ((pixel_t)(BLUE_COMPONENT(c) * multiplier) & 0x1F))

#define GUI_BACKGROUND_COLOR RGB24_TO_RGB565(0x00, 0x00, 0x00)
#define GUI_SEGMENT_INACTIVE_COLOR DARKEN(GUI_BACKGROUND_COLOR, 0.8)
#define GUI_SEGMENT_ACTIVE_COLOR 0x0000

void gw_gui_fill(pixel_t color);
void gw_gui_draw_text(int x, int y, const char *text, pixel_t color);
void gw_gui_draw_progress_bar(int x, int y, int width, int height, int percent, pixel_t border_color, pixel_t fill_color);
void gw_gui_draw();
