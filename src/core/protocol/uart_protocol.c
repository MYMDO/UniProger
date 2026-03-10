/**
 * @file uart_protocol.c
 * @brief UART protocol engine — passthrough, sniffer
 *
 * SPDX-License-Identifier: MIT
 */

#include "protocol.h"
#include "src/core/hal/hal.h"

typedef struct {
    hal_uart_config_t hal_cfg;
} uart_proto_config_t;

typedef struct {
    hal_uart_inst_t *uart;
} uart_proto_priv_t;

static uart_proto_priv_t s_uart_priv;

static up_status_t uart_proto_init(up_protocol_t *proto, const void *config)
{
    const uart_proto_config_t *cfg = (const uart_proto_config_t *)config;
    uart_proto_priv_t *priv = &s_uart_priv;
    up_status_t st = hal_uart_init(&priv->uart, &cfg->hal_cfg);
    if (st != UP_OK) return st;
    proto->priv = priv;
    proto->hw_inst = priv->uart;
    return UP_OK;
}

static up_status_t uart_proto_deinit(up_protocol_t *proto)
{
    return hal_uart_deinit(((uart_proto_priv_t *)proto->priv)->uart);
}

static up_status_t uart_proto_reset(up_protocol_t *proto)
{
    return hal_uart_flush(((uart_proto_priv_t *)proto->priv)->uart);
}

static up_status_t uart_proto_transfer(up_protocol_t *proto,
                                        const uint8_t *tx, size_t tx_len,
                                        uint8_t *rx, size_t rx_len)
{
    uart_proto_priv_t *priv = (uart_proto_priv_t *)proto->priv;
    up_status_t st = UP_OK;
    if (tx && tx_len > 0) {
        st = hal_uart_write(priv->uart, tx, tx_len, UP_DEFAULT_TIMEOUT_MS);
        if (st != UP_OK) return st;
    }
    if (rx && rx_len > 0) {
        size_t actual = 0;
        st = hal_uart_read(priv->uart, rx, rx_len, &actual, UP_DEFAULT_TIMEOUT_MS);
    }
    return st;
}

static up_status_t uart_proto_detect(up_protocol_t *proto,
                                      uint32_t *ids, size_t max_ids, size_t *found)
{
    UP_UNUSED(proto); UP_UNUSED(ids); UP_UNUSED(max_ids);
    *found = 0;
    return UP_OK;
}

static up_status_t uart_proto_set_speed(up_protocol_t *proto, uint32_t speed_hz)
{
    return hal_uart_set_baud(((uart_proto_priv_t *)proto->priv)->uart, speed_hz);
}

const up_protocol_ops_t uart_protocol_ops = {
    .name      = "UART",
    .type      = UP_PROTO_UART,
    .init      = uart_proto_init,
    .deinit    = uart_proto_deinit,
    .reset     = uart_proto_reset,
    .transfer  = uart_proto_transfer,
    .detect    = uart_proto_detect,
    .set_speed = uart_proto_set_speed,
};
