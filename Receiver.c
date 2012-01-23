/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Reads PWM signals off of an R/C receiver, like the Spektrum AR6100e
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "Receiver.h"

icucnt_t receiver_data[MAX_RC_CHANNELS];

static void throttle_cb(ICUDriver *icup) {

	icucnt_t width = icuGetWidthI(icup);
	if (width < 1000 || width > 10000) return;

	receiver_data[THROTTLE_CHANNEL] = width / 10 + 1000;
	palSetPad(GPIOD, GPIOD_LED4); // green
}

static void icuperiodcb(ICUDriver *icup) {

	(void)icup;
	palClearPad(GPIOD, GPIOD_LED4); // green
}

static ICUConfig throttlecfg = {
	ICU_INPUT_ACTIVE_HIGH,
	10000,                                    /* 10KHz ICU clock frequency.   */
	throttle_cb,
	icuperiodcb
};

void ReceiverInit(void){
	unsigned i;
	for (i=0; i<MAX_RC_CHANNELS; i++){
		receiver_data[i] = 1000;
	}

	icuStart(&ICUD1, &throttlecfg);
	palSetPadMode(GPIOE,  9, PAL_MODE_ALTERNATE(1)); // THRO
	//palSetPadMode(GPIOE,  9, PAL_MODE_ALTERNATE(1)); // AILE
	//palSetPadMode(GPIOE, 10, PAL_MODE_ALTERNATE(1)); // ELEV
	//palSetPadMode(GPIOE, 11, PAL_MODE_ALTERNATE(1)); // RUDD
	//palSetPadMode(GPIOE, 12, PAL_MODE_ALTERNATE(1)); // GEAR
	//palSetPadMode(GPIOE, 13, PAL_MODE_ALTERNATE(1)); // AUX1
	icuEnable(&ICUD1);
}

inline icucnt_t ReceiverGetThrottle(void){ return receiver_data[THROTTLE_CHANNEL]; }
inline icucnt_t ReceiverGetAile(void){ return receiver_data[AILE_CHANNEL]; }
inline icucnt_t ReceiverGetElev(void){ return receiver_data[ELEV_CHANNEL]; }
inline icucnt_t ReceiverGetRudd(void){ return receiver_data[RUDD_CHANNEL]; }
inline icucnt_t ReceiverGetGear(void){ return receiver_data[GEAR_CHANNEL]; }
inline icucnt_t ReceiverGetAux1(void){ return receiver_data[AUX1_CHANNEL]; }