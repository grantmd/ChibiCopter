/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Manages motor speeds and sends them via PWM
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "Motors.h"

static void pwmpcb(PWMDriver *pwmp) {

	(void)pwmp;
	palClearPad(GPIOD, GPIOD_LED5); // red
}

static void pwmc1cb(PWMDriver *pwmp) {

	(void)pwmp;
	palSetPad(GPIOD, GPIOD_LED5); // red
}

static PWMConfig pwmcfg = {
	10000,                                    /* 10KHz PWM clock frequency.   */
	10000,                                    /* Initial PWM period 1S.       */
	pwmpcb,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, pwmc1cb},
		{PWM_OUTPUT_ACTIVE_HIGH, pwmc1cb},
		{PWM_OUTPUT_ACTIVE_HIGH, pwmc1cb},
		{PWM_OUTPUT_ACTIVE_HIGH, pwmc1cb}
	},
	0,
};

pwmcnt_t motor_speeds[NUM_MOTORS];

void MotorsInit(void){

	unsigned i;
	for (i=0; i<NUM_MOTORS; i++){
		motor_speeds[i] = 0;
	}

	pwmStart(&PWMD8, &pwmcfg);
	palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(3));
	palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(3));
	palSetPadMode(GPIOC, 8, PAL_MODE_ALTERNATE(3));
	palSetPadMode(GPIOC, 9, PAL_MODE_ALTERNATE(3));
}

void MotorsSetSpeed(unsigned motor, pwmcnt_t speed){
	if (motor >= NUM_MOTORS) return;
	if (speed > MAX_MOTOR_SPEED) speed = MAX_MOTOR_SPEED;

	motor_speeds[motor] = speed;
	pwmEnableChannel(&PWMD8, motor, PWM_FRACTION_TO_WIDTH(&PWMD8, 1000, speed));
}

pwmcnt_t MotorsGetSpeed(unsigned motor){
	if (motor >= NUM_MOTORS) return 0;
	return motor_speeds[motor];
}