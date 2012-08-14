/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Read accelerometer data over SPI: the LIS302DL
*/

#ifndef _ACCEL_H_
#define _ACCEL_H_

extern uint8_t accel_x, accel_y, accel_z;

// Public functions
void AccelInit(void);
void AccelRead(void);

float AccelGetRollAngle(void);
float AccelGetPitchAngle(void);
float AccelGetYawAngle(void);

#endif /* _ACCEL_H_ */