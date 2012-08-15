/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Read gyro data
*/

#ifndef _GYRO_H_
#define _GYRO_H_

extern uint8_t gyro_x, gyro_y, gyro_z;

// Public functions
void GyroInit(void);
void GyroRead(void);

float GyroGetRollAngle(void);
float GyroGetPitchAngle(void);
float GyroGetYawAngle(void);

#endif /* _GYRO_H_ */