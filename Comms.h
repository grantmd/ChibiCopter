/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Handles communication with the ground/other UAVs and robots using mavlink
*/

#ifndef _COMMS_H_
#define _COMMS_H_

#define MAVLINK_USE_CONVENIENCE_FUNCTIONS
 
#include "mavlink_types.h"

extern mavlink_system_t mavlink_system;

/**
 * @brief Send buffer over a comm channel
 *
 * @param chan MAVLink channel to use, usually MAVLINK_COMM_0 = UART0
 * @param buf Data to send
 * @param len Length of data
 */
static inline void comms_send_bytes(mavlink_channel_t chan, const uint8_t *buf, uint16_t len){
	if (chan == MAVLINK_COMM_0){
		chIOWriteTimeout(&SD2, buf, len, TIME_IMMEDIATE);
	}
}
#define MAVLINK_SEND_UART_BYTES(chan,buf,len) comms_send_bytes(chan, buf, len)

// Public functions
void CommsInit(void);
void CommsHeartbeat(void);
    
#endif /* _COMMS_H_ */