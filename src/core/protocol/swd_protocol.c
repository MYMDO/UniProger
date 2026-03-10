/**
 * @file swd_protocol.c
 * @brief SWD (Serial Wire Debug) protocol engine — line reset, AP/DP access
 *
 * SPDX-License-Identifier: MIT
 */

#include "protocol.h"
#include "src/core/hal/hal.h"
#include <string.h>

/* ── SWD constants ───────────────────────────────────────────────────── */

#define SWD_ACK_OK     0x01
#define SWD_ACK_WAIT   0x02
#define SWD_ACK_FAULT  0x04

#define SWD_DP   0  /* Debug Port */
#define SWD_AP   1  /* Access Port */

/* DP register addresses */
#define DP_DPIDR     0x00
#define DP_CTRL_STAT 0x04
#define DP_SELECT    0x08
#define DP_RDBUFF    0x0C

/* ── SWD private data ───────────────────────────────────────────────── */

typedef struct {
    up_pin_t pin_swclk;
    up_pin_t pin_swdio;
    up_pin_t pin_reset;
    uint32_t freq_hz;
} swd_proto_config_t;

typedef struct {
    up_pin_t swclk;
    up_pin_t swdio;
    up_pin_t reset;
    uint32_t delay_us;
} swd_priv_t;

static swd_priv_t s_swd_priv;

/* ── Low-level bitbang ───────────────────────────────────────────────── */

static inline void swd_clock_pulse(swd_priv_t *s)
{
    hal_gpio_write(s->swclk, UP_GPIO_LEVEL_HIGH);
    if (s->delay_us) hal_timer_delay_us(s->delay_us);
    hal_gpio_write(s->swclk, UP_GPIO_LEVEL_LOW);
    if (s->delay_us) hal_timer_delay_us(s->delay_us);
}

static inline void swd_write_bit(swd_priv_t *s, bool bit)
{
    hal_gpio_write(s->swdio, bit ? UP_GPIO_LEVEL_HIGH : UP_GPIO_LEVEL_LOW);
    swd_clock_pulse(s);
}

static inline bool swd_read_bit(swd_priv_t *s)
{
    hal_gpio_write(s->swclk, UP_GPIO_LEVEL_HIGH);
    if (s->delay_us) hal_timer_delay_us(s->delay_us);
    bool bit = hal_gpio_read(s->swdio) == UP_GPIO_LEVEL_HIGH;
    hal_gpio_write(s->swclk, UP_GPIO_LEVEL_LOW);
    if (s->delay_us) hal_timer_delay_us(s->delay_us);
    return bit;
}

static void swd_write_bits(swd_priv_t *s, uint32_t data, uint8_t bits)
{
    hal_gpio_set_dir(s->swdio, UP_GPIO_DIR_OUTPUT);
    for (uint8_t i = 0; i < bits; i++) {
        swd_write_bit(s, (data >> i) & 1);
    }
}

static uint32_t swd_read_bits(swd_priv_t *s, uint8_t bits)
{
    hal_gpio_set_dir(s->swdio, UP_GPIO_DIR_INPUT);
    uint32_t data = 0;
    for (uint8_t i = 0; i < bits; i++) {
        if (swd_read_bit(s)) {
            data |= (1u << i);
        }
    }
    return data;
}

static void swd_turnaround(swd_priv_t *s)
{
    swd_clock_pulse(s);
}

/* ── SWD line reset (50+ clocks with SWDIO high) ────────────────────── */

static void swd_line_reset(swd_priv_t *s)
{
    hal_gpio_set_dir(s->swdio, UP_GPIO_DIR_OUTPUT);
    hal_gpio_write(s->swdio, UP_GPIO_LEVEL_HIGH);
    for (int i = 0; i < 56; i++) {
        swd_clock_pulse(s);
    }
    /* JTAG-to-SWD switch sequence: 0xE79E */
    swd_write_bits(s, 0xE79E, 16);
    /* Another line reset */
    hal_gpio_write(s->swdio, UP_GPIO_LEVEL_HIGH);
    for (int i = 0; i < 56; i++) {
        swd_clock_pulse(s);
    }
    /* Idle clocks */
    hal_gpio_write(s->swdio, UP_GPIO_LEVEL_LOW);
    for (int i = 0; i < 8; i++) {
        swd_clock_pulse(s);
    }
}

