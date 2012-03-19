/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Manages motor speeds and sends them via PWM
*/

#ifndef _MOTORS_H_
#define _MOTORS_H_

#define NUM_MOTORS 4
#define MAX_MOTOR_SPEED 10000

void MotorsInit(void);

void MotorsSetSpeed(unsigned motor, pwmcnt_t speed);
pwmcnt_t MotorsGetSpeed(unsigned motor);

#endif /* _MOTORS_H_ */