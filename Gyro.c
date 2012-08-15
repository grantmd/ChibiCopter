/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Read gyro data
*/

#include "ch.h"
#include "hal.h"

#include "Gyro.h"

#include <math.h>

uint8_t gyro_x, gyro_y, gyro_z;

void GyroInit(void){
}

void GyroRead(void){
}

float GyroGetRollAngle(void){
	return atan2f( (float)gyro_x, (float)sqrt( pow(gyro_y, 2) + pow(gyro_z, 2) ) );
}

float GyroGetPitchAngle(void){
	return atan2f( (float)gyro_y, (float)sqrt( pow(gyro_x, 2) + pow(gyro_z, 2) ) );
}

float GyroGetYawAngle(void){
	return atan2f( (float)sqrt( pow(gyro_x, 2) + pow(gyro_y, 2) ), (float)gyro_z );
}