/* ── SWD read/write ──────────────────────────────────────────────────── */

static uint8_t swd_calc_parity(uint32_t data)
{
    uint32_t p = data;
    p ^= p >> 16;
    p ^= p >> 8;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return p & 1;
}

/**
 * @brief Build SWD request packet header
 * @param ap_ndp  0=DP, 1=AP
 * @param rnw     0=write, 1=read
 * @param addr    Register address (A[3:2])
 */
static uint8_t swd_request(uint8_t ap_ndp, uint8_t rnw, uint8_t addr)
{
    uint8_t req = 0x81; /* Start=1, Park=1 */
    if (ap_ndp) req |= (1 << 1);
    if (rnw)    req |= (1 << 2);
    req |= ((addr >> 2) & 0x03) << 3;
    /* Parity of APnDP, RnW, A[2], A[3] */
    uint8_t parity = ((req >> 1) ^ (req >> 2) ^ (req >> 3) ^ (req >> 4)) & 1;
    req |= (parity << 5);
    return req;
}

static up_status_t swd_read_reg(swd_priv_t *s, uint8_t ap_ndp,
                                 uint8_t addr, uint32_t *value)
{
    uint8_t req = swd_request(ap_ndp, 1, addr);

    /* Send request */
    swd_write_bits(s, req, 8);

    /* Turnaround */
    swd_turnaround(s);

    /* Read ACK */
    uint8_t ack = swd_read_bits(s, 3);
    if (ack != SWD_ACK_OK) {
        swd_turnaround(s);
        return (ack == SWD_ACK_WAIT) ? UP_ERROR_BUSY : UP_ERROR_IO;
    }

    /* Read 32-bit data */
    *value = swd_read_bits(s, 32);

    /* Read parity */
    uint8_t parity = swd_read_bit(s);
    UP_UNUSED(parity); /* TODO: verify */

    /* Turnaround back to output */
    swd_turnaround(s);

    /* Idle clocks */
    swd_write_bits(s, 0, 8);

    return UP_OK;
}

static up_status_t swd_write_reg(swd_priv_t *s, uint8_t ap_ndp,
                                  uint8_t addr, uint32_t value)
{
    uint8_t req = swd_request(ap_ndp, 0, addr);

    /* Send request */
    swd_write_bits(s, req, 8);

    /* Turnaround */
    swd_turnaround(s);

    /* Read ACK */
    uint8_t ack = swd_read_bits(s, 3);

    /* Turnaround back to output */
    swd_turnaround(s);

    if (ack != SWD_ACK_OK) {
        return (ack == SWD_ACK_WAIT) ? UP_ERROR_BUSY : UP_ERROR_IO;
    }

    /* Write 32-bit data + parity */
    swd_write_bits(s, value, 32);
    swd_write_bit(s, swd_calc_parity(value));

    /* Idle clocks */
    swd_write_bits(s, 0, 8);

    return UP_OK;
}

/* ── Protocol ops ────────────────────────────────────────────────────── */

static up_status_t swd_proto_init(up_protocol_t *proto, const void *config)
{
    const swd_proto_config_t *cfg = (const swd_proto_config_t *)config;
    swd_priv_t *s = &s_swd_priv;

    s->swclk = cfg->pin_swclk;
    s->swdio = cfg->pin_swdio;
    s->reset = cfg->pin_reset;

    if (cfg->freq_hz > 0) {
        s->delay_us = 500000 / cfg->freq_hz;
        if (s->delay_us == 0) s->delay_us = 1;
    } else {
        s->delay_us = 5;
    }

    hal_gpio_init(s->swclk, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_NONE);
    hal_gpio_init(s->swdio, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_UP);
    hal_gpio_write(s->swclk, UP_GPIO_LEVEL_LOW);

    if (s->reset != UP_PIN_NONE) {
        hal_gpio_init(s->reset, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_NONE);
        hal_gpio_write(s->reset, UP_GPIO_LEVEL_HIGH);
    }

    proto->priv = s;

    /* Perform line reset + JTAG-to-SWD sequence */
    swd_line_reset(s);

    return UP_OK;
}

