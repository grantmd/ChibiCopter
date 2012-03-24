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

inline void comms_send_bytes(mavlink_channel_t chan, const uint8_t *buf, uint16_t len);
#define MAVLINK_SEND_UART_BYTES(chan,buf,len) comms_send_bytes(chan, buf, len)

// Public functions
void CommsInit(void);
void CommsHeartbeat(void);
    
#endif /* _COMMS_H_ */