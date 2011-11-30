/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Based on demo code from ChibiOS/RT
*/

#include "ch.h"
#include "hal.h"

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
  i2cStart(&I2CD1, &i2cfg1);

  /*
   * Activates the serial driver 1 using the driver default configuration.
   * PA2(TX) and PA3(RX) are routed to USART1.
   */
  sdStart(&SD2, NULL);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

  /*
   * Creates the blink thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Setup the accelerometer.
   */
  accel_init();

  /*
   * Normal main() thread activity
   */
  while (TRUE) {
    chThdSleepMilliseconds(500);
    //accel_read();
  }
}
