#include "cppp_init.h"
#include "cppp_lcd_driver.h"
#include "cppp_lcd.h"
#include "cppp_gfx.h"
#include "adc/adc.h"


void cppp_initBoard(void){
  lcd_init();
  lcd_reset();
  lcd_setup();
  lcd_initCursor_s();
  cppp_initAdc();
  gfx_fillScreen(BLACK);
  lcd_setCursor_s(480,320);
}
