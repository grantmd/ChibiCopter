/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Talks to a Spektrum Satellite receiver over Serial and receives commands
*/

#include "ch.h"
#include "hal.h"

#include "Spektrum.h"

static const SerialConfig sd3cfg = {
	115200,
	0,
	USART_CR2_STOP1_BITS | USART_CR2_LINEN,
	0
};

int receiver_data[MAX_RC_CHANNELS];

/*
 * Activates the serial driver 3
 * PD8(TX) and PD9(RX) are routed to USART3.
 */

void SpektrumInit(void){

	sdStart(&SD3, &sd3cfg);
	palSetPadMode(GPIOD, 8, PAL_MODE_ALTERNATE(7)); // not currently connected
	palSetPadMode(GPIOD, 9, PAL_MODE_ALTERNATE(7)); // incoming data from the receiver

	unsigned i;
	for (i=0; i<MAX_RC_CHANNELS; i++){
		receiver_data[i] = 0;
	}
}

/*
 * Returns the most recent data for the given channel
 */

int getChannelData(int channel){
	if (channel < 0 || channel >= MAX_RC_CHANNELS) return 0;

	return receiver_data[channel];
}