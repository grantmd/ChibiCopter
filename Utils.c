/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Utility functions
*/

#include "Utils.h"

/*
 * Replacement square root function -- calling regular math sqrt caused program halt
 * This is probably faster anyway
 * From: https://github.com/dancollins/Quad-Rotor/blob/master/QuadRotor/imu.c
 */

float nr_sqrt(float input) {
	float output = 10.0; // Holds output value of itteration, and initial guess
	float old_output = 0.0; // Stores old output incase we can terminate early
	int i; // Used for the for loop.  Declare here for C99

	for (i=0; i<NR_ITTR_COUNT; i++) {
		old_output = output;
		output = output - (((output*output)-input)/(2*output)); // SQRT itteration
		if (old_output == output) // If the value has not changed
			break;
	}

	return output;
}