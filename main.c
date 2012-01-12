/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Based on demo code from ChibiOS/RT
*/

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "lis302dl.h"
#include "chprintf.h"

#include "TinyGPS.h"

BaseChannel *chp; // serial port

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
 * This is a periodic thread that blinks some leds
 */
static WORKING_AREA(BlinkWA, 128);
static msg_t Blink(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palSetPad(GPIOD, GPIOD_LED5);
    chThdSleepMilliseconds(100);
    palClearPad(GPIOD, GPIOD_LED5);
    palSetPad(GPIOD, GPIOD_LED6);
    chThdSleepMilliseconds(100);
    palClearPad(GPIOD, GPIOD_LED6);
    palSetPad(GPIOD, GPIOD_LED4);
    chThdSleepMilliseconds(100);
    palClearPad(GPIOD, GPIOD_LED4);
    palSetPad(GPIOD, GPIOD_LED3);
    chThdSleepMilliseconds(100);
    palClearPad(GPIOD, GPIOD_LED3);
  }
  return 0;
}

/*
 * GPS setup
 */
static const SerialConfig sd1cfg = {
  57600,
  0,
  USART_CR2_STOP1_BITS | USART_CR2_LINEN,
  0
};

static WORKING_AREA(GPSWA, 128);
static msg_t GPS(void *arg) {

  (void)arg;
  chRegSetThreadName("GPS");
  while (TRUE) {
    unsigned char newdata = 0;
    // Read a byte off the GPS
    uint8_t c = chIOGet((BaseChannel *)&SD1);
    if (TinyGPS_encode(c)){
        newdata = 1;
        //chprintf(chp, "Got GPS data.\r\n");
    }
    //chIOPut(chp, c);
  }
  return 0;
}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 2 using the driver default configuration.
   * PA2(TX) and PA3(RX) are routed to USART2.
   */

  sdStart(&SD2, NULL);
  chp = &SD2;
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7)); // yellow wire on the FTDI cable
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7)); // orange wire on the FTDI cable
  chprintf(chp, "Hello, startup!\r\n");

  chprintf(chp, "Configurating I/O.\r\n");

  /*
   * Activates the serial driver 1 using the driver default configuration, but at 38400
   * PA9(TX) and PA10(RX) are routed to USART1.
   */

  chprintf(chp, "GPS...");
  sdStart(&SD1, &sd1cfg);
  palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7)); // not currently connected
  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7)); // incoming data from the GPS
  //TinyGPS_init();
  chprintf(chp, "OK\r\n");

  /*
   * SPI1 I/O pins setup.
   */
  chprintf(chp, "SPI...");
  /*
   * Initializes the SPI driver 1 in order to access the MEMS. The signals
   * are initialized in the board file.
   * Several LIS302DL registers are then initialized.
   */
  spiStart(&SPID1, &spi1cfg);
  lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG1, 0x43);
  lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG2, 0x00);
  lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG3, 0x00);
  chprintf(chp, "OK\r\n");

  /*
   * Receiver I/O
   */
  chprintf(chp, "Receiver...");
  palSetPadMode(GPIOE, 10, PIN_MODE_INPUT(9));  // THRO
  palSetPadMode(GPIOE, 11, PIN_MODE_INPUT(10)); // AILE
  palSetPadMode(GPIOE, 12, PIN_MODE_INPUT(11)); // ELEV
  palSetPadMode(GPIOE, 13, PIN_MODE_INPUT(12)); // RUDD
  palSetPadMode(GPIOE, 14, PIN_MODE_INPUT(13)); // GEAR
  palSetPadMode(GPIOE, 15, PIN_MODE_INPUT(14)); // AUX1
  chprintf(chp, "OK\r\n");

  chprintf(chp, "I/O configured.\r\n");

  /*
   * Creates the threads
   */
  chprintf(chp, "Launching threads...");
  chThdCreateStatic(BlinkWA, sizeof(BlinkWA), NORMALPRIO, Blink, NULL);
  chThdCreateStatic(GPSWA, sizeof(GPSWA), NORMALPRIO, GPS, NULL);
  chprintf(chp, "OK\r\n");

  /*
   * Normal main() thread activity
   */
  while (TRUE) {
    chThdSleepMilliseconds(500);

    long lat, lon;
    unsigned long fix_age;
    TinyGPS_get_position(&lat, &lon, &fix_age);

    chprintf(chp, "GPS location: %d, %d, %d\r\n", lat, lon, fix_age);

    int year;
    char month, day, hour, minute, second, hundredths;
    TinyGPS_crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);
    chprintf(chp, "GPS date/time: %d/%d/%d, %d:%d:%d.%d, %d\r\n", year, month, day, hour, minute, second, fix_age);
  }
}
