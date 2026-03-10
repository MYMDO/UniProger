/**
 * @file hal_pio.h
 * @brief HAL PIO / bitbang engine interface — platform-independent
 *
 * On RP2040, this maps to the PIO state machine hardware.
 * On other platforms, it falls back to GPIO bitbang.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_HAL_PIO_H
#define UNIPROGER_HAL_PIO_H

#include "uniproger/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** PIO program handle */
typedef struct hal_pio_program hal_pio_program_t;

/** PIO state machine handle */
typedef struct hal_pio_sm hal_pio_sm_t;

/** PIO configuration */
typedef struct {
    const void *program_data;     /**< Platform-specific program binary */
    size_t      program_len;      /**< Program length */
    uint32_t    clock_div;        /**< Clock divider (16.8 fixed-point on RP2040) */
    up_pin_t    pin_base;         /**< Base pin for set/out/sideset */
    uint8_t     pin_count;        /**< Number of consecutive pins */
    up_pin_t    pin_in_base;      /**< Base pin for IN */
    up_pin_t    pin_sideset_base; /**< Sideset base pin (UP_PIN_NONE if unused) */
    uint8_t     sideset_count;    /**< Number of sideset pins */
} hal_pio_config_t;

/**
 * @brief Initialize PIO and load program
 * @param[out] sm    State machine handle
 * @param      cfg   Configuration
 */
up_status_t hal_pio_init(hal_pio_sm_t **sm, const hal_pio_config_t *cfg);

/**
 * @brief Deinitialize PIO state machine
 */
up_status_t hal_pio_deinit(hal_pio_sm_t *sm);

/**
 * @brief Start PIO state machine execution
 */
up_status_t hal_pio_start(hal_pio_sm_t *sm);

/**
 * @brief Stop PIO state machine
 */
up_status_t hal_pio_stop(hal_pio_sm_t *sm);

/**
 * @brief Put a 32-bit word into PIO TX FIFO (blocking)
 */
up_status_t hal_pio_put(hal_pio_sm_t *sm, uint32_t data);

/**
 * @brief Get a 32-bit word from PIO RX FIFO (blocking)
 */
up_status_t hal_pio_get(hal_pio_sm_t *sm, uint32_t *data);

/**
 * @brief Put data into TX FIFO (non-blocking)
 * @return UP_OK if written, UP_ERROR_BUSY if FIFO full
 */
up_status_t hal_pio_try_put(hal_pio_sm_t *sm, uint32_t data);

/**
 * @brief Get data from RX FIFO (non-blocking)
 * @return UP_OK if read, UP_ERROR_BUSY if FIFO empty
 */
up_status_t hal_pio_try_get(hal_pio_sm_t *sm, uint32_t *data);

/**
 * @brief Set clock divider for state machine
 */
up_status_t hal_pio_set_clkdiv(hal_pio_sm_t *sm, uint32_t clkdiv);

/**
 * @brief Execute a single instruction on the state machine
 */
up_status_t hal_pio_exec(hal_pio_sm_t *sm, uint16_t instruction);

/**
 * @brief Check if PIO hardware is available on this platform
 */
bool hal_pio_available(void);

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_HAL_PIO_H */
