#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mcu.h"
#include "pdl_header.h"
#include "cppp_lcd_registers.h"
#include "cppp_pins.h"
#include "cppp_adc.h"


#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define WIDTH   480
#define HEIGHT  320

extern struct TouchPoint{
  uint16_t x;
  uint16_t y;
  uint16_t z;
} touchPoint;  

void lcd_delay(int n);
void lcd_writeStrobe(void);
void lcd_write8(char d);
void lcd_write32(char r, long d);
void lcd_writeRegisterPair(char aH, char aL, int d);
void lcd_writeRegister8(char a, char d);
void lcd_setAddrWindow(int x1, int y1, int x2, int y2);
void lcd_init(void);
void lcd_reset(void);
void lcd_setup(void);
void lcd_setLR(void);
void lcd_flood(int color, long len);

#endif /* LCD_H */
