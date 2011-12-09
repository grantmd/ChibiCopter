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

BaseChannel *chp; // serial port

/* I2C1 config */
static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000, // clock speed
    FAST_DUTY_CYCLE_16_9,
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
 * Reads accel data off the i2c bus
 */
static WORKING_AREA(AccelWA, 128);
static msg_t AccelPoll(void *arg) {

  (void)arg;
  chRegSetThreadName("accel");
  while (TRUE) {
    chThdSleepMilliseconds(32);
    accel_read();
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
  i2cInit();
  i2cStart(&I2CD1, &i2cfg1);

  /*
   * Creates the threads
   */
  chThdCreateStatic(BlinkWA, sizeof(BlinkWA), NORMALPRIO, Blink, NULL);
  chThdCreateStatic(AccelWA, sizeof(AccelWA), NORMALPRIO, AccelPoll, NULL);

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
  }
}
