/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Reads PWM signals off of an R/C receiver, like the Spektrum AR6100e
*/

#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#define MAX_RC_CHANNELS 6
#define THROTTLE_CHANNEL 0
#define AILE_CHANNEL 0
#define ELEV_CHANNEL 0
#define RUDD_CHANNEL 0
#define GEAR_CHANNEL 0
#define AUX1_CHANNEL 0

void ReceiverInit(void);

inline icucnt_t ReceiverGetThrottle(void);
inline icucnt_t ReceiverGetAile(void);
inline icucnt_t ReceiverGetElev(void);
inline icucnt_t ReceiverGetRudd(void);
inline icucnt_t ReceiverGetGear(void);
inline icucnt_t ReceiverGetAux1(void);

#endif /* _RECEIVER_H_ */