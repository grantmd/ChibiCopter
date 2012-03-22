/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Handles communication with the ground/other UAVs and robots using mavlink
*/

#ifndef _COMMS_H_
#define _COMMS_H_

// Public functions
void CommsInit(void);
void CommsHeartbeat(void);
    
#endif /* _COMMS_H_ */