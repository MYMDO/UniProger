/**
 * @file i2c_protocol.c
 * @brief I2C protocol engine — address scan, page writes, ACK polling
 *
 * SPDX-License-Identifier: MIT
 */

#include "protocol.h"
#include "src/core/hal/hal.h"
#include <string.h>

/* ── I2C protocol private data ───────────────────────────────────────── */

typedef struct {
    hal_i2c_config_t hal_cfg;
} i2c_proto_config_t;

typedef struct {
    hal_i2c_inst_t *i2c;
    uint8_t         target_addr;  /**< Current target device address */
} i2c_proto_priv_t;

static i2c_proto_priv_t s_i2c_priv;

/* ── Protocol ops ────────────────────────────────────────────────────── */

static up_status_t i2c_proto_init(up_protocol_t *proto, const void *config)
{
    const i2c_proto_config_t *cfg = (const i2c_proto_config_t *)config;
    i2c_proto_priv_t *priv = &s_i2c_priv;

    priv->target_addr = 0;

    up_status_t st = hal_i2c_init(&priv->i2c, &cfg->hal_cfg);
    if (st != UP_OK) return st;

    proto->priv = priv;
    proto->hw_inst = priv->i2c;

    return UP_OK;
}

static up_status_t i2c_proto_deinit(up_protocol_t *proto)
{
    i2c_proto_priv_t *priv = (i2c_proto_priv_t *)proto->priv;
    return hal_i2c_deinit(priv->i2c);
}

static up_status_t i2c_proto_reset(up_protocol_t *proto)
{
    UP_UNUSED(proto);
    hal_timer_delay_ms(10);
    return UP_OK;
}

static up_status_t i2c_proto_transfer(up_protocol_t *proto,
                                       const uint8_t *tx, size_t tx_len,
                                       uint8_t *rx, size_t rx_len)
{
    i2c_proto_priv_t *priv = (i2c_proto_priv_t *)proto->priv;

    if (tx && tx_len > 0 && rx && rx_len > 0) {
        return hal_i2c_write_read(priv->i2c, priv->target_addr,
                                   tx, tx_len, rx, rx_len,
                                   UP_DEFAULT_TIMEOUT_MS);
    } else if (tx && tx_len > 0) {
        return hal_i2c_write(priv->i2c, priv->target_addr,
                              tx, tx_len, false, UP_DEFAULT_TIMEOUT_MS);
    } else if (rx && rx_len > 0) {
        return hal_i2c_read(priv->i2c, priv->target_addr,
                             rx, rx_len, false, UP_DEFAULT_TIMEOUT_MS);
    }

    return UP_ERROR_INVALID_ARG;
}

static up_status_t i2c_proto_detect(up_protocol_t *proto,
                                     uint32_t *ids, size_t max_ids, size_t *found)
{
    i2c_proto_priv_t *priv = (i2c_proto_priv_t *)proto->priv;

    uint8_t addrs[128];
    size_t addr_count = 0;

    up_status_t st = hal_i2c_scan(priv->i2c, addrs, sizeof(addrs), &addr_count);
    if (st != UP_OK) return st;

    *found = UP_MIN(addr_count, max_ids);
    for (size_t i = 0; i < *found; i++) {
        ids[i] = (uint32_t)addrs[i];
    }

    return UP_OK;
}

static up_status_t i2c_proto_set_speed(up_protocol_t *proto, uint32_t speed_hz)
{
    UP_UNUSED(proto);
    UP_UNUSED(speed_hz);
    /* I2C speed is set at init time on most platforms */
    return UP_ERROR_NOT_SUPPORTED;
}

/* ── Public vtable ───────────────────────────────────────────────────── */

const up_protocol_ops_t i2c_protocol_ops = {
    .name      = "I2C",
    .type      = UP_PROTO_I2C,
    .init      = i2c_proto_init,
    .deinit    = i2c_proto_deinit,
    .reset     = i2c_proto_reset,
    .transfer  = i2c_proto_transfer,
    .detect    = i2c_proto_detect,
    .set_speed = i2c_proto_set_speed,
};
