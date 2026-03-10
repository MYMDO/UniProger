/**
 * @file rp2040_hal_spi.c
 * @brief RP2040 SPI HAL implementation using hardware SPI + DMA
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/hal/hal_spi.h"

#ifdef UNIPROGER_PLATFORM_RP2040

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include <stdlib.h>
#include <string.h>

/* ── Instance storage ────────────────────────────────────────────────── */

struct hal_spi_inst {
    spi_inst_t *hw_spi;
    up_pin_t    pin_cs;
    int         dma_tx;
    int         dma_rx;
};

static struct hal_spi_inst s_spi_instances[2];

/* ── SPI API ─────────────────────────────────────────────────────────── */

up_status_t hal_spi_init(hal_spi_inst_t **inst, const hal_spi_config_t *cfg)
{
    if (!inst || !cfg || cfg->instance > 1) return UP_ERROR_INVALID_ARG;

    spi_inst_t *hw = cfg->instance == 0 ? spi0 : spi1;
    struct hal_spi_inst *si = &s_spi_instances[cfg->instance];

    si->hw_spi = hw;
    si->pin_cs = cfg->pin_cs;

    /* Initialize SPI hardware */
    spi_init(hw, cfg->freq_hz);

    /* Set format: 8 bits, specified mode */
    spi_set_format(hw, 8,
                   (cfg->mode & 0x02) ? SPI_CPOL_1 : SPI_CPOL_0,
                   (cfg->mode & 0x01) ? SPI_CPHA_1 : SPI_CPHA_0,
                   SPI_MSB_FIRST);

    /* Configure pins */
    gpio_set_function(cfg->pin_sck,  GPIO_FUNC_SPI);
    gpio_set_function(cfg->pin_mosi, GPIO_FUNC_SPI);
    gpio_set_function(cfg->pin_miso, GPIO_FUNC_SPI);

    /* CS: software controlled for flexibility */
    if (cfg->pin_cs != UP_PIN_NONE) {
        gpio_init(cfg->pin_cs);
        gpio_set_dir(cfg->pin_cs, GPIO_OUT);
        gpio_put(cfg->pin_cs, 1); /* CS deasserted */
    }

    /* Claim DMA channels */
    si->dma_tx = dma_claim_unused_channel(false);
    si->dma_rx = dma_claim_unused_channel(false);

    *inst = si;
    return UP_OK;
}

up_status_t hal_spi_deinit(hal_spi_inst_t *inst)
{
    if (!inst) return UP_ERROR_INVALID_ARG;

    spi_deinit(inst->hw_spi);

    if (inst->dma_tx >= 0) dma_channel_unclaim(inst->dma_tx);
    if (inst->dma_rx >= 0) dma_channel_unclaim(inst->dma_rx);
    inst->dma_tx = -1;
    inst->dma_rx = -1;

    return UP_OK;
}

up_status_t hal_spi_set_freq(hal_spi_inst_t *inst, uint32_t freq_hz)
{
    if (!inst) return UP_ERROR_INVALID_ARG;
    spi_set_baudrate(inst->hw_spi, freq_hz);
    return UP_OK;
}

up_status_t hal_spi_transfer(hal_spi_inst_t *inst,
                              const uint8_t *tx, uint8_t *rx, size_t len)
{
    if (!inst || len == 0) return UP_ERROR_INVALID_ARG;

    if (tx && rx) {
        /* Full duplex */
        spi_write_read_blocking(inst->hw_spi, tx, rx, len);
    } else if (tx) {
        spi_write_blocking(inst->hw_spi, tx, len);
    } else if (rx) {
        spi_read_blocking(inst->hw_spi, 0x00, rx, len);
    }

    return UP_OK;
}

up_status_t hal_spi_write(hal_spi_inst_t *inst,
                           const uint8_t *data, size_t len)
{
    return hal_spi_transfer(inst, data, NULL, len);
}

up_status_t hal_spi_read(hal_spi_inst_t *inst,
                          uint8_t *data, size_t len)
{
    return hal_spi_transfer(inst, NULL, data, len);
}

up_status_t hal_spi_transfer_dma(hal_spi_inst_t *inst,
                                  const uint8_t *tx, uint8_t *rx, size_t len)
{
    if (!inst || len == 0) return UP_ERROR_INVALID_ARG;

    /* Fall back to blocking if DMA not available */
    if (inst->dma_tx < 0 || inst->dma_rx < 0) {
        return hal_spi_transfer(inst, tx, rx, len);
    }

    static uint8_t dummy_tx = 0x00;
    static uint8_t dummy_rx;

    /* Configure TX DMA */
    dma_channel_config tx_cfg = dma_channel_get_default_config(inst->dma_tx);
    channel_config_set_transfer_data_size(&tx_cfg, DMA_SIZE_8);
    channel_config_set_dreq(&tx_cfg, spi_get_dreq(inst->hw_spi, true));

    if (tx) {
        channel_config_set_read_increment(&tx_cfg, true);
        dma_channel_configure(inst->dma_tx, &tx_cfg,
                              &spi_get_hw(inst->hw_spi)->dr, tx, len, false);
    } else {
        channel_config_set_read_increment(&tx_cfg, false);
        dma_channel_configure(inst->dma_tx, &tx_cfg,
                              &spi_get_hw(inst->hw_spi)->dr, &dummy_tx, len, false);
    }

    /* Configure RX DMA */
    dma_channel_config rx_cfg = dma_channel_get_default_config(inst->dma_rx);
    channel_config_set_transfer_data_size(&rx_cfg, DMA_SIZE_8);
    channel_config_set_dreq(&rx_cfg, spi_get_dreq(inst->hw_spi, false));

    if (rx) {
        channel_config_set_write_increment(&rx_cfg, true);
        dma_channel_configure(inst->dma_rx, &rx_cfg,
                              rx, &spi_get_hw(inst->hw_spi)->dr, len, false);
    } else {
        channel_config_set_write_increment(&rx_cfg, false);
        dma_channel_configure(inst->dma_rx, &rx_cfg,
                              &dummy_rx, &spi_get_hw(inst->hw_spi)->dr, len, false);
    }

    /* Start both channels simultaneously */
    dma_start_channel_mask((1u << inst->dma_tx) | (1u << inst->dma_rx));

    /* Wait for completion */
    dma_channel_wait_for_finish_blocking(inst->dma_rx);

    return UP_OK;
}

#endif /* UNIPROGER_PLATFORM_RP2040 */
