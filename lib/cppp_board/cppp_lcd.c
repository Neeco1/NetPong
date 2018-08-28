#include "cppp_lcd.h"
#include "cppp_gfx.h"
#include "glcdfont.h"

static int16_t cursorX, cursorY;
static uint16_t textColor;
static uint8_t textSize;
static int textBackground;

uint16_t lcd_color565(const uint8_t r, const uint8_t g, const uint8_t b) {
  uint8_t highr = ( r & 0b11111000 ) >> 3;
  uint8_t highg = ( g & 0b11111100 ) >> 2;
  uint8_t highb = ( b & 0b11111000 ) >> 3;
  
  uint16_t result = (highr << 11) | (highg << 5) | (highb);
  return result;
}

void lcd_printPattern(const uint16_t backgroundColor, const uint16_t foregroundColor) {
  gfx_fillScreen(backgroundColor);
  
  size_t i, j;
  for(i=0; i < 480; i+=8) {
    for(j=0; j < 320; j+=8) {
      gfx_fillRect(i, j, 4, 4, foregroundColor);  
    }
  }
}

void lcd_initCursor(){
  lcd_setCursor(0,0);
  lcd_setTextColor(WHITE);
  lcd_setBackgroundColor(BLACK);
}
void lcd_initCursor_s(){
  lcd_setCursor(0,0);
  lcd_setTextColor(WHITE);
  lcd_setBackgroundColor(BLACK);
}

void lcd_setCursor(const int16_t x, int16_t y) {
  cursorX = x;
  cursorY = y;
}
void lcd_setCursor_s(const int16_t x, int16_t y) {
  cursorX = x;
  cursorY = y;
}

void lcd_setTextColor(const uint16_t c) {
  // For 'transparent' background, we'll set the bg to the same as the rest of the display
  textColor = c;
}
void lcd_setTextColor_s(const uint16_t c) {
  // For 'transparent' background, we'll set the bg to the same as the rest of the display
  textColor = c;
}

void lcd_setTextSize(const uint8_t s) {
  textSize = s;
}
void lcd_setTextSize_s(const uint8_t s) {
  textSize = s;
}

void lcd_setBackgroundColor(const int bg){
  textBackground = bg;
}

void lcd_drawChar(const int x, const int y, const char c, const int color, const int bg, const char size) {
  lcd_setTextColor(color);
  lcd_setBackgroundColor(bg);
  lcd_setTextSize(size);
  
  // if (x,y) is not inside the display return
  if(x >= 480 || y >= 320) {
    return; 
  }
  
  char i, j;
  for(i=0; i<6; i++ ) 
  {  
    // draw in x-direction
    char line;
    if(i < 5)
      line = font[c*5+i];
    else
      line = 0;
    
    for(j=0; j<8; j++, line >>= 1) 
    {  
      // draw in y-direction     
      uint16_t currentColor; 
      if((line & 0x1) == 1) 
      {                                      
        currentColor = color;  
      } 
      else 
      {
        currentColor = bg; 
      }
        
      if(textSize == 1) 
      {
        gfx_drawPixel(x + i, y + (7-j), currentColor);    
      } else {
        gfx_fillRect(x + i*textSize, y + (7-j)*textSize, textSize, textSize, currentColor);  
      }
    }
  }
}

void lcd_writeAuto(const char c) {
  // if char c == '\n' then jump cursor to next line
  if(c == '\n') 
  {
    cursorX = 0;
    cursorY += 8 * textSize;  
  }
  else
  {
    //check for end of line and reset cursor. 
    //in the case of end of the display set cursorY to the starting point.    
    if(cursorX > 480-7*textSize) {
      cursorX = 0;  
      cursorY += 8*textSize;
    }                                
    if(cursorY > 320-7*textSize) {
      cursorY = 0;  
    }
    // draw the char
    lcd_drawChar(cursorX, cursorY, c, textColor, textBackground, textSize);
    // move cursorX
    cursorX = cursorX + 7*textSize;
  }
}

void lcd_writeText(const char *text){
  char *textPtr = (char *)text;
  
  while(*textPtr != '\0') {
    lcd_writeAuto(*textPtr); 
    textPtr++;
  }
}
void lcd_writeText_s(const char *text){
  char *textPtr = (char *)text;
  
  while(*textPtr != '\0') {
    lcd_writeAuto(*textPtr); 
    textPtr++;
  }
}

void lcd_writeTextln(const char *text){
    // your code here ... 
}
void lcd_writeTextln_s(const char *text){
    // your code here ... 
}

void lcd_writeNumberOnDisplay(const uint16_t *value){
    // your code here...
}
void lcd_writeNumberOnDisplay_s(const uint16_t *value){
    // your code here...
}
