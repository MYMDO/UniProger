/**
 * @file hal_timer.h
 * @brief HAL Timer interface — platform-independent
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_HAL_TIMER_H
#define UNIPROGER_HAL_TIMER_H

#include "uniproger/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Periodic timer callback */
typedef void (*hal_timer_cb_t)(void *user_data);

/**
 * @brief Delay for specified microseconds (blocking)
 */
void hal_timer_delay_us(uint32_t us);

/**
 * @brief Delay for specified milliseconds (blocking)
 */
void hal_timer_delay_ms(uint32_t ms);

/**
 * @brief Get current tick count in microseconds
 * @return Monotonic microsecond counter (wraps after ~71 minutes on 32-bit)
 */
uint64_t hal_timer_get_us(void);

/**
 * @brief Get current tick count in milliseconds
 */
uint32_t hal_timer_get_ms(void);

/**
 * @brief Start a periodic timer
 * @param period_us  Period in microseconds
 * @param cb         Callback function
 * @param user_data  Opaque user data
 * @return UP_OK on success
 */
up_status_t hal_timer_start_periodic(uint32_t period_us,
                                      hal_timer_cb_t cb, void *user_data);

/**
 * @brief Stop periodic timer
 */
up_status_t hal_timer_stop_periodic(void);

/**
 * @brief Check if a timeout has elapsed
 * @param start_us   Start time from hal_timer_get_us()
 * @param timeout_us Timeout duration in microseconds
 * @return true if timeout elapsed
 */
static inline bool hal_timer_timeout(uint64_t start_us, uint64_t timeout_us)
{
    return (hal_timer_get_us() - start_us) >= timeout_us;
}

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_HAL_TIMER_H */
