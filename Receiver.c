/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Reads PWM signals off of an R/C receiver, like the Spektrum AR6100e
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

icucnt_t last_width, last_period;

static void icuwidthcb(ICUDriver *icup) {

  last_width = icuGetWidthI(icup);
  chprintf((BaseChannel *)&SD2, "ICU width: %d.\r\n", last_width);
}

static void icuperiodcb(ICUDriver *icup) {

  last_period = icuGetPeriodI(icup);
  chprintf((BaseChannel *)&SD2, "ICU period: %d.\r\n", last_period);
}

static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_LOW,
  10000,                                    /* 10KHz ICU clock frequency.   */
  icuwidthcb,
  icuperiodcb
};

void ReceiverInit(void){
	icuStart(&ICUD1, &icucfg);
	palSetPadMode(GPIOE, 10, PAL_MODE_ALTERNATE(2)); // THRO
	palSetPadMode(GPIOE, 11, PAL_MODE_ALTERNATE(2)); // AILE
	palSetPadMode(GPIOE, 12, PAL_MODE_ALTERNATE(2)); // ELEV
	palSetPadMode(GPIOE, 13, PAL_MODE_ALTERNATE(2)); // RUDD
	palSetPadMode(GPIOE, 14, PAL_MODE_ALTERNATE(2)); // GEAR
	palSetPadMode(GPIOE, 15, PAL_MODE_ALTERNATE(2)); // AUX1

	icuEnable(&ICUD1);
}