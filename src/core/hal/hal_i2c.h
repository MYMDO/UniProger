/**
 * @file hal_i2c.h
 * @brief HAL I2C interface — platform-independent
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_HAL_I2C_H
#define UNIPROGER_HAL_I2C_H

#include "uniproger/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** I2C instance handle */
typedef struct hal_i2c_inst hal_i2c_inst_t;

/** I2C configuration */
typedef struct {
    uint8_t      instance;
    uint32_t     freq_hz;
    up_pin_t     pin_sda;
    up_pin_t     pin_scl;
} hal_i2c_config_t;

/**
 * @brief Initialize I2C peripheral
 */
up_status_t hal_i2c_init(hal_i2c_inst_t **inst, const hal_i2c_config_t *cfg);

/**
 * @brief Deinitialize I2C
 */
up_status_t hal_i2c_deinit(hal_i2c_inst_t *inst);

/**
 * @brief Write data to I2C device
 * @param inst       I2C instance
 * @param addr       7-bit device address
 * @param data       Data to write
 * @param len        Data length
 * @param nostop     If true, don't send STOP condition (for repeated start)
 * @param timeout_ms Timeout in milliseconds
 */
up_status_t hal_i2c_write(hal_i2c_inst_t *inst, uint8_t addr,
                           const uint8_t *data, size_t len,
                           bool nostop, uint32_t timeout_ms);

/**
 * @brief Read data from I2C device
 */
up_status_t hal_i2c_read(hal_i2c_inst_t *inst, uint8_t addr,
                          uint8_t *data, size_t len,
                          bool nostop, uint32_t timeout_ms);

/**
 * @brief Write then read (combined transaction)
 */
up_status_t hal_i2c_write_read(hal_i2c_inst_t *inst, uint8_t addr,
                                const uint8_t *wr_data, size_t wr_len,
                                uint8_t *rd_data, size_t rd_len,
                                uint32_t timeout_ms);

/**
 * @brief Scan I2C bus for devices
 * @param inst       I2C instance
 * @param addrs      Output array of found addresses
 * @param max_addrs  Max entries in addrs
 * @param found      Number of devices found
 */
up_status_t hal_i2c_scan(hal_i2c_inst_t *inst,
                          uint8_t *addrs, size_t max_addrs, size_t *found);

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_HAL_I2C_H */
