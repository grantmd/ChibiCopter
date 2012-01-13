/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Reads PWM signals off of an R/C receiver, like the Spektrum AR6100e
*/

#include "ch.h"
#include "hal.h"

void ReceiverInit(void){
	palSetPadMode(GPIOE, 10, PAL_MODE_ALTERNATE(2));  // THRO
	palSetPadMode(GPIOE, 11, PAL_MODE_ALTERNATE(2)); // AILE
	palSetPadMode(GPIOE, 12, PAL_MODE_ALTERNATE(2)); // ELEV
	palSetPadMode(GPIOE, 13, PAL_MODE_ALTERNATE(2)); // RUDD
	palSetPadMode(GPIOE, 14, PAL_MODE_ALTERNATE(2)); // GEAR
	palSetPadMode(GPIOE, 15, PAL_MODE_ALTERNATE(2)); // AUX1
}