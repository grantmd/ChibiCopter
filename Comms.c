/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Handles communication with the ground/other UAVs and robots using mavlink
*/

#include "ch.h"
#include "hal.h"

#include "comms.h"

#include <mavlink.h>

static void comms_rxchar(UARTDriver *uartp, uint16_t c) {

	(void)uartp;
	chSysLockFromIsr();
	
	chSysUnlockFromIsr();
}

// Our config for the serial connection to the RX
static UARTConfig uart2cfg = {
	NULL,
	NULL,
	NULL,
	comms_rxchar,
	NULL, // TODO: We may need this
	115200,
	0,
	USART_CR2_STOP1_BITS | USART_CR2_LINEN,
	0
};

/*
 * Activates the serial driver 2
 * PA2(TX) and PA3(RX) are routed to USART2.
 */

void CommsInit(void){

	uartStart(&UARTD2, &uart2cfg);
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7)); // yellow wire on the FTDI cable
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7)); // orange wire on the FTDI cable
}