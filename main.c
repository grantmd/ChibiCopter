/*
	ChibiCopter - https://github.com/grantmd/ChibiCopter
	A quadcopter platform running under ChibiOS/RT.

	Based on demo code from ChibiOS/RT
*/

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "Comms.h"
#include "Accel.h"
#include "Spektrum.h"
#include "Motors.h"
#include "GPS.h"

/*
 * This is a periodic thread that blinks some leds
 */
static WORKING_AREA(BlinkWA, 128);
static msg_t Blink(void *arg){

	(void)arg;
	chRegSetThreadName("blinker");
	while (TRUE){
		//palSetPad(GPIOD, GPIOD_LED5); // red
		chThdSleepMilliseconds(100);
		//palClearPad(GPIOD, GPIOD_LED5); // red
		palSetPad(GPIOD, GPIOD_LED6); // blue
		chThdSleepMilliseconds(100);
		palClearPad(GPIOD, GPIOD_LED6); // blue
		//palSetPad(GPIOD, GPIOD_LED4); // green
		chThdSleepMilliseconds(100);
		//palClearPad(GPIOD, GPIOD_LED4); // green
		palSetPad(GPIOD, GPIOD_LED3); // orange
		chThdSleepMilliseconds(100);
		palClearPad(GPIOD, GPIOD_LED3); // orange
	}
	return 0;
}

/*
 * Application entry point.
 */
int main(void){

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
	 * Startup comms
	 */

	CommsInit();

	/*
	 * Sensor I/O
	 */
	AccelInit();

	/*
	 * Receiver I/O
	 */
	SpektrumInit();

	/*
	 * Motors I/O
	 */
	MotorsInit();

	/*
	 * GPS
	 */
	GPSInit();

	/*
	 * Creates the threads
	 */
	chThdCreateStatic(BlinkWA, sizeof(BlinkWA), NORMALPRIO, Blink, NULL);

	/*
	 * Normal main() thread activity
	 */

	/*******************************************************************
	  // tasks (microseconds of interval)
	  ReadGyro        readGyro      (   5000); // 200hz
	  ReadAccel       readAccel     (   5000); // 200hz
	  RunDCM          runDCM        (  10000); // 100hz
	  FlightControls  flightControls(  10000); // 100hz
	  ReadReceiver    readReceiver  (  20000); //  50hz
	  ReadBaro        readBaro      (  40000); //  25hz
	  ReadCompass     readCompass   ( 100000); //  10Hz
	  ProcessTelem    processTelem  ( 100000); //  10Hz
	  ReadBattery     readBattery   ( 100000); //  10Hz
	*******************************************************************/

	systime_t previousTime = chTimeNow();
	systime_t currentTime = 0;
	systime_t deltaTime;
	uint8_t frameCounter = 0;
	while (TRUE){
		currentTime = chTimeNow();
  		deltaTime = currentTime - previousTime;

  		// Main scheduler loop set for 100hz
  		if (deltaTime >= US2ST(10000)){
			frameCounter++;

			/*
			 * 100hz task loop
	 		 */
			if (frameCounter % 1 == 0){
				AccelRead();
			}

			/*
			 * 50hz task loop
	 		 */
			if (frameCounter % 2 == 0){
				MotorsSetSpeed(0, getSpektrumData(THROTTLE_CHANNEL));
			}

			/*
			 * 25hz task loop
	 		 */
			if (frameCounter % 4 == 0){
			}

			/*
			 * 10hz task loop
	 		 */
			if (frameCounter % 10 == 0){
			}

			/*
			 * 1hz task loop
	 		 */
			if (frameCounter % 100 == 0){
				CommsHeartbeat();
			}

			previousTime = currentTime;
			if (frameCounter >= 100) frameCounter = 0;
		}
	}
}
