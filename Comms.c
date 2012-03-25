/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Handles communication with the ground/other UAVs and robots using mavlink
*/

#include "ch.h"
#include "hal.h"

#include "Comms.h"

#include <mavlink.h>

// Setup some mavlink vars
mavlink_system_t mavlink_system;
static mavlink_status_t comms_status;
static mavlink_message_t comms_msg_in;
static int comms_packet_drops, comms_packet_success = 0;

// Setup some timers and callbacks
static VirtualTimer vt1;
static void ledoff(void *p) {

	(void)p;
	palClearPad(GPIOD, GPIOD_LED4);
}

static WORKING_AREA(COMMSWA, 128);
static msg_t Comms(void *arg){

	(void)arg;
	chRegSetThreadName("Comms");
	while (TRUE){
		// Read a byte off the receiver
		uint8_t c = chIOGet((BaseChannel *)&SD2);

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
			comms_packet_success += comms_status.packet_rx_success_count;
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

	CommsSendHeartbeat();
	chThdCreateStatic(COMMSWA, sizeof(COMMSWA), NORMALPRIO, Comms, NULL);
}

/*
 * Sends a heartbeat message out, so everyone knows we're still alive
 */

void CommsSendHeartbeat(void){
	palSetPad(GPIOD, GPIOD_LED4); // green
	mavlink_msg_heartbeat_send(MAVLINK_COMM_0, mavlink_system.type, mavlink_system.nav_mode,  mavlink_system.mode, 0, mavlink_system.state);

	chSysLock();
	if (chVTIsArmedI(&vt1))
		chVTResetI(&vt1);
	chVTSetI(&vt1, MS2ST(500), ledoff, NULL);
	chSysUnlock();
}

/*
 * Sends a system status message
 */

void CommsSendSysStatus(void){
	// Indices: 
	// 0: 3D gyro
	// 1: 3D acc
	// 2: 3D mag
	// 3: absolute pressure
	// 4: differential pressure
	// 5: GPS
	// 6: optical flow
	// 7: computer vision position
	// 8: laser based position
	// 9: external ground-truth (Vicon or Leica)
	// Controls:
	// 10: 3D angular rate control
	// 11: attitude stabilization
	// 12: yaw position control
	// 13: z/altitude control
	// 14: x/y position control
	// 15: motor outputs / control
	uint32_t sensors_present = 0xFC23;
	uint32_t sensors_enabled = sensors_present;

	mavlink_msg_sys_status_send(MAVLINK_COMM_0, sensors_present, sensors_enabled, sensors_enabled, 0, 11100, -1, -1, comms_packet_drops/comms_packet_success*10000, comms_packet_drops, 0, 0, 0, 0);
}

void CommsSendAttitude(uint32_t time_boot_ms, float roll, float pitch, float yaw, float rollspeed, float pitchspeed, float yawspeed){
	mavlink_msg_attitude_send(MAVLINK_COMM_0, time_boot_ms, roll, pitch, yaw, rollspeed, pitchspeed, yawspeed);
}


/**
 * @brief Send buffer over a comm channel
 *
 * @param chan MAVLink channel to use, usually MAVLINK_COMM_0 = UART0
 * @param buf Data to send
 * @param len Length of data
 */
inline void comms_send_bytes(mavlink_channel_t chan, const uint8_t *buf, uint16_t len){
	if (chan == MAVLINK_COMM_0){
		chIOWriteTimeout(&SD2, buf, len, TIME_INFINITE);
	}
}