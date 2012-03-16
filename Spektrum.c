/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Talks to a Spektrum Satellite receiver over Serial and receives commands
*/

#include "ch.h"
#include "hal.h"

#include "Spektrum.h"

// Our config for the serial connection to the RX
static const SerialConfig sd3cfg = {
	115200,
	0,
	USART_CR2_STOP1_BITS | USART_CR2_LINEN,
	0
};

// Most recent valid data, per-channel
int receiver_data[MAX_RC_CHANNELS];

// A struct to maintain decoding state
typedef struct {
	char state;
	unsigned char data[14];
	unsigned char valid;
	long frameNum;
} spektrum_t;

spektrum_t rx_state;

#define SPEKTRUM_NUM_BYTES_IN_FRAME (2*MAX_RC_CHANNELS+2)

/*
 * Activates the serial driver 3
 * PD8(TX) and PD9(RX) are routed to USART3.
 */

void SpektrumInit(void){

	sdStart(&SD3, &sd3cfg);
	palSetPadMode(GPIOD, 8, PAL_MODE_ALTERNATE(7)); // not currently connected
	palSetPadMode(GPIOD, 9, PAL_MODE_ALTERNATE(7)); // incoming data from the receiver

	unsigned i;
	for (i=0; i<MAX_RC_CHANNELS; i++){
		receiver_data[i] = 0;
	}

	rx_state.state = 0;
	rx_state.valid = 0;
	rx_state.frameNum = 0L;
}

/*
 * Reads data off of the serial connection and runs it through the parser
 * TODO: This should be a thread
 */

void SpektrumRead(void){
	// Read a byte off the receiver
    uint8_t c = chIOGet((BaseChannel *)&SD3);
    if (_SpektrumParse(c)){
    }
}

/*
 * Returns the most recent data for the given channel
 */

int getChannelData(int channel){
	if (channel < 0 || channel >= MAX_RC_CHANNELS) return 0;

	return receiver_data[channel];
}

////////////////////////////////////////////
// Private functions

/*
 * Takes a single character read off the serial connection. Combines it with our current state to progress that state.
 * Sets receiver_data if the frame is complete and valid.
 * Returns whether or not the frame is now complete and valid 
 */

// Mostly borred from: http://mbed.org/users/offroad/libraries/spektRx/lzmwp4/docs/spektRx_8c_source.html
unsigned char _SpektrumParse(uint8_t c){
	switch(rx_state.state){
		case 0: /* new frame cycle */
			_SpektrumInvalidateFrame();
			/* first preamble byte received: now expect 2nd preamble byte */
			rx_state.state = (c == 0x03) ? 1 : 0;
			break;

		case 1:
			/* 2nd preamble byte received: Now expect first data byte */             
			rx_state.state = (c == 0x01) ? 2 : 0;
			break;

		default:
			/* store received byte */
			rx_state.data[rx_state.state - 2] = c;
			++rx_state.state;

			if (rx_state.state == SPEKTRUM_NUM_BYTES_IN_FRAME){
				/* one complete frame was received.
				* - Copy the data
				* - mask out bits 10..15
				*/
				int ix; 
				for (ix = 0; ix < MAX_RC_CHANNELS; ++ix){
					// TODO: This probably requires some other sort of conversion
					receiver_data[ix] = ((rx_state.data[2*ix] & 3) << 8) | rx_state.data[2*ix+1];
				}

				/* output data is now valid */
				_SpektrumValidateFrame();
				++rx_state.frameNum;

				/* ready for the next frame */
				rx_state.state = 0;
			} /* if frame complete */
	} /* switch state */

	return _SpektrumFrameIsValid();
}

long _SpektrumGetFrameNum(void){
	return rx_state.frameNum;
}

void _SpektrumInvalidateFrame(void){
	palClearPad(GPIOD, GPIOD_LED4); // green
	rx_state.valid = 0;
}

void _SpektrumValidateFrame(void){
	palSetPad(GPIOD, GPIOD_LED4); // green
	rx_state.valid = 1;
}

unsigned char _SpektrumFrameIsValid(void){
	return rx_state.valid;
}