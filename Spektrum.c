/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Talks to a Spektrum Satellite receiver over Serial and receives commands
*/

#include "ch.h"
#include "hal.h"

#include "Spektrum.h"

static const SerialConfig sd3cfg = {
	57600,
	0,
	USART_CR2_STOP1_BITS | USART_CR2_LINEN,
	0
};

/*
 * Activates the serial driver 3
 * PD8(TX) and PD9(RX) are routed to USART3.
 */

void SpektrumInit(void){

	sdStart(&SD3, &sd3cfg);
	palSetPadMode(GPIOD, 8, PAL_MODE_ALTERNATE(7)); // not currently connected
	palSetPadMode(GPIOD, 9, PAL_MODE_ALTERNATE(7)); // incoming data from the receiver
}