/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Serial helper functions
*/

#include "ch.h"
#include "hal.h"

#include "serial_help.h"

static BaseChannel *chp;

/**
 * @brief   Prints a decimal unsigned number.
 *
 * @param[in] n         the number to be printed
 */
void serial_printn(uint32_t n) {
  char buf[16], *p;

  if (!n)
    chIOPut(chp, '0');
  else {
    p = buf;
    while (n)
      *p++ = (n % 10) + '0', n /= 10;
    while (p > buf)
      chIOPut(chp, *--p);
  }
}

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