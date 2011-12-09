/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    On-board accelerometer config
*/

#include <stdlib.h>
#include "ch.h"

#ifndef ACCEL_H_
#define ACCEL_H_

#define accel_addr 0x1D

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

struct accel_data {
	int16_t x;
	int16_t y;
	int16_t z;
};

int accel_init(void);
void accel_read(void);

int16_t get_accel_x(void);
int16_t get_accel_y(void);
int16_t get_accel_z(void);

#endif /* ACCEL_H_ */