/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Convenience functions for accessing I2C data
*/

#include "ch.h"
#include "hal.h"

#include "I2CSensor.h"

/* I2C interface #1 */
static const I2CConfig i2cfg1 = {
	OPMODE_I2C,
	400000,
	FAST_DUTY_CYCLE_2,
};

void I2CSensorInit(void){
	/*
	 * Starts I2C
	 */
	i2cStart(&I2CD1, &i2cfg1);
}