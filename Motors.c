/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Manages motor speeds and sends them via PWM
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "Motors.h"

static PWMConfig pwmcfg = {
  10000,                                    /* 10KHz PWM clock frequency.   */
  10000,                                    /* Initial PWM period 1S.       */
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  0,
  0
};

pwmcnt_t motor_speeds[NUM_MOTORS];

void MotorosInit(void){
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

	motor_speeds[motor] = speed;
	pwmEnableChannel(&PWMD8, motor, speed);
}