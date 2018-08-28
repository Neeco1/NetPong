#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "cppp_board/cppp_adc.h"

/**
 *  Initializes the wires of the joysticks.
 */
void controlLedsInit(void);
void controlLedsInit_s(void);

/**
 *  Controlls the color of the rgb-led with the x-axis of joystick 1.
 */
void controlLeds(void);
void controlLeds_s(void);


/**
 *  Prints the analog values of joystick 1 and 2 on the display.
 */
void printValues(void);
void printValues_s(void);

#endif
