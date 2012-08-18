/*
	ChibiCopter - https://github.com/grantmd/ChibiCopter
	A quadcopter platform running under ChibiOS/RT.

	Based on demo code from ChibiOS/RT
*/

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "Comms.h"
#include "I2CSensor.h"
#include "Accel.h"
#include "Gyro.h"
#include "Spektrum.h"
#include "Motors.h"
#include "TinyGPS.h"
#include "GPS.h"

#include <mavlink.h>

#define ST2MS(st)   ((st / CH_FREQUENCY) * 1000L)

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

	systime_t startTime = chTimeNow();

	/*
	 * Startup comms
	 */

	CommsInit();

	/*
	 * Sensor I/O
	 */
	mavlink_system.state = MAV_STATE_CALIBRATING;
	I2CSensorInit();
	AccelInit();
	GyroInit();

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

	mavlink_system.state = MAV_STATE_STANDBY;
	mavlink_system.mode = MAV_MODE_STABILIZE_DISARMED;

	/*******************************************************************
	  // tasks (microseconds of interval)
	  ReadGyro        (   5000); // 200hz
	  ReadAccel       (   5000); // 200hz
	  RunDCM          (  10000); // 100hz
	  FlightControls  (  10000); // 100hz
	  ReadReceiver    (  20000); //  50hz
	  ReadBaro        (  40000); //  25hz
	  ReadCompass     ( 100000); //  10Hz
	  ProcessTelem    ( 100000); //  10Hz
	  ReadBattery     ( 100000); //  10Hz
	  ProcessGPS      ( 200000); //   5Hz
	  Heartbeats      (1000000); //   1Hz
	*******************************************************************/

	systime_t previousTime = chTimeNow();
	systime_t currentTime = 0;
	systime_t deltaTime;
	uint8_t frameCounter = 0;

	float accelRoll = 0.0;
	float accelPitch = 0.0;
	float accelYaw = 0.0;

	uint8_t gpsFixType = 0;
	uint32_t fixAge = 0;
	int32_t latitude = 0;
	int32_t longitude = 0;
	int32_t altitude = 0;
	uint16_t velocity = 0;
	uint16_t cog = 0;

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
				GyroRead();
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
				accelRoll = AccelGetRollAngle();
				accelPitch = AccelGetPitchAngle();
				accelYaw = AccelGetYawAngle();
				CommsSendAttitude(ST2MS(currentTime - startTime), accelRoll, accelPitch, accelYaw, 0, 0, 0);
			}

			/*
			 * 5hz task loop
			 */
			if (frameCounter % 20 == 0){
				TinyGPS_get_position((int32_t*)latitude, (int32_t*)longitude, (uint32_t*)fixAge);
				altitude = TinyGPS_altitude()*10;
				velocity = TinyGPS_f_speed_mps()*100;
				cog = TinyGPS_f_course()*100;

				gpsFixType = 0;
				if (fixAge != GPS_INVALID_AGE) gpsFixType = 3;

				CommsSendGPSRaw(ST2MS(currentTime - startTime), gpsFixType, latitude, longitude, altitude, 65535, 65535, velocity, cog, 255);
			}

			/*
			 * 1hz task loop
	 		 */
			if (frameCounter % 100 == 0){
				CommsSendSysStatus();
				CommsSendHeartbeat();
			}

			previousTime = currentTime;
			if (frameCounter >= 100) frameCounter = 0;
		}
	}
}
