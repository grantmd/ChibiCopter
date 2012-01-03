/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    On-board accelerometer (LIS302DL) config
*/

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "LIS302DL.h"

/* buffers */
static uint8_t LIS302DL_rx_data[LIS302DL_RX_DEPTH];
static uint8_t LIS302DL_tx_data[LIS302DL_TX_DEPTH];

struct LIS302DL_data data;

int LIS302DL_init(void){
	LIS302DL_tx_data[0] = LIS302DL_CTRL_REG1; /* register address */
	LIS302DL_tx_data[1] = 0b11100111;
	LIS302DL_tx_data[2] = 0b01000001;
	LIS302DL_tx_data[3] = 0b00000000;

	/* sending */
  spiAcquireBus(&SPID1);
  spiSelect(&SPID1);
	spiExchange(&SPID1, 4, LIS302DL_tx_data, LIS302DL_rx_data);
  spiUnselect(&SPID1);
  spiReleaseBus(&SPID1);

  return 0;
}

void LIS302DL_read(void){
  spiAcquireBus(&SPID1);
  spiSelect(&SPID1);

  LIS302DL_tx_data[0] = LIS302DL_OUT_DATA_X; // register address
  spiExchange(&SPID1, 1, LIS302DL_tx_data, LIS302DL_rx_data);
  data.x = LIS302DL_rx_data[0] + (LIS302DL_rx_data[1] << 8);

  LIS302DL_tx_data[0] = LIS302DL_OUT_DATA_Y; // register address
  spiExchange(&SPID1, 1, LIS302DL_tx_data, LIS302DL_rx_data);
  data.y = LIS302DL_rx_data[0] + (LIS302DL_rx_data[1] << 8);

  LIS302DL_tx_data[0] = LIS302DL_OUT_DATA_Z; // register address
  spiExchange(&SPID1, 1, LIS302DL_tx_data, LIS302DL_rx_data);
  data.z = LIS302DL_rx_data[0] + (LIS302DL_rx_data[1] << 8);

  spiUnselect(&SPID1);
  spiReleaseBus(&SPID1);
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