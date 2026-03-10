/**
 * @file spi_protocol.c
 * @brief SPI protocol engine — CS management, multi-byte transfer, mode config
 *
 * SPDX-License-Identifier: MIT
 */

#include "protocol.h"
#include "src/core/hal/hal.h"
#include <string.h>

/* ── SPI protocol config ─────────────────────────────────────────────── */

typedef struct {
    hal_spi_config_t  hal_cfg;
} spi_proto_config_t;

typedef struct {
    hal_spi_inst_t *spi;
    up_pin_t        cs_pin;
} spi_proto_priv_t;

static spi_proto_priv_t s_spi_priv;

/* ── CS control ──────────────────────────────────────────────────────── */

static inline void spi_cs_select(spi_proto_priv_t *p)
{
    if (p->cs_pin != UP_PIN_NONE) {
        hal_gpio_write(p->cs_pin, UP_GPIO_LEVEL_LOW);
        hal_timer_delay_us(1);
    }
}

static inline void spi_cs_deselect(spi_proto_priv_t *p)
{
    if (p->cs_pin != UP_PIN_NONE) {
        hal_timer_delay_us(1);
        hal_gpio_write(p->cs_pin, UP_GPIO_LEVEL_HIGH);
    }
}

/* ── Protocol ops ────────────────────────────────────────────────────── */

static up_status_t spi_proto_init(up_protocol_t *proto, const void *config)
{
    const spi_proto_config_t *cfg = (const spi_proto_config_t *)config;
    spi_proto_priv_t *priv = &s_spi_priv;

    priv->cs_pin = cfg->hal_cfg.pin_cs;

    up_status_t st = hal_spi_init(&priv->spi, &cfg->hal_cfg);
    if (st != UP_OK) return st;

    proto->priv = priv;
    proto->hw_inst = priv->spi;

    return UP_OK;
}

static up_status_t spi_proto_deinit(up_protocol_t *proto)
{
    spi_proto_priv_t *priv = (spi_proto_priv_t *)proto->priv;
    if (!priv || !priv->spi) return UP_ERROR_INVALID_ARG;
    return hal_spi_deinit(priv->spi);
}

static up_status_t spi_proto_reset(up_protocol_t *proto)
{
    spi_proto_priv_t *priv = (spi_proto_priv_t *)proto->priv;
    spi_cs_deselect(priv);
    hal_timer_delay_ms(10);
    return UP_OK;
}

static up_status_t spi_proto_transfer(up_protocol_t *proto,
                                       const uint8_t *tx, size_t tx_len,
                                       uint8_t *rx, size_t rx_len)
{
    spi_proto_priv_t *priv = (spi_proto_priv_t *)proto->priv;

    spi_cs_select(priv);

    up_status_t st = UP_OK;

    /* Send command/address bytes */
    if (tx && tx_len > 0) {
        st = hal_spi_write(priv->spi, tx, tx_len);
        if (st != UP_OK) goto done;
    }

    /* Read response bytes */
    if (rx && rx_len > 0) {
        st = hal_spi_read(priv->spi, rx, rx_len);
    }

done:
    spi_cs_deselect(priv);
    return st;
}

static up_status_t spi_proto_detect(up_protocol_t *proto,
                                     uint32_t *ids, size_t max_ids, size_t *found)
{
    *found = 0;

    /* Try JEDEC READ ID (0x9F) */
    uint8_t cmd = 0x9F;
    uint8_t resp[3] = {0};

    up_status_t st = spi_proto_transfer(proto, &cmd, 1, resp, 3);
    if (st != UP_OK) return st;

    uint32_t jedec_id = ((uint32_t)resp[0] << 16) |
                        ((uint32_t)resp[1] << 8)  |
                        (uint32_t)resp[2];

    /* 0xFFFFFF or 0x000000 means no device */
    if (jedec_id != 0xFFFFFF && jedec_id != 0x000000) {
        if (max_ids > 0) {
            ids[0] = jedec_id;
        }
        *found = 1;
    }

    return UP_OK;
}

static up_status_t spi_proto_set_speed(up_protocol_t *proto, uint32_t speed_hz)
{
    spi_proto_priv_t *priv = (spi_proto_priv_t *)proto->priv;
    return hal_spi_set_freq(priv->spi, speed_hz);
}

/* ── Public vtable ───────────────────────────────────────────────────── */

const up_protocol_ops_t spi_protocol_ops = {
    .name      = "SPI",
    .type      = UP_PROTO_SPI,
    .init      = spi_proto_init,
    .deinit    = spi_proto_deinit,
    .reset     = spi_proto_reset,
    .transfer  = spi_proto_transfer,
    .detect    = spi_proto_detect,
    .set_speed = spi_proto_set_speed,
};
