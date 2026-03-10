/**
 * @file onewire_protocol.c
 * @brief 1-Wire protocol engine — reset, ROM search, read/write
 *
 * SPDX-License-Identifier: MIT
 */

#include "protocol.h"
#include "src/core/hal/hal.h"

typedef struct {
    up_pin_t pin_data;
} onewire_proto_config_t;

typedef struct {
    up_pin_t data_pin;
} onewire_priv_t;

static onewire_priv_t s_ow_priv;

/* ── Low-level timing (standard speed) ───────────────────────────────── */

static bool ow_reset(onewire_priv_t *ow)
{
    hal_gpio_set_dir(ow->data_pin, UP_GPIO_DIR_OUTPUT);
    hal_gpio_write(ow->data_pin, UP_GPIO_LEVEL_LOW);
    hal_timer_delay_us(480);
    hal_gpio_set_dir(ow->data_pin, UP_GPIO_DIR_INPUT);
    hal_timer_delay_us(70);
    bool presence = (hal_gpio_read(ow->data_pin) == UP_GPIO_LEVEL_LOW);
    hal_timer_delay_us(410);
    return presence;
}

static void ow_write_bit(onewire_priv_t *ow, bool bit)
{
    hal_gpio_set_dir(ow->data_pin, UP_GPIO_DIR_OUTPUT);
    hal_gpio_write(ow->data_pin, UP_GPIO_LEVEL_LOW);
    if (bit) {
        hal_timer_delay_us(6);
        hal_gpio_set_dir(ow->data_pin, UP_GPIO_DIR_INPUT);
        hal_timer_delay_us(64);
    } else {
        hal_timer_delay_us(60);
        hal_gpio_set_dir(ow->data_pin, UP_GPIO_DIR_INPUT);
        hal_timer_delay_us(10);
    }
}

static bool ow_read_bit(onewire_priv_t *ow)
{
    hal_gpio_set_dir(ow->data_pin, UP_GPIO_DIR_OUTPUT);
    hal_gpio_write(ow->data_pin, UP_GPIO_LEVEL_LOW);
    hal_timer_delay_us(6);
    hal_gpio_set_dir(ow->data_pin, UP_GPIO_DIR_INPUT);
    hal_timer_delay_us(9);
    bool bit = (hal_gpio_read(ow->data_pin) == UP_GPIO_LEVEL_HIGH);
    hal_timer_delay_us(55);
    return bit;
}

static void ow_write_byte(onewire_priv_t *ow, uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        ow_write_bit(ow, (byte >> i) & 1);
    }
}

static uint8_t ow_read_byte(onewire_priv_t *ow)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        if (ow_read_bit(ow)) byte |= (1 << i);
    }
    return byte;
}

/* ── Protocol ops ────────────────────────────────────────────────────── */

static up_status_t ow_proto_init(up_protocol_t *proto, const void *config)
{
    const onewire_proto_config_t *cfg = (const onewire_proto_config_t *)config;
    onewire_priv_t *priv = &s_ow_priv;
    priv->data_pin = cfg->pin_data;
    hal_gpio_init(priv->data_pin, UP_GPIO_DIR_INPUT, UP_GPIO_PULL_UP);
    proto->priv = priv;
    return UP_OK;
}

static up_status_t ow_proto_deinit(up_protocol_t *proto)
{
    onewire_priv_t *priv = (onewire_priv_t *)proto->priv;
    hal_gpio_deinit(priv->data_pin);
    return UP_OK;
}

static up_status_t ow_proto_reset(up_protocol_t *proto)
{
    onewire_priv_t *priv = (onewire_priv_t *)proto->priv;
    return ow_reset(priv) ? UP_OK : UP_ERROR_NO_DEVICE;
}

static up_status_t ow_proto_transfer(up_protocol_t *proto,
                                      const uint8_t *tx, size_t tx_len,
                                      uint8_t *rx, size_t rx_len)
{
    onewire_priv_t *priv = (onewire_priv_t *)proto->priv;
    if (!ow_reset(priv)) return UP_ERROR_NO_DEVICE;
    for (size_t i = 0; i < tx_len; i++) ow_write_byte(priv, tx[i]);
    for (size_t i = 0; i < rx_len; i++) rx[i] = ow_read_byte(priv);
    return UP_OK;
}

static up_status_t ow_proto_detect(up_protocol_t *proto,
                                    uint32_t *ids, size_t max_ids, size_t *found)
{
    onewire_priv_t *priv = (onewire_priv_t *)proto->priv;
    *found = 0;
    if (!ow_reset(priv)) return UP_OK;

    /* Read ROM command: 0x33 */
    ow_write_byte(priv, 0x33);
    uint8_t rom[8];
    for (int i = 0; i < 8; i++) rom[i] = ow_read_byte(priv);

    uint32_t id = ((uint32_t)rom[3] << 24) | ((uint32_t)rom[2] << 16) |
                  ((uint32_t)rom[1] << 8)  | rom[0];
    if (id != 0 && id != 0xFFFFFFFF && max_ids > 0) {
        ids[0] = id;
        *found = 1;
    }
    return UP_OK;
}

static up_status_t ow_proto_set_speed(up_protocol_t *proto, uint32_t speed_hz)
{
    UP_UNUSED(proto); UP_UNUSED(speed_hz);
    return UP_ERROR_NOT_SUPPORTED;
}

const up_protocol_ops_t onewire_protocol_ops = {
    .name      = "1-Wire",
    .type      = UP_PROTO_ONEWIRE,
    .init      = ow_proto_init,
    .deinit    = ow_proto_deinit,
    .reset     = ow_proto_reset,
    .transfer  = ow_proto_transfer,
    .detect    = ow_proto_detect,
    .set_speed = ow_proto_set_speed,
};
