#include "ch.h"
#include "hal.h"

#include "accel.h"

/* buffers */
static i2cblock_t accel_rx_data[ACCEL_RX_DEPTH];
static i2cblock_t accel_tx_data[ACCEL_TX_DEPTH];

void accel_init(void){
	accel_tx_data[0] = ACCEL_CTRL_REG1 | AUTO_INCREMENT_BIT; /* register address */
	accel_tx_data[1] = 0b11100111;
	accel_tx_data[2] = 0b01000001;
	accel_tx_data[3] = 0b00000000;

	/* sending */
	i2cMasterTransmit(&I2CD1, &accel, accel_addr, accel_tx_data, 4, accel_rx_data, 0);
}