/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Reads PWM signals off of an R/C receiver, like the Spektrum AR6100e
*/

#include "ch.h"
#include "hal.h"

void ReceiverInit(void){
	palSetPadMode(GPIOE, 10, PIN_MODE_INPUT(9));  // THRO
	palSetPadMode(GPIOE, 11, PIN_MODE_INPUT(10)); // AILE
	palSetPadMode(GPIOE, 12, PIN_MODE_INPUT(11)); // ELEV
	palSetPadMode(GPIOE, 13, PIN_MODE_INPUT(12)); // RUDD
	palSetPadMode(GPIOE, 14, PIN_MODE_INPUT(13)); // GEAR
	palSetPadMode(GPIOE, 15, PIN_MODE_INPUT(14)); // AUX1
}