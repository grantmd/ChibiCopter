/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    On-board LIS302DLerometer config
*/

#include <stdlib.h>
#include "ch.h"

#ifndef LIS302DL_H_
#define LIS302DL_H_

#define LIS302DL_addr 0x1D

/* autoincrement bit position. This bit needs to perform reading of
 * multiple bytes at one request */
#define AUTO_INCREMENT_BIT (1<<7)

/* slave specific addresses */
#define LIS302DL_STATUS_REG  0x27
#define LIS302DL_CTRL_REG1   0x20
#define LIS302DL_OUT_DATA    0x28

/* buffers depth */
#define LIS302DL_RX_DEPTH 8
#define LIS302DL_TX_DEPTH 8

struct LIS302DL_data {
	int16_t x;
	int16_t y;
	int16_t z;
};

int LIS302DL_init(void);
void LIS302DL_read(void);

int16_t get_LIS302DL_x(void);
int16_t get_LIS302DL_y(void);
int16_t get_LIS302DL_z(void);

#endif /* LIS302DL_H_ */