#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

/**
 * Converts RGB-888 color to RGB-565.
 */
uint16_t lcd_color565(const uint8_t r, const uint8_t g, const uint8_t b);
uint16_t lcd_color565_s(const uint8_t r, const uint8_t g, const uint8_t b);

/**
 * Prints a chess pattern on the display.
 */
void lcd_printPattern(const uint16_t backgroundColor, const uint16_t foregroundColor);
void lcd_printPattern_s(const uint16_t backgroundColor, const uint16_t foregroundColor);

/**
 * Initializes cursor properties.
 */
void lcd_initCursor(void);
void lcd_initCursor_s(void);

/**
 * Sets the textwriter cursor.
 */
void lcd_setCursor(const int16_t x, int16_t y);
void lcd_setCursor_s(const int16_t x, int16_t y);

/**
 * Sets the color of the font in RGB-565 format.
 */
void lcd_setTextColor(const uint16_t color);
void lcd_setTextColor_s(const uint16_t color);

/**
 * Sets the size of the font.
 */
void lcd_setTextSize(const uint8_t s);
void lcd_setTextSize_s(const uint8_t s);

/**
 * Sets the color of the font background.
 */
void lcd_setBackgroundColor(const int bg);
void lcd_setBackgroundColor_s(const int bg);

/**
 * Draws a char c on (x,y).
 * The character is shown in color c, and its background is colored using color bg.
 */
void lcd_drawChar(const int x, const int y,  const char c, const int color,  const int bg, const char size);
void lcd_drawChar_s(const int x, const int y, const char c, const int color, const int bg, const char size);


/**
 * Draws a char c on the actual cursor position and updates the cursor afterwards.
 */
void lcd_writeAuto(const char c);
void lcd_writeAuto_s(const char c);

/**
 * Draws a strings on the display with automated cursor update.
 */
void lcd_writeText(const char *text);
void lcd_writeText_s(const char *text);

/**
 * Draws a string on the display with automated cursor update and linebreak.
 */
void lcd_writeTextln(const char *text);
void lcd_writeTextln_s(const char *text);

/**
 *    Write a 16 bit variable on the display.
 */
void lcd_writeNumberOnDisplay(const uint16_t *value);
void lcd_writeNumberOnDisplay_s(const uint16_t *value);

#endif
