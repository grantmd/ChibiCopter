/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Talks to a Spektrum Satellite receiver over Serial and receives commands
*/

#ifndef _SPEKTRUM_H_
#define _SPEKTRUM_H_

// Warning: Possibly only accurate for the DX6i TX
#define MAX_RC_CHANNELS 7
#define AILE_CHANNEL 0
#define AUX1_CHANNEL 1 // Flaps?
#define ELEV_CHANNEL 2
#define RUDD_CHANNEL 3
#define THROTTLE_CHANNEL 4
#define GEAR_CHANNEL 5
#define AUX2_CHANNEL 6

static EventSource spektrum_event;

// Public functions
void SpektrumInit(void);
int getSpektrumData(int channel);

// Private functions
unsigned char _SpektrumParse(uint8_t c);
long _SpektrumGetFrameNum(void);
void _SpektrumInvalidateFrame(void);
void _SpektrumValidateFrame(void);
unsigned char _SpektrumFrameIsValid(void);

#endif /* _SPEKTRUM_H_ */