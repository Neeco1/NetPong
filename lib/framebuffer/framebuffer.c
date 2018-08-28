#include "framebuffer/framebuffer.h"
#include "cppp_board/cppp_gfx.h"
#include "cppp_board/cppp_lcd_driver.h"
#include "cppp_board/cppp_lcd.h"
#include <string.h>
#include <stdio.h>
#include "cppp_board/glcdfont.h"

uint8_t _frame[FRAME_WIDTH][FRAME_HEIGHT];

void fb_sendToDisplay() {
    
    lcd_setAddrWindow(0, 0, FRAME_WIDTH-1, FRAME_HEIGHT-1);
    
    //Tell display data is coming
    LCD_CS = 0u;
    //Tell display the next data is a command
    LCD_CD = 0u;
    //Command to write to the GRAM of the LCD
    lcd_write8(0x2C);
    //Tell display the next data is image data
    LCD_CD = 1u;
    
    //Write actual pixel data    
    for(uint16_t horizontal = 0; horizontal < FRAME_WIDTH; ++horizontal)
    {
        for(uint16_t vertical = 0; vertical < FRAME_HEIGHT; ++vertical) 
        {
            //Write upper 8 bits of color
            LCD_DATA = _frame[horizontal][vertical];
            LCD_WR = 0u;
            LCD_WR = 1u; 
            LCD_DATA = _frame[horizontal][vertical];
            LCD_WR = 0u;
            LCD_WR = 1u; 
        }
    }
    //Release display
    //LCD_CS = 1u;
    lcd_setLR();
}

void fb_fill(uint8_t color) {
    memset(_frame, color, FRAME_WIDTH*FRAME_HEIGHT);
}

void fb_setPixel(uint16_t x, uint16_t y, uint8_t color) {
    if( (x < FRAME_WIDTH) && (y < FRAME_HEIGHT) )
    {
        _frame[x][y] = color;
    }
}

void fb_fillRectangle(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint8_t color) {
    uint16_t x2 = x1+w;
    uint16_t y2 = y1+h;

    if( (x1 < FRAME_WIDTH) && (y1 < FRAME_HEIGHT) )
    {
        //Check if size would overlap
        if( (x2 >= FRAME_WIDTH) ) { x2 = FRAME_WIDTH; }
        if( (y2 >= FRAME_HEIGHT) ) { y2 = FRAME_HEIGHT; }
        
        for(uint16_t vertical = y1; vertical < y2; ++vertical)
        {
            for(uint16_t horizontal = x1; horizontal < x2; ++horizontal)
            {
                _frame[horizontal][vertical] = color;
            }
        }
    }
}
void fb_drawRectangle(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint8_t thickness, uint8_t color) {
    uint16_t x2 = x1+w;
    uint16_t y2 = y1+h;

    if( (x1 < FRAME_WIDTH) && (y1 < FRAME_HEIGHT) )
    {
        //Check if size would overlap, clip if necessary
        if( (x2 >= FRAME_WIDTH) ) { x2 = FRAME_WIDTH-thickness; }
        if( (y2 >= FRAME_HEIGHT) ) { y2 = FRAME_HEIGHT-thickness; }
        
        //Draw upper line (multiple if thickness > 1)
        for(uint8_t lineCount = 0; lineCount < thickness; ++lineCount)
        {
            uint16_t curY = y1+lineCount;
            for(uint16_t horizontal = x1; horizontal < x2+thickness; ++horizontal)
            {
                _frame[horizontal][curY] = color;
            }
        }
        
        //Draw vertical pixels (multiple if thickness > 1)
        for(uint8_t lineCount = 0; lineCount < thickness; ++lineCount)
        {
            uint16_t curX1 = x1+lineCount;
            uint16_t curX2 = x2+lineCount;
            for(uint16_t vertical = y1+thickness; vertical < y2; ++vertical)
            {
                _frame[curX1][vertical] = color;
                _frame[curX2][vertical] = color;
            }
        }
        //Draw lower line (multiple if thickness > 1)
        for(uint8_t lineCount = 0; lineCount < thickness; ++lineCount)
        {
            uint16_t curY = y2+lineCount;
            for(uint16_t horizontal = x1; horizontal < x2+thickness; ++horizontal)
            {
                _frame[horizontal][curY] = color;
            }
        }
    }
}

