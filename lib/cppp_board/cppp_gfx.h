#ifndef GFX_H
#define GFX_H

#include <stdlib.h>
#include <stdio.h>
#include "cppp_pins.h"
#include "pdl_header.h"
#include "glcdfont.h"

#define WIDTH   480
#define HEIGHT  320

void gfx_fillScreen(int16_t color);
void gfx_drawFastHLine(int x, int y, int length, int16_t color);
void gfx_drawFastVLine(int x, int y, int length, int16_t color);
void gfx_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void gfx_fillRect(int x1, int y1, int w, int h, int16_t fillcolor);
void gfx_drawPixel(int16_t x, int16_t y, uint16_t color);
void gfx_drawCircle(int x0, int y0, int r, unsigned int color);
void gfx_drawCircleHelper( int x0, int y0, int r, char cornername, unsigned int color);
void gfx_fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
void gfx_fillCircle(int x0, int y0, int r, unsigned int color);
void gfx_drawRoundRect(int x, int y, int w, int h, int r, unsigned int color);
void gfx_fillRoundRect(int x, int y, int w, int h, int r, unsigned color);
void gfx_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void gfx_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void gfx_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);


void gfx_write8BitValueOnLCD(const uint8_t *value);
void gfx_write16BitValueOnLCD(const uint16_t *value);
void gfx_write32BitValueOnLCD(const uint32_t *value);
void gfx_write3Digits8Bit(const uint8_t *value);
void gfx_write3Digits16Bit(const uint16_t *value);

#endif
