/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Reads PWM signals off of an R/C receiver, like the Spektrum AR6100e
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "Receiver.h"

icucnt_t last_width, last_period;

static void icuwidthcb(ICUDriver *icup) {

  last_width = icuGetWidthI(icup);
}

static void icuperiodcb(ICUDriver *icup) {

  last_period = icuGetPeriodI(icup);
}

static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_HIGH,
  1000000,                                    /* 1MHz ICU clock frequency.   */
  icuwidthcb,
  icuperiodcb
};

void ReceiverInit(void){
	/*icuStart(&ICUD1, &icucfg);
	palSetPadMode(GPIOE, 10, PAL_MODE_ALTERNATE(2)); // THRO
	palSetPadMode(GPIOE, 11, PAL_MODE_ALTERNATE(2)); // AILE
	palSetPadMode(GPIOE, 12, PAL_MODE_ALTERNATE(2)); // ELEV
	palSetPadMode(GPIOE, 13, PAL_MODE_ALTERNATE(2)); // RUDD
	palSetPadMode(GPIOE, 14, PAL_MODE_ALTERNATE(2)); // GEAR
	palSetPadMode(GPIOE, 15, PAL_MODE_ALTERNATE(2)); // AUX1

	icuEnable(&ICUD1);*/

	icuStart(&ICUD3, &icucfg);
	palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(2));
	icuEnable(&ICUD3);
}

icucnt_t ReceiverGetWidth(void){ return last_width; }
icucnt_t ReceiverGetPeriod(void){ return last_period; }