void fb_fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
    //Check if we are in the bounds
    if( (x0+r < FRAME_WIDTH) && (y0+r < FRAME_HEIGHT) )
    {
        uint16_t diameter = 2*r;
        uint16_t right_bound = x0+r;
        //uint16_t lower_bound = y0+r;
        //From left to right
        for(uint16_t hor = x0-r; hor < right_bound; ++hor)
        {
            //TODO: Currently only draws a rectangle
            fb_drawVLine(hor, y0, y0+diameter, color);
        }
    }
}
void fb_drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {

}

void fb_drawVLine(uint16_t x, uint16_t y1, uint16_t y2, uint16_t color) {
    if( (x < FRAME_WIDTH) && (y1 < FRAME_HEIGHT) && (y2 < FRAME_HEIGHT) )
    {
        if(y1 < y2) 
        {
            for(uint16_t count = y1; count < y2; ++count) { _frame[x][count] = color; }
        }
        else if(y1 > y2) 
        {
            for(uint16_t count = y1; count > y2; --count) { _frame[x][count] = color; }
        }
    }
}
void fb_drawHLine(uint16_t y, uint16_t x1, uint16_t x2, uint16_t color) {
    if( (y < FRAME_HEIGHT) && (x1 < FRAME_WIDTH) && (x2 < FRAME_WIDTH) )
    {
        if(x1 < x2) 
        {
            for(uint16_t count = x1; count < x2; ++count) { _frame[count][y] = color; }
        }
        else if(x1 > x2) 
        {
            for(uint16_t count = x1; count > x2; --count) { _frame[count][y] = color; }
        }
    }
}

void fb_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
  
}

void fb_writeChar(uint16_t x, uint16_t y, char c, uint16_t txtColor, uint16_t bgColor, uint8_t txtSize) {
    //Check if we are in the bounds of the screen. Return, if not.
    if(x >= FRAME_WIDTH || y >= FRAME_HEIGHT) return; 

    //Horizontal direction
    for(uint8_t hor = 0; hor < 6; hor++) 
    {  
        //Get current vertical pixel-line of character from font
        //(leave space for 6th line)
        char line;
        if(hor < 5) line = font[c*5+hor];
        else        line = 0;
        
        for(uint8_t vertical = 0; vertical < 8; vertical++) 
        {              
            //Determine color of pixel (either text or background color)
            uint16_t currentColor; 
            if( (line & 0x1) == 1 ) currentColor = txtColor;  
            else currentColor = bgColor; 
            
            if(txtSize == 1) 
                fb_setPixel(x + hor, y + (7-vertical), currentColor);    
            else
                fb_fillRectangle(x + hor*txtSize, y + (7-vertical)*txtSize, txtSize, txtSize, currentColor);
                
            //Shift current line by one bit to the right
            line >>= 1;
        }
  }
}

void fb_writeText(uint16_t x, uint16_t y, const char * text, uint16_t txtColor, uint16_t bgColor, uint8_t txtSize) {
    //Check if we are in the bounds of the screen. Return, if not.
    if(x >= FRAME_WIDTH || y >= FRAME_HEIGHT) return; 
    
    uint16_t cursorX = x;
    uint16_t cursorY = y;
    
    for(uint16_t i = 0; text[i] != 0; ++i)
    {
        char c = text[i];
        //Handle newline character
        if(c == '\n') 
        {
            cursorY -= txtSize*8;
            cursorX  = 0;
        } 
        else if(c == '\r') 
        {
            continue;
        } 
        else 
        {
            if( (cursorX + txtSize * 6) >= FRAME_WIDTH )
            { 
                //Need to switch to next line
                cursorX  = 0;           // Reset x to start
                cursorY -= txtSize * 8; // Advance y one line
            }
            if( (cursorY - txtSize * 6) <= 0 ) 
            { 
                //Restart at the beginning
                cursorY = FRAME_HEIGHT-1; // Advance y one line
                cursorX = 0;              // Reset y to start
            }
            
            fb_writeChar(cursorX, cursorY, c, txtColor, bgColor, txtSize);
            cursorX += txtSize * 6;
        }
    }
}

