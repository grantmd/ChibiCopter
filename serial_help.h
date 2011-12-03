/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Serial helper functions
*/

#include "ch.h"

#ifndef SERIAL_HELP_H_
#define SERIAL_HELP_H_

extern BaseChannel *chp;

void serial_printn(uint32_t n);
void serial_print(const char *msgp);
void serial_println(const char *msgp);

#endif /* SERIAL_HELP_H_ */