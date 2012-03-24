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
//mavlink_system_t mavlink_system;
mavlink_status_t comms_status;

//mavlink_message_t comms_msg_out;
//uint8_t comms_buf_out[MAVLINK_MAX_PACKET_LEN];

mavlink_message_t comms_msg_in;

static int comms_packet_drops = 0;

// Setup some timers and callbacks
static VirtualTimer vt_heartbeat, vt2, vt3;

static void gledoff(void *p) {

	(void)p;
	palClearPad(GPIOD, GPIOD_LED4); // green
}

static void rledoff(void *p) {

	(void)p;
	palClearPad(GPIOD, GPIOD_LED5); // red
}

static void hb_interrupt(void *p){

	(void)p;
	palSetPad(GPIOD, GPIOD_LED4); // green
	//CommsHeartbeat();

	chSysLockFromIsr();
	//uartStartSendI(&UARTD2, len, comms_buf_out);
	chVTSetI(&vt_heartbeat, MS2ST(1000), hb_interrupt, NULL);
	chVTSetI(&vt2, MS2ST(200), gledoff, NULL);
	chSysUnlockFromIsr();
}

static void comms_rxchar(UARTDriver *uartp, uint16_t c){

	(void)uartp;
	palSetPad(GPIOD, GPIOD_LED5); // red

	if (mavlink_parse_char(MAVLINK_COMM_0, c, &comms_msg_in, &comms_status)){
		// Handle message

		switch (comms_msg_in.msgid){
			case MAVLINK_MSG_ID_HEARTBEAT:
				// E.g. read GCS heartbeat and go into
				// comm lost mode if timer times out
				break;
			case MAVLINK_MSG_ID_COMMAND_LONG:
				// EXECUTE ACTION
				break;
			default:
				//Do nothing
				break;
		}

		// Update global packet drops counter
		comms_packet_drops += comms_status.packet_rx_drop_count;
	}

	chSysLockFromIsr();
  	if (chVTIsArmedI(&vt3))
    	chVTResetI(&vt3);
	chVTSetI(&vt3, MS2ST(200), rledoff, NULL);
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

	mavlink_system.sysid = 1;
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

	//CommsHeartbeat();
}

/*
 * Sends a heartbeat message out, so everyone knows we're still alive
 */

void CommsHeartbeat(void){
	mavlink_msg_heartbeat_send(MAVLINK_COMM_0, mavlink_system.type, mavlink_system.nav_mode,  mavlink_system.mode, 0, mavlink_system.state);
}