void fb_writeTextCentered(uint16_t y, const char * text, uint16_t txtColor, uint16_t bgColor, uint8_t txtSize) {
    //Check if we are in the bounds of the screen. Return, if not.
    if(y >= FRAME_HEIGHT) return; 
    
    //Count number of chars in the string
    uint8_t count = 0;
    while(text[count] != 0) { ++count; }
    //Number of pixels to subtract
    uint16_t pxls = (count*txtSize*6)/2;
    if(pxls > FRAME_WIDTH/2)
    {
        //TODO: Do something (e.g. reduce text size, or force line break
        return;
    }
    uint16_t x = FRAME_WIDTH/2 - pxls;
    fb_writeText(x, y, text, txtColor, bgColor, txtSize);
}

void fb_writeTextRight(uint16_t y, const char * text, uint16_t txtColor, uint16_t bgColor, uint8_t txtSize) {
    //Check if we are in the bounds of the screen. Return, if not.
    if(y >= FRAME_HEIGHT) return;
    //Count number of chars in the string
    uint8_t count = 0;
    while(text[count] != 0) { ++count; }
    //Calculate number of pixels to subtract from max width
    uint16_t pxls = count*txtSize*6;
    if(pxls > FRAME_WIDTH)
    {
        //TODO: Do something (e.g. reduce text size, or force line break
        return;
    }
    uint16_t x = FRAME_WIDTH - pxls - 3;
    fb_writeText(x, y, text, txtColor, bgColor, txtSize);
}

void fb_drawButton(uint16_t x, uint16_t y, const char * text, uint16_t txtColor, uint16_t borderColor, uint16_t bgColor, uint8_t borderWidth) {
    //Check if we are in the bounds of the screen. Return, if not.
    if(x >= FRAME_WIDTH || y >= FRAME_HEIGHT) return; 
    
    //Count number of chars in the string
    uint8_t count = 0;
    while(text[count] != 0) { ++count; }
    //Number of pixels for text (6pxls per character width, text size 3)
    uint16_t pxls = count*6*3;
    
    //Draw rectangle for button
    uint16_t btnWidth = pxls + BTN_SPACE_LEFT + BTN_SPACE_RIGHT;
    uint16_t btnHeight = 8*3 + BTN_SPACE_TOP + BTN_SPACE_BOTTOM; //(8pxls per character height, text size 3)
    //Special case if border and button have same color
    if(borderColor == bgColor)
    {    
        btnWidth += borderWidth;
        btnHeight += borderWidth;
        fb_fillRectangle(x, y, btnWidth, btnHeight, bgColor);
    }
    else
    {
        //Draw border
        fb_drawRectangle(x, y, btnWidth+borderWidth, btnHeight+borderWidth, borderWidth, borderColor);
        //Draw Background
        fb_fillRectangle(x+borderWidth, y+borderWidth, btnWidth, btnHeight, bgColor);
    }
    
    //Write text on button
    fb_writeText(x+borderWidth+BTN_SPACE_LEFT, y+borderWidth+BTN_SPACE_TOP, text, txtColor, bgColor, 3);
}

void fb_drawButtonCentered(uint16_t y, const char * text, uint16_t txtColor, uint16_t borderColor, uint16_t bgColor, uint8_t borderWidth) {
    //Check if we are in the bounds of the screen. Return, if not.
    if(y >= FRAME_HEIGHT) return; 
    
    //Count number of chars in the string
    uint8_t count = 0;
    while(text[count] != 0) { ++count; }
    //Number of pixels to subtract (6pxls per character, text size 3)
    uint16_t pxls = (count*6*3)/2;
    if(pxls > FRAME_WIDTH/2)
    {
        //TODO: Do something (e.g. reduce text size, or force line break
        return;
    }
    uint16_t x = FRAME_WIDTH/2 - borderWidth - pxls - BTN_SPACE_LEFT;
    fb_drawButton(x, y, text, txtColor, borderColor, bgColor, borderWidth);
}

void fb_writeNumber(uint16_t x, uint16_t y, const int * value, uint16_t txtColor, uint16_t bgColor, uint8_t txtSize) {
    char buffer[20];
    sprintf(buffer, "%d", *value);
    fb_writeText(x, y, buffer, txtColor, bgColor, txtSize);
}
