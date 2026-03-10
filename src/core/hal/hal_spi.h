/**
 * @file hal_spi.h
 * @brief HAL SPI interface — platform-independent
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_HAL_SPI_H
#define UNIPROGER_HAL_SPI_H

#include "uniproger/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** SPI instance handle (opaque, platform-defined) */
typedef struct hal_spi_inst hal_spi_inst_t;

/** SPI configuration */
typedef struct {
    uint8_t      instance;    /**< HW SPI instance index (0, 1, ...) */
    uint32_t     freq_hz;     /**< Clock frequency in Hz */
    up_spi_mode_t mode;       /**< SPI mode (0-3) */
    up_spi_bit_order_t bit_order;
    up_pin_t     pin_sck;
    up_pin_t     pin_mosi;
    up_pin_t     pin_miso;
    up_pin_t     pin_cs;      /**< UP_PIN_NONE if SW-controlled */
} hal_spi_config_t;

/**
 * @brief Initialize SPI peripheral
 * @param[out] inst   Pointer to receive instance handle
 * @param      cfg    Configuration
 */
up_status_t hal_spi_init(hal_spi_inst_t **inst, const hal_spi_config_t *cfg);

/**
 * @brief Deinitialize SPI peripheral
 */
up_status_t hal_spi_deinit(hal_spi_inst_t *inst);

/**
 * @brief Change SPI clock frequency
 */
up_status_t hal_spi_set_freq(hal_spi_inst_t *inst, uint32_t freq_hz);

/**
 * @brief Full-duplex transfer
 * @param inst   SPI instance
 * @param tx     TX buffer (NULL = send zeros)
 * @param rx     RX buffer (NULL = discard)
 * @param len    Transfer length in bytes
 */
up_status_t hal_spi_transfer(hal_spi_inst_t *inst,
                              const uint8_t *tx, uint8_t *rx, size_t len);

/**
 * @brief Write-only transfer (faster on some platforms)
 */
up_status_t hal_spi_write(hal_spi_inst_t *inst,
                           const uint8_t *data, size_t len);

/**
 * @brief Read-only transfer (sends zeros on MOSI)
 */
up_status_t hal_spi_read(hal_spi_inst_t *inst,
                          uint8_t *data, size_t len);

/**
 * @brief DMA-accelerated transfer (if available, falls back to blocking)
 */
up_status_t hal_spi_transfer_dma(hal_spi_inst_t *inst,
                                  const uint8_t *tx, uint8_t *rx, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_HAL_SPI_H */
