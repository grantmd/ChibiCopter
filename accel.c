/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    On-board accelerometer config
*/

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "accel.h"

/* buffers */
static uint8_t accel_rx_data[ACCEL_RX_DEPTH];
static uint8_t accel_tx_data[ACCEL_TX_DEPTH];

static i2cflags_t errors = 0;

struct accel_data data;

int accel_init(void){
	accel_tx_data[0] = ACCEL_CTRL_REG1 | AUTO_INCREMENT_BIT; /* register address */
	accel_tx_data[1] = 0b11100111;
	accel_tx_data[2] = 0b01000001;
	accel_tx_data[3] = 0b00000000;

	/* sending */
  i2cAcquireBus(&I2CD1);
	i2cMasterTransmit(&I2CD1, accel_addr, accel_tx_data, 4, accel_rx_data, 0, &errors, TIME_INFINITE);
  i2cReleaseBus(&I2CD1);

  return 0;
}

void accel_read(void){
  accel_tx_data[0] = ACCEL_OUT_DATA | AUTO_INCREMENT_BIT; // register address
  i2cAcquireBus(&I2CD1);
  i2cMasterTransmit(&I2CD1, accel_addr, accel_tx_data, 1, accel_rx_data, 6, &errors, TIME_INFINITE);
  i2cReleaseBus(&I2CD1);

  data.x = accel_rx_data[0] + (accel_rx_data[1] << 8);
  data.y = accel_rx_data[2] + (accel_rx_data[3] << 8);
  data.z = accel_rx_data[4] + (accel_rx_data[5] << 8);
}

int16_t get_accel_x(void){
  return data.x;
}

int16_t get_accel_y(void){
  return data.y;
}

int16_t get_accel_z(void){
  return data.z;
}