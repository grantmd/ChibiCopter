/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    On-board LIS302DLerometer config
*/

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "LIS302DL.h"

/* buffers */
static uint8_t LIS302DL_rx_data[LIS302DL_RX_DEPTH];
static uint8_t LIS302DL_tx_data[LIS302DL_TX_DEPTH];

static i2cflags_t errors = 0;

struct LIS302DL_data data;

int LIS302DL_init(void){
	LIS302DL_tx_data[0] = LIS302DL_CTRL_REG1 | AUTO_INCREMENT_BIT; /* register address */
	LIS302DL_tx_data[1] = 0b11100111;
	LIS302DL_tx_data[2] = 0b01000001;
	LIS302DL_tx_data[3] = 0b00000000;

	/* sending */
  i2cAcquireBus(&I2CD1);
	i2cMasterTransmit(&I2CD1, LIS302DL_addr, LIS302DL_tx_data, 4, LIS302DL_rx_data, 0, &errors, TIME_INFINITE);
  i2cReleaseBus(&I2CD1);

  return 0;
}

void LIS302DL_read(void){
  LIS302DL_tx_data[0] = LIS302DL_OUT_DATA | AUTO_INCREMENT_BIT; // register address
  i2cAcquireBus(&I2CD1);
  i2cMasterTransmit(&I2CD1, LIS302DL_addr, LIS302DL_tx_data, 1, LIS302DL_rx_data, 6, &errors, TIME_INFINITE);
  i2cReleaseBus(&I2CD1);

  data.x = LIS302DL_rx_data[0] + (LIS302DL_rx_data[1] << 8);
  data.y = LIS302DL_rx_data[2] + (LIS302DL_rx_data[3] << 8);
  data.z = LIS302DL_rx_data[4] + (LIS302DL_rx_data[5] << 8);
}

int16_t get_LIS302DL_x(void){
  return data.x;
}

int16_t get_LIS302DL_y(void){
  return data.y;
}

int16_t get_LIS302DL_z(void){
  return data.z;
}