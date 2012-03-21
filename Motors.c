/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Manages motor speeds and sends them via PWM
*/

#include "ch.h"
#include "hal.h"

#include "Motors.h"

static PWMConfig pwmcfg = {
	1000000, /* 1MHz PWM clock frequency.   */
	2500,   /* Initial PWM period 20ms.    */ // 50hz (20ms) for standard servo/ESC, 400hz for fast servo/ESC (2.5ms)
	NULL,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL}
	},
	0,
#if STM32_PWM_USE_ADVANCED
	0
#endif
};

pwmcnt_t motor_speeds[NUM_MOTORS];

void MotorsInit(void){

	pwmStart(&PWMD8, &pwmcfg);
	palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(3));
	palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(3));
	palSetPadMode(GPIOC, 8, PAL_MODE_ALTERNATE(3));
	palSetPadMode(GPIOC, 9, PAL_MODE_ALTERNATE(3));

	unsigned i;
	for (i=0; i<NUM_MOTORS; i++){
		MotorsSetSpeed(i, 0);
	}
}

void MotorsSetSpeed(unsigned motor, pwmcnt_t speed){
	if (motor >= NUM_MOTORS) return;
	if (speed > MAX_MOTOR_SPEED) speed = MAX_MOTOR_SPEED;

	motor_speeds[motor] = speed;
	pwmEnableChannel(&PWMD8, motor, PWM_PERCENTAGE_TO_WIDTH(&PWMD8, speed));
}

pwmcnt_t MotorsGetSpeed(unsigned motor){
	if (motor >= NUM_MOTORS) return 0;
	return motor_speeds[motor];
}