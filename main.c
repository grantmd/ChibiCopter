/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Based on demo code from ChibiOS/RT
*/

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "serial_help.h"
#include "LIS302DL.h"

BaseChannel *chp; // serial port

/* I2C1 config */
static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000, // clock speed
    FAST_DUTY_CYCLE_16_9,
};

/*
 * SPI configuration (21MHz(???), CPHA=0, CPOL=0, MSb first).
 */
static const SPIConfig spicfg = {
  NULL,
  GPIOA,
  4,
  SPI_CR1_BR_1
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
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED5);
    palSetPad(GPIOD, GPIOD_LED6);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED6);
    palSetPad(GPIOD, GPIOD_LED4);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED4);
    palSetPad(GPIOD, GPIOD_LED3);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED3);
  }
  return 0;
}

/*
 * Reads accel data off the spi bus
 */
static WORKING_AREA(LIS302DLWA, 128);
static msg_t LIS302DLPoll(void *arg) {

  (void)arg;
  chRegSetThreadName("LIS302DL");
  while (TRUE) {
    chThdSleepMilliseconds(32);
    LIS302DL_read();
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
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
  serial_println("Hello, startup!");

  /*
   * Init the I2C subsystem
   */
  serial_println("Configurating I/O:");
  serial_print("I2C...");
  i2cInit();
  i2cStart(&I2CD1, &i2cfg1);
  serial_println("OK");

  /*
   * SPI1 I/O pins setup.
   */
  serial_print("SPI...");
  palSetPadMode(GPIOA, 5, PAL_MODE_ALTERNATE(5) |
                           PAL_STM32_OSPEED_HIGHEST);       /* New SCK.     */
  palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(5) |
                           PAL_STM32_OSPEED_HIGHEST);       /* New MISO.    */
  palSetPadMode(GPIOA, 7, PAL_MODE_ALTERNATE(5) |
                           PAL_STM32_OSPEED_HIGHEST);       /* New MOSI.    */
  palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL |
                           PAL_STM32_OSPEED_HIGHEST);       /* New CS.      */
  palSetPad(GPIOA, 4);
  spiStart(&SPID1, &spicfg);
  serial_println("OK");
  serial_println("I/O configured.");

  /*
   * Setup the accelerometer.
   */
  serial_print("Activating Accelerometer...");
  LIS302DL_init();
  serial_println("OK");

  /*
   * Creates the threads
   */
  serial_print("Launching threads...");
  chThdCreateStatic(BlinkWA, sizeof(BlinkWA), NORMALPRIO, Blink, NULL);
  chThdCreateStatic(LIS302DLWA, sizeof(LIS302DLWA), NORMALPRIO, LIS302DLPoll, NULL);
  serial_println("OK");

  /*
   * Normal main() thread activity
   */
  while (TRUE) {
    chThdSleepMilliseconds(500);
    serial_print("Accel x:");
    serial_printn(get_LIS302DL_x());
    serial_println("");

    serial_print("Accel y:");
    serial_printn(get_LIS302DL_y());
    serial_println("");

    serial_print("Accel z:");
    serial_printn(get_LIS302DL_z());
    serial_println("");
  }
}
