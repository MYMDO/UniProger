/**
 * @file config.h
 * @brief UniProger global configuration
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 UniProger Contributors
 */

#ifndef UNIPROGER_CONFIG_H
#define UNIPROGER_CONFIG_H

/* ── Platform selection ──────────────────────────────────────────────── */
/* Defined via CMake: UNIPROGER_PLATFORM_RP2040, _STM32, _ESP32, etc.  */

/* ── Buffer configuration ────────────────────────────────────────────── */
#ifndef UP_BUFFER_SIZE
#define UP_BUFFER_SIZE           (4096U)
#endif

#ifndef UP_CLI_LINE_SIZE
#define UP_CLI_LINE_SIZE         (256U)
#endif

#ifndef UP_CLI_HISTORY_DEPTH
#define UP_CLI_HISTORY_DEPTH     (16U)
#endif

#ifndef UP_CLI_MAX_ARGS
#define UP_CLI_MAX_ARGS          (16U)
#endif

/* ── Device registry ─────────────────────────────────────────────────── */
#ifndef UP_MAX_DEVICES
#define UP_MAX_DEVICES           (32U)
#endif

#ifndef UP_MAX_PROTOCOLS
#define UP_MAX_PROTOCOLS         (8U)
#endif

/* ── Logging ─────────────────────────────────────────────────────────── */
#ifndef UP_LOG_LEVEL
#define UP_LOG_LEVEL             UP_LOG_LEVEL_INFO
#endif

#define UP_LOG_LEVEL_TRACE  0
#define UP_LOG_LEVEL_DEBUG  1
#define UP_LOG_LEVEL_INFO   2
#define UP_LOG_LEVEL_WARN   3
#define UP_LOG_LEVEL_ERROR  4
#define UP_LOG_LEVEL_NONE   5

#ifndef UP_LOG_USE_COLOR
#define UP_LOG_USE_COLOR         1
#endif

#ifndef UP_LOG_USE_TIMESTAMP
#define UP_LOG_USE_TIMESTAMP     1
#endif

/* ── Timeouts (ms) ───────────────────────────────────────────────────── */
#ifndef UP_DEFAULT_TIMEOUT_MS
#define UP_DEFAULT_TIMEOUT_MS    (5000U)
#endif

#ifndef UP_FLASH_WRITE_TIMEOUT_MS
#define UP_FLASH_WRITE_TIMEOUT_MS (10000U)
#endif

#ifndef UP_ERASE_TIMEOUT_MS
#define UP_ERASE_TIMEOUT_MS      (60000U)
#endif

/* ── SPI defaults ────────────────────────────────────────────────────── */
#ifndef UP_SPI_DEFAULT_FREQ_HZ
#define UP_SPI_DEFAULT_FREQ_HZ   (1000000U)   /* 1 MHz */
#endif

#ifndef UP_SPI_MAX_FREQ_HZ
#define UP_SPI_MAX_FREQ_HZ       (62500000U)  /* 62.5 MHz (RP2040 max) */
#endif

/* ── I2C defaults ────────────────────────────────────────────────────── */
#ifndef UP_I2C_DEFAULT_FREQ_HZ
#define UP_I2C_DEFAULT_FREQ_HZ   (100000U)    /* 100 kHz */
#endif

/* ── UART defaults ───────────────────────────────────────────────────── */
#ifndef UP_UART_DEFAULT_BAUD
#define UP_UART_DEFAULT_BAUD     (115200U)
#endif

/* ── FreeRTOS task config ────────────────────────────────────────────── */
#ifndef UP_CLI_TASK_STACK_SIZE
#define UP_CLI_TASK_STACK_SIZE   (4096U)
#endif

#ifndef UP_CLI_TASK_PRIORITY
#define UP_CLI_TASK_PRIORITY     (1U)
#endif

#ifndef UP_WORKER_TASK_STACK_SIZE
#define UP_WORKER_TASK_STACK_SIZE (8192U)
#endif

#ifndef UP_WORKER_TASK_PRIORITY
#define UP_WORKER_TASK_PRIORITY  (2U)
#endif

/* ── Feature flags ───────────────────────────────────────────────────── */
#ifndef UP_FEATURE_EMULATION
#define UP_FEATURE_EMULATION     1
#endif

#ifndef UP_FEATURE_ANALYZER
#define UP_FEATURE_ANALYZER      1
#endif

#ifndef UP_FEATURE_JTAG
#define UP_FEATURE_JTAG          1
#endif

#ifndef UP_FEATURE_SWD
#define UP_FEATURE_SWD           1
#endif

#ifndef UP_FEATURE_ONEWIRE
#define UP_FEATURE_ONEWIRE       1
#endif

#endif /* UNIPROGER_CONFIG_H */
