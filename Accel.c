/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Read accelerometer data over SPI: the LIS302DL
*/

#include "ch.h"
#include "hal.h"

#include "lis302dl.h"

#include "Accel.h"

int8_t accel_x, accel_y, accel_z;

/*
 * SPI1 configuration structure.
 * Speed 5.25MHz, CPHA=1, CPOL=1, 8bits frames, MSb transmitted first.
 * The slave select line is the pin GPIOE_CS_SPI on the port GPIOE.
 */
static const SPIConfig spi1cfg = {
	NULL,
	/* HW dependent part.*/
	GPIOE,
	GPIOE_CS_SPI,
	SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_CPOL | SPI_CR1_CPHA
};

/*
 * Initializes the SPI driver 1 in order to access the MEMS. The signals
 * are initialized in the board file.
 * Several LIS302DL registers are then initialized.
 */

void AccelInit(void){
	spiStart(&SPID1, &spi1cfg);
	lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG1, 0x43);
	lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG2, 0x00);
	lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG3, 0x00);
}

void AccelRead(void){

    accel_x = (int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTX);
    accel_y = (int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTY);
    accel_z = (int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTZ);
}