#include "joystick.h"

#include <stdint.h>
#include "s6e2ccxj.h"
#include "cppp_adc.h"

// Declare pointers for the red, green and blue LED    
volatile uint32_t *direction1 = &(FM4_GPIO->DDR1);  
volatile uint32_t *direction2 = &(FM4_GPIO->DDRB);           
volatile uint32_t *dataOut1 = &(FM4_GPIO->PDOR1);          
volatile uint32_t *dataOut2 = &(FM4_GPIO->PDORB);

void controlLedsInit(){
  // turn off analog Pins
  bFM4_GPIO_ADE_AN10 = 0; // Red: Analog off 
  bFM4_GPIO_ADE_AN18 = 0; // Green: Analog off
  bFM4_GPIO_ADE_AN08 = 0; // Blue: Analog off
  
  // Define Pins as outputs
  *direction1 |= 0x0400; // Red
  *direction2 |= 0x0004; // Green
  *direction1 |= 0x0100; // Blue
  
  // turn all LEDs down
  *dataOut1 |= 0x0400; // Red off
  *dataOut2 |= 0x0004; // Green off
  *dataOut1 |= 0x0100; // Blue off

}

void controlLeds(){
  // get analog values 
  /*uint8_t analog16_X1;
  uint8_t analog19_Y1;
  uint8_t analog13_X2;
  uint8_t analog23_Y2;*/
  
  // green => Pin104 PB2/A18    blue => Pin106 P18/A08     red => Pin108 P1A/A10
  // JS1X is analog19
  
  // left: JS1 X < 255 und >= 200   => green
  // middle: JS1 X < 200 && >= 180  => blue
  // right: JS1 X <180 && >= 0      => red
  
  // joystick left
   // green on
   // blue off
   // red off
  
  // joystick middle
    // green off
    // blue on
    // red off
  
  // joystick right
    // green off
    // blue off
    // red on
  
  // delay 0,01s
}

void printValues(){
  // get analog valuess
  // Read and print all analog values of the system
  
  // set cursor of the display to (480,320)
  
  
  // write one linebreak then the headline, afterwards 4 linebreaks
  
  
  
  // write analog values of joystick 1
  
  // write analog values of joystick 2
}
