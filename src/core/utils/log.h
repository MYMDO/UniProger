/**
 * @file log.h
 * @brief Logging with levels, colors, timestamps
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_LOG_H
#define UNIPROGER_LOG_H

#include "uniproger/config.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void up_log_output(int level, const char *tag, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

#define UP_LOG_TRACE(tag, ...) \
    do { if (UP_LOG_LEVEL <= UP_LOG_LEVEL_TRACE) up_log_output(UP_LOG_LEVEL_TRACE, tag, __VA_ARGS__); } while(0)

#define UP_LOG_DEBUG(tag, ...) \
    do { if (UP_LOG_LEVEL <= UP_LOG_LEVEL_DEBUG) up_log_output(UP_LOG_LEVEL_DEBUG, tag, __VA_ARGS__); } while(0)

#define UP_LOG_INFO(tag, ...) \
    do { if (UP_LOG_LEVEL <= UP_LOG_LEVEL_INFO)  up_log_output(UP_LOG_LEVEL_INFO,  tag, __VA_ARGS__); } while(0)

#define UP_LOG_WARN(tag, ...) \
    do { if (UP_LOG_LEVEL <= UP_LOG_LEVEL_WARN)  up_log_output(UP_LOG_LEVEL_WARN,  tag, __VA_ARGS__); } while(0)

#define UP_LOG_ERROR(tag, ...) \
    do { if (UP_LOG_LEVEL <= UP_LOG_LEVEL_ERROR) up_log_output(UP_LOG_LEVEL_ERROR, tag, __VA_ARGS__); } while(0)

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_LOG_H */