static up_status_t swd_proto_deinit(up_protocol_t *proto)
{
    swd_priv_t *s = (swd_priv_t *)proto->priv;
    hal_gpio_deinit(s->swclk);
    hal_gpio_deinit(s->swdio);
    if (s->reset != UP_PIN_NONE) hal_gpio_deinit(s->reset);
    return UP_OK;
}

static up_status_t swd_proto_reset(up_protocol_t *proto)
{
    swd_priv_t *s = (swd_priv_t *)proto->priv;

    if (s->reset != UP_PIN_NONE) {
        hal_gpio_write(s->reset, UP_GPIO_LEVEL_LOW);
        hal_timer_delay_ms(50);
        hal_gpio_write(s->reset, UP_GPIO_LEVEL_HIGH);
        hal_timer_delay_ms(50);
    }

    swd_line_reset(s);
    return UP_OK;
}

static up_status_t swd_proto_transfer(up_protocol_t *proto,
                                       const uint8_t *tx, size_t tx_len,
                                       uint8_t *rx, size_t rx_len)
{
    swd_priv_t *s = (swd_priv_t *)proto->priv;

    if (tx && tx_len >= 5) {
        /* Write: first byte = AP/DP|addr flags, next 4 bytes = data */
        uint8_t ap_ndp = (tx[0] >> 4) & 1;
        uint8_t addr = tx[0] & 0x0F;
        uint32_t data = ((uint32_t)tx[4] << 24) | ((uint32_t)tx[3] << 16) |
                        ((uint32_t)tx[2] << 8)  | (uint32_t)tx[1];
        return swd_write_reg(s, ap_ndp, addr, data);
    }

    if (tx && tx_len == 1 && rx && rx_len >= 4) {
        /* Read: first byte = AP/DP|addr flags */
        uint8_t ap_ndp = (tx[0] >> 4) & 1;
        uint8_t addr = tx[0] & 0x0F;
        uint32_t data = 0;
        up_status_t st = swd_read_reg(s, ap_ndp, addr, &data);
        if (st == UP_OK) {
            rx[0] = data & 0xFF;
            rx[1] = (data >> 8) & 0xFF;
            rx[2] = (data >> 16) & 0xFF;
            rx[3] = (data >> 24) & 0xFF;
        }
        return st;
    }

    return UP_ERROR_INVALID_ARG;
}

static up_status_t swd_proto_detect(up_protocol_t *proto,
                                     uint32_t *ids, size_t max_ids, size_t *found)
{
    swd_priv_t *s = (swd_priv_t *)proto->priv;

    *found = 0;

    swd_line_reset(s);

    uint32_t dpidr = 0;
    up_status_t st = swd_read_reg(s, SWD_DP, DP_DPIDR, &dpidr);
    if (st != UP_OK) return st;

    if (dpidr != 0 && dpidr != 0xFFFFFFFF && max_ids > 0) {
        ids[0] = dpidr;
        *found = 1;
    }

    return UP_OK;
}

static up_status_t swd_proto_set_speed(up_protocol_t *proto, uint32_t speed_hz)
{
    swd_priv_t *s = (swd_priv_t *)proto->priv;
    if (speed_hz > 0) {
        s->delay_us = 500000 / speed_hz;
        if (s->delay_us == 0) s->delay_us = 1;
    }
    return UP_OK;
}

/* ── Public vtable ───────────────────────────────────────────────────── */

const up_protocol_ops_t swd_protocol_ops = {
    .name      = "SWD",
    .type      = UP_PROTO_SWD,
    .init      = swd_proto_init,
    .deinit    = swd_proto_deinit,
    .reset     = swd_proto_reset,
    .transfer  = swd_proto_transfer,
    .detect    = swd_proto_detect,
    .set_speed = swd_proto_set_speed,
};
