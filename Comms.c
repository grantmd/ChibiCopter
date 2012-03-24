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
static mavlink_status_t comms_status;
static mavlink_message_t comms_msg_in;
static int comms_packet_drops = 0;

// Setup some timers and callbacks

static WORKING_AREA(COMMSWA, 128);
static msg_t Comms(void *arg){

	(void)arg;
	chRegSetThreadName("Comms");
	while (TRUE){
		// Read a byte off the receiver
		uint8_t c = chIOGet((BaseChannel *)&SD2);

		if (mavlink_parse_char(MAVLINK_COMM_0, c, &comms_msg_in, &comms_status)){
			// Handle message
			palSetPad(GPIOD, GPIOD_LED4); // green

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
	}
	return 0;
}

// Our config for the serial connection to the RX
static const SerialConfig sd2cfg = {
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
	mavlink_system.state = MAV_STATE_BOOT;
	mavlink_system.nav_mode = MAV_AUTOPILOT_INVALID;

	sdStart(&SD2, &sd2cfg);
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7)); // yellow wire on the FTDI cable
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7)); // orange wire on the FTDI cable

	CommsHeartbeat();
	chThdCreateStatic(COMMSWA, sizeof(COMMSWA), NORMALPRIO, Comms, NULL);
}

/*
 * Sends a heartbeat message out, so everyone knows we're still alive
 */

void CommsHeartbeat(void){
	mavlink_msg_heartbeat_send(MAVLINK_COMM_0, mavlink_system.type, mavlink_system.nav_mode,  mavlink_system.mode, 0, mavlink_system.state);
}