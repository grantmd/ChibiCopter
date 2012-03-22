/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Handles communication with the ground/other UAVs and robots using mavlink
*/

#include "ch.h"
#include "hal.h"

#include "comms.h"

#include <mavlink.h>

// Setup some mavlink vars
mavlink_system_t mavlink_system;

mavlink_message_t comms_msg;
uint8_t comms_buf[MAVLINK_MAX_PACKET_LEN];

// Setup some timers and callbacks
static VirtualTimer vt_heartbeat;

static void hb_interrupt(void *p) {

	(void)p;

	chSysLockFromIsr();
	CommsHeartbeat();
	chVTSetI(&vt_heartbeat, MS2ST(1000), hb_interrupt, NULL);
	chSysUnlockFromIsr();
}

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

	mavlink_system.sysid = 0;
	mavlink_system.compid = MAV_COMP_ID_IMU;
	mavlink_system.type = MAV_TYPE_QUADROTOR;
	mavlink_system.mode = MAV_MODE_PREFLIGHT;
	mavlink_system.state = MAV_STATE_STANDBY;
	mavlink_system.nav_mode = MAV_AUTOPILOT_INVALID;

	uartStart(&UARTD2, &uart2cfg);
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7)); // yellow wire on the FTDI cable
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7)); // orange wire on the FTDI cable

	// Schedule regular heartbeat
	chVTSet(&vt_heartbeat, MS2ST(1000), hb_interrupt, NULL);
}

/*
 * Sends a heartbeat message out, so everyone knows we're still alive
 */

void CommsHeartbeat(void){

	// Pack the message
	mavlink_msg_heartbeat_pack(mavlink_system.sysid, mavlink_system.compid, &comms_msg, mavlink_system.type, mavlink_system.nav_mode, mavlink_system.mode, 0, mavlink_system.state);

	// Copy the message to the send buffer
	uint16_t len = mavlink_msg_to_send_buffer(comms_buf, &comms_msg);
	uartStartSend(&UARTD2, len, comms_buf);
}