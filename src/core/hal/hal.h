/**
 * @file hal.h
 * @brief Master HAL include — aggregates all HAL sub-interfaces
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_HAL_H
#define UNIPROGER_HAL_H

#include "uniproger/config.h"
#include "uniproger/types.h"

/* Sub-HAL interfaces */
#include "hal_gpio.h"
#include "hal_spi.h"
#include "hal_i2c.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "hal_pio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize platform hardware (clocks, peripherals, USB, etc.)
 * @return UP_OK on success
 */
up_status_t hal_platform_init(void);

/**
 * @brief Deinitialize platform hardware
 */
up_status_t hal_platform_deinit(void);

/**
 * @brief Get platform name string (e.g. "RP2040", "STM32F4")
 */
const char *hal_platform_name(void);

/**
 * @brief Get unique chip ID (if available)
 * @param[out] id     Buffer for chip ID
 * @param      id_len Buffer length
 * @return Actual ID length, or 0 if not available
 */
size_t hal_platform_get_chip_id(uint8_t *id, size_t id_len);

/**
 * @brief Get system clock frequency in Hz
 */
uint32_t hal_platform_get_clock_hz(void);

/**
 * @brief System reset
 */
void hal_platform_reset(void) __attribute__((noreturn));

/**
 * @brief Set system LED state
 */
void hal_platform_led(bool on);

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_HAL_H */
