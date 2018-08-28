#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include "s6e2ccxj.h" 

#define FRAME_WIDTH   480
#define FRAME_HEIGHT  320

#define BTN_SPACE_TOP 10
#define BTN_SPACE_RIGHT 20
#define BTN_SPACE_BOTTOM 10
#define BTN_SPACE_LEFT 20

void fb_fill(uint8_t color);

void fb_setPixel(uint16_t x, uint16_t y, uint8_t color);

void fb_fillRectangle(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint8_t color);
void fb_drawRectangle(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint8_t thickness, uint8_t color);

void fb_fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void fb_drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

void fb_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void fb_drawVLine(uint16_t x, uint16_t y1, uint16_t y2, uint16_t color);
void fb_drawHLine(uint16_t y, uint16_t x1, uint16_t x2, uint16_t color);

void fb_drawButton(uint16_t x, uint16_t y, const char * text, uint16_t txtColor, uint16_t borderColor, uint16_t bgColor, uint8_t borderWidth);
void fb_drawButtonCentered(uint16_t y, const char * text, uint16_t txtColor, uint16_t borderColor, uint16_t bgColor, uint8_t borderWidth);

void fb_writeChar(uint16_t x, uint16_t y, char c, uint16_t txtColor, uint16_t bgColor, uint8_t txtSize);
void fb_writeText(uint16_t x, uint16_t y, const char * text, uint16_t txtColor, uint16_t bgColor, uint8_t txtSize);
void fb_writeTextCentered(uint16_t y, const char * text, uint16_t txtColor, uint16_t bgColor, uint8_t txtSize);
void fb_writeTextRight(uint16_t y, const char * text, uint16_t txtColor, uint16_t bgColor, uint8_t txtSize);
void fb_writeNumber(uint16_t x, uint16_t y, const int * value, uint16_t txtColor, uint16_t bgColor, uint8_t txtSize);

void fb_sendToDisplay(void);

#endif
