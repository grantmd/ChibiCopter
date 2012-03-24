/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Handles communication with the ground/other UAVs and robots using mavlink
*/

#ifndef _COMMS_H_
#define _COMMS_H_

#define MAVLINK_USE_CONVENIENCE_FUNCTIONS
 
#include "mavlink_types.h"
 
/* Struct that stores the communication settings of this system.
   you can also define / alter these settings elsewhere, as long
   as they're included BEFORE mavlink.h.
   So you can set the
 
   mavlink_system.sysid = 100; // System ID, 1-255
   mavlink_system.compid = 50; // Component/Subsystem ID, 1-255
 
   Lines also in your main.c, e.g. by reading these parameter from EEPROM.
 */
static mavlink_system_t mavlink_system;
 
/**
 * @brief Send buffer over a comm channel
 *
 * @param chan MAVLink channel to use, usually MAVLINK_COMM_0 = UART0
 * @param buf Data to send
 * @param len Length of data
 */
static inline void comms_send_bytes(mavlink_channel_t chan, const uint8_t *buf, uint16_t len){
	if (chan == MAVLINK_COMM_0){
		uartStartSend(&UARTD2, len, buf);
	}
}
#define MAVLINK_SEND_UART_BYTES(chan,buf,len) comms_send_bytes(chan, buf, len)

// Public functions
void CommsInit(void);
void CommsHeartbeat(void);
    
#endif /* _COMMS_H_ */