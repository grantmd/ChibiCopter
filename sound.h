/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    On-board sound config
*/

#include <stdlib.h>
#include "ch.h"

#ifndef SOUND_H_
#define SOUND_H_

#define sound_addr 0x94

/* autoincrement bit position. This bit needs to perform reading of
 * multiple bytes at one request */
#define AUTO_INCREMENT_BIT (1<<7)

/* slave specific addresses */
#define SOUND_STATUS_REG  0x27
#define SOUND_CTRL_REG1   0x20
#define SOUND_OUT_DATA    0x28

/* buffers depth */
#define SOUND_RX_DEPTH 8
#define SOUND_TX_DEPTH 8

int sound_init(void);
void sound_read(void);

#endif /* SOUND_H_ */