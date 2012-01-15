/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Reads PWM signals off of an R/C receiver, like the Spektrum AR6100e
*/

#ifndef _RECEIVER_H_
#define _RECEIVER_H_

void ReceiverInit(void);

icucnt_t ReceiverGetWidth(void);
icucnt_t ReceiverGetPeriod(void);

#endif /* _RECEIVER_H_ */