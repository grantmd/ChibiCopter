/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    On-board accelerometer config
*/

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "sound.h"

/* buffers */
static uint8_t sound_rx_data[SOUND_RX_DEPTH];
static uint8_t sound_tx_data[SOUND_TX_DEPTH];

int sound_init(void){
	sound_tx_data[0] = SOUND_CTRL_REG1 | AUTO_INCREMENT_BIT; /* register address */
	sound_tx_data[1] = 0b11100111;
	sound_tx_data[2] = 0b01000001;
	sound_tx_data[3] = 0b00000000;

	/* sending */
  i2cAcquireBus(&I2CD1);
	i2cMasterTransmit(&I2CD1, sound_addr, sound_tx_data, 4, sound_rx_data, 0);
  i2cReleaseBus(&I2CD1);

  return 0;
}

void sound_read(void){
  sound_tx_data[0] = SOUND_OUT_DATA | AUTO_INCREMENT_BIT; // register address
  i2cAcquireBus(&I2CD1);
  i2cMasterTransmit(&I2CD1, sound_addr, sound_tx_data, 1, sound_rx_data, 6);
  i2cReleaseBus(&I2CD1);

  /*data.x = sound_rx_data[0] + (sound_rx_data[1] << 8);
  data.y = sound_rx_data[2] + (sound_rx_data[3] << 8);
  data.z = sound_rx_data[4] + (sound_rx_data[5] << 8);*/
}