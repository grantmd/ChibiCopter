/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Serial helper functions
*/

#include "ch.h"
#include "hal.h"

#include "serial_help.h"

/**
 * @brief   Prints a line without final end-of-line.
 *
 * @param[in] msgp      the message
 */
void serial_print(const char *msgp) {

  while (*msgp)
    chIOPut(chp, *msgp++);
}

/**
 * @brief   Prints a line.
 *
 * @param[in] msgp      the message
 */
void serial_println(const char *msgp) {

  serial_print(msgp);
  chIOPut(chp, '\r');
  chIOPut(chp, '\n');
}