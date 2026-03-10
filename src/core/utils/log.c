/**
 * @file log.c
 * @brief Logging implementation with ANSI colors and timestamps
 *
 * SPDX-License-Identifier: MIT
 */

#include "log.h"
#include "src/core/hal/hal_timer.h"
#include <stdarg.h>
#include <stdio.h>

static const char *level_names[] = {
    "TRACE", "DEBUG", "INFO ", "WARN ", "ERROR"
};

#if UP_LOG_USE_COLOR
static const char *level_colors[] = {
    "\033[90m",   /* TRACE: gray */
    "\033[36m",   /* DEBUG: cyan */
    "\033[32m",   /* INFO:  green */
    "\033[33m",   /* WARN:  yellow */
    "\033[31m",   /* ERROR: red */
};
#define COLOR_RESET "\033[0m"
#else
#define COLOR_RESET ""
#endif

void up_log_output(int level, const char *tag, const char *fmt, ...)
{
    if (level < UP_LOG_LEVEL_TRACE || level > UP_LOG_LEVEL_ERROR) return;

#if UP_LOG_USE_TIMESTAMP
    uint32_t ms = hal_timer_get_ms();
    printf("[%6lu.%03lu] ", (unsigned long)(ms / 1000), (unsigned long)(ms % 1000));
#endif

#if UP_LOG_USE_COLOR
    printf("%s%s" COLOR_RESET " [%s] ", level_colors[level], level_names[level], tag);
#else
    printf("%s [%s] ", level_names[level], tag);
#endif

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}
