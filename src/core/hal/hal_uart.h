/**
 * @file hal_uart.h
 * @brief HAL UART interface — platform-independent
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_HAL_UART_H
#define UNIPROGER_HAL_UART_H

#include "uniproger/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** UART instance handle */
typedef struct hal_uart_inst hal_uart_inst_t;

/** UART RX callback */
typedef void (*hal_uart_rx_cb_t)(hal_uart_inst_t *inst,
                                  const uint8_t *data, size_t len,
                                  void *user_data);

/** UART configuration */
typedef struct {
    uint8_t              instance;
    uint32_t             baud_rate;
    up_uart_data_bits_t  data_bits;
    up_uart_parity_t     parity;
    up_uart_stop_bits_t  stop_bits;
    up_pin_t             pin_tx;
    up_pin_t             pin_rx;
    hal_uart_rx_cb_t     rx_callback;
    void                *rx_user_data;
} hal_uart_config_t;

/**
 * @brief Initialize UART
 */
up_status_t hal_uart_init(hal_uart_inst_t **inst, const hal_uart_config_t *cfg);

/**
 * @brief Deinitialize UART
 */
up_status_t hal_uart_deinit(hal_uart_inst_t *inst);

/**
 * @brief Change baud rate on the fly
 */
up_status_t hal_uart_set_baud(hal_uart_inst_t *inst, uint32_t baud_rate);

/**
 * @brief Write data to UART (blocking)
 */
up_status_t hal_uart_write(hal_uart_inst_t *inst,
                            const uint8_t *data, size_t len,
                            uint32_t timeout_ms);

/**
 * @brief Read data from UART (blocking with timeout)
 * @param[out] actual  Number of bytes actually read
 */
up_status_t hal_uart_read(hal_uart_inst_t *inst,
                           uint8_t *data, size_t len,
                           size_t *actual, uint32_t timeout_ms);

/**
 * @brief Check if data is available for reading
 */
bool hal_uart_available(hal_uart_inst_t *inst);

/**
 * @brief Flush TX buffer
 */
up_status_t hal_uart_flush(hal_uart_inst_t *inst);

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_HAL_UART_H */
