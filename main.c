/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Based on demo code from ChibiOS/RT
*/

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "serial_help.h"
#include "accel.h"

/* I2C1 config */
static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    100000, // clock speed
    STD_DUTY_CYCLE,
    0,
    0,
    0,
    0,
};

/*
 * This is a periodic thread that blinks some leds
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

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
   * Init the I2C subsystem
   */
  i2cInit();
  i2cStart(&I2CD1, &i2cfg1);

  /* tune ports for I2C1*/
  //palSetPadMode(IOPORT2, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
  //palSetPadMode(IOPORT2, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);

  /*
   * Activates the serial driver 2 using the driver default configuration.
   * PA2(TX) and PA3(RX) are routed to USART2.
   */

   // FIXME: USART2 instead?
  sdStart(&SD2, NULL);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
  serial_println("Hello, startup!");

  /*
   * Creates the blink thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Setup the accelerometer.
   */
  serial_print("Activating Accelerometer...");
  accel_init();
  serial_println("OK");

  /*
   * Normal main() thread activity
   */
  while (TRUE) {
    chThdSleepMilliseconds(500);
    //accel_read();
  }
}
