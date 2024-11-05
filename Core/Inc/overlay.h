#pragma once
#include "lcd.h"

void overlay_fill(pixel_t color);
int overlay_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg);
