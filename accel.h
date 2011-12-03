/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    On-board accelerometer config
*/

#include <stdlib.h>
#include "ch.h"

#ifndef ACCEL_H_
#define ACCEL_H_

#define accel_addr 0b0011101

/* autoincrement bit position. This bit needs to perform reading of
 * multiple bytes at one request */
#define AUTO_INCREMENT_BIT (1<<7)

/* slave specific addresses */
#define ACCEL_STATUS_REG  0x27
#define ACCEL_CTRL_REG1   0x20
#define ACCEL_OUT_DATA    0x28

/* buffers depth */
#define ACCEL_RX_DEPTH 8
#define ACCEL_TX_DEPTH 8

/* Error callback */
static void accel_error_cb(I2CDriver *i2cp, const I2CSlaveConfig *i2cscfg){
	(void)i2cscfg;
	int status = 0;
	status = i2cp->id_i2c->SR1;
	while(TRUE);
}

/* Transfer finished callback */
static void accel_cb(I2CDriver *i2cp, const I2CSlaveConfig *i2cscfg){
	(void)i2cp;
	(void)i2cscfg;
}

/* Accelerometer config */
static const I2CSlaveConfig accel = {
	accel_cb,
	accel_error_cb,
};

struct accel_data {
	int x;
	int y;
	int z;
};

void accel_init(void);
void accel_read(void);

#endif /* ACCEL_H_ */