/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Talks to a GPS over serial
*/

#ifndef _GPS_H_
#define _GPS_H_

#define GPS_MAILBOX_SIZE 128
extern Mailbox GPSmb;
extern msg_t GPSMessage[GPS_MAILBOX_SIZE];

// Public functions
void GPSInit(void);

#endif /* _GPS_H_ */