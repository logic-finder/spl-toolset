#ifndef FATAL_H
#define FATAL_H

#include <stdarg.h>

#define ERR(msg) fatal(ENPREFIX msg)
#define VERR(msg, ...) vfatal(ENPREFIX msg, __VA_ARGS__)

/**
 * @brief `fatal` prints an error message `msg` to `stderr`, exiting 1.
 *    The string "fatal: " is prepended. A newline is appended at EOL.
 * @param msg an error message
 */
void fatal(const char *msg);

/**
 * @brief `vfatal` prints a format string `msg` to `stderr`, exiting 1.
 *    The string "fatal: " is prepended. A newline is appended at EOL.
 * @param msg a format string
 * @param va variable number of arguments
 */
void vfatal(const char *msg, ...);

#endif
