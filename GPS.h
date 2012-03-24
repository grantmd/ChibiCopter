/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Talks to a GPS over serial
*/

#ifndef _GPS_H_
#define _GPS_H_

static EventSource gps_event;

// Public functions
void GPSInit(void);

#endif /* _GPS_H_ */