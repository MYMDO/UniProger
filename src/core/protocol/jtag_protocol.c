/**
 * @file jtag_protocol.c
 * @brief JTAG protocol engine — TAP state machine, IR/DR scan, chain detection
 *
 * Supports both PIO-accelerated and GPIO bitbang modes.
 *
 * SPDX-License-Identifier: MIT
 */

#include "protocol.h"
#include "src/core/hal/hal.h"
#include <string.h>

/* ── JTAG TAP states ─────────────────────────────────────────────────── */

typedef enum {
    TAP_RESET        = 0,
    TAP_IDLE         = 1,
    TAP_DR_SELECT    = 2,
    TAP_DR_CAPTURE   = 3,
    TAP_DR_SHIFT     = 4,
    TAP_DR_EXIT1     = 5,
    TAP_DR_PAUSE     = 6,
    TAP_DR_EXIT2     = 7,
    TAP_DR_UPDATE    = 8,
    TAP_IR_SELECT    = 9,
    TAP_IR_CAPTURE   = 10,
    TAP_IR_SHIFT     = 11,
    TAP_IR_EXIT1     = 12,
    TAP_IR_PAUSE     = 13,
    TAP_IR_EXIT2     = 14,
    TAP_IR_UPDATE    = 15,
    TAP_STATES_COUNT = 16,
} jtag_tap_state_t;

/* ── JTAG private data ──────────────────────────────────────────────── */

typedef struct {
    up_pin_t pin_tck;
    up_pin_t pin_tms;
    up_pin_t pin_tdi;
    up_pin_t pin_tdo;
    up_pin_t pin_trst;
    uint32_t freq_hz;
    bool     use_pio;
} jtag_proto_config_t;

typedef struct {
    up_pin_t         tck, tms, tdi, tdo, trst;
    jtag_tap_state_t state;
    uint32_t         delay_us;  /**< Half-period delay */
    hal_pio_sm_t    *pio_sm;    /**< PIO state machine (NULL = bitbang) */
} jtag_priv_t;

static jtag_priv_t s_jtag_priv;

/* ── TAP state transition table (TMS value to go from state to state) ─ */
/* Simplified: provides TMS sequence to go from any state to Shift-DR/IR */

/* ── Low-level bitbang ───────────────────────────────────────────────── */

static inline void jtag_tck_pulse(jtag_priv_t *j)
{
    hal_gpio_write(j->tck, UP_GPIO_LEVEL_HIGH);
    if (j->delay_us) hal_timer_delay_us(j->delay_us);
    hal_gpio_write(j->tck, UP_GPIO_LEVEL_LOW);
    if (j->delay_us) hal_timer_delay_us(j->delay_us);
}

static inline void jtag_tms_write(jtag_priv_t *j, bool tms)
{
    hal_gpio_write(j->tms, tms ? UP_GPIO_LEVEL_HIGH : UP_GPIO_LEVEL_LOW);
}

static inline void jtag_tdi_write(jtag_priv_t *j, bool tdi)
{
    hal_gpio_write(j->tdi, tdi ? UP_GPIO_LEVEL_HIGH : UP_GPIO_LEVEL_LOW);
}

static inline bool jtag_tdo_read(jtag_priv_t *j)
{
    return hal_gpio_read(j->tdo) == UP_GPIO_LEVEL_HIGH;
}

static void jtag_goto_state(jtag_priv_t *j, jtag_tap_state_t target)
{
    /* Navigate via Test-Logic-Reset as universal reset */
    if (target == TAP_RESET) {
        jtag_tms_write(j, true);
        for (int i = 0; i < 5; i++) jtag_tck_pulse(j);
        j->state = TAP_RESET;
        return;
    }

    /* Go to IDLE from RESET */
    if (j->state == TAP_RESET && target != TAP_RESET) {
        jtag_tms_write(j, false);
        jtag_tck_pulse(j);
        j->state = TAP_IDLE;
        if (target == TAP_IDLE) return;
    }

    /* IDLE → Shift-DR */
    if (target == TAP_DR_SHIFT && j->state == TAP_IDLE) {
        jtag_tms_write(j, true);  jtag_tck_pulse(j); /* → Select-DR */
        jtag_tms_write(j, false); jtag_tck_pulse(j); /* → Capture-DR */
        jtag_tms_write(j, false); jtag_tck_pulse(j); /* → Shift-DR */
        j->state = TAP_DR_SHIFT;
        return;
    }

    /* IDLE → Shift-IR */
    if (target == TAP_IR_SHIFT && j->state == TAP_IDLE) {
        jtag_tms_write(j, true);  jtag_tck_pulse(j); /* → Select-DR */
        jtag_tms_write(j, true);  jtag_tck_pulse(j); /* → Select-IR */
        jtag_tms_write(j, false); jtag_tck_pulse(j); /* → Capture-IR */
        jtag_tms_write(j, false); jtag_tck_pulse(j); /* → Shift-IR */
        j->state = TAP_IR_SHIFT;
        return;
    }

    /* Shift-xR → IDLE (via Exit1 → Update → Idle) */
    if (target == TAP_IDLE &&
        (j->state == TAP_DR_SHIFT || j->state == TAP_IR_SHIFT)) {
        jtag_tms_write(j, true);  jtag_tck_pulse(j); /* → Exit1 */
        jtag_tms_write(j, true);  jtag_tck_pulse(j); /* → Update */
        jtag_tms_write(j, false); jtag_tck_pulse(j); /* → Idle */
        j->state = TAP_IDLE;
        return;
    }

    /* Fallback: reset then navigate */
    jtag_goto_state(j, TAP_RESET);
    if (target != TAP_RESET) {
        jtag_goto_state(j, TAP_IDLE);
        if (target != TAP_IDLE) {
            jtag_goto_state(j, target);
        }
    }
}

/**
 * @brief Shift data through DR or IR
 * @param j       JTAG instance
 * @param tdi     Input data (LSB first)
 * @param tdo     Output data (LSB first), can be NULL
 * @param bits    Number of bits to shift
 */
static void jtag_shift_data(jtag_priv_t *j,
                             const uint8_t *tdi, uint8_t *tdo, size_t bits)
{
    if (tdo) memset(tdo, 0, (bits + 7) / 8);

    for (size_t i = 0; i < bits; i++) {
        bool last_bit = (i == bits - 1);

        /* TMS=1 on last bit to exit Shift state */
        jtag_tms_write(j, last_bit);

        /* Drive TDI */
        bool tdi_bit = false;
        if (tdi) {
            tdi_bit = (tdi[i / 8] >> (i % 8)) & 1;
        }
        jtag_tdi_write(j, tdi_bit);

        /* Rising edge — sample TDO */
        hal_gpio_write(j->tck, UP_GPIO_LEVEL_HIGH);
        if (j->delay_us) hal_timer_delay_us(j->delay_us);

        if (tdo) {
            if (jtag_tdo_read(j)) {
                tdo[i / 8] |= (1 << (i % 8));
            }
        }

        hal_gpio_write(j->tck, UP_GPIO_LEVEL_LOW);
        if (j->delay_us) hal_timer_delay_us(j->delay_us);
    }

    /* After shifting, we're in Exit1 state */
    if (j->state == TAP_DR_SHIFT) j->state = TAP_DR_EXIT1;
    if (j->state == TAP_IR_SHIFT) j->state = TAP_IR_EXIT1;
}

/* ── Protocol ops ────────────────────────────────────────────────────── */

static up_status_t jtag_proto_init(up_protocol_t *proto, const void *config)
{
    const jtag_proto_config_t *cfg = (const jtag_proto_config_t *)config;
    jtag_priv_t *j = &s_jtag_priv;

    j->tck = cfg->pin_tck;
    j->tms = cfg->pin_tms;
    j->tdi = cfg->pin_tdi;
    j->tdo = cfg->pin_tdo;
    j->trst = cfg->pin_trst;
    j->pio_sm = NULL;

    /* Calculate delay for requested frequency */
    if (cfg->freq_hz > 0) {
        j->delay_us = 500000 / cfg->freq_hz; /* half-period */
        if (j->delay_us == 0) j->delay_us = 1;
    } else {
        j->delay_us = 5; /* ~100 kHz default */
    }

    /* Init GPIO pins */
    hal_gpio_init(j->tck, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_NONE);
    hal_gpio_init(j->tms, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_NONE);
    hal_gpio_init(j->tdi, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_NONE);
    hal_gpio_init(j->tdo, UP_GPIO_DIR_INPUT,  UP_GPIO_PULL_UP);

    hal_gpio_write(j->tck, UP_GPIO_LEVEL_LOW);
    hal_gpio_write(j->tms, UP_GPIO_LEVEL_HIGH);
    hal_gpio_write(j->tdi, UP_GPIO_LEVEL_LOW);

    if (j->trst != UP_PIN_NONE) {
        hal_gpio_init(j->trst, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_NONE);
        hal_gpio_write(j->trst, UP_GPIO_LEVEL_HIGH);
    }

    j->state = TAP_RESET;
    proto->priv = j;

    /* Go to reset state */
    jtag_goto_state(j, TAP_RESET);

    return UP_OK;
}

static up_status_t jtag_proto_deinit(up_protocol_t *proto)
{
    jtag_priv_t *j = (jtag_priv_t *)proto->priv;

    hal_gpio_deinit(j->tck);
    hal_gpio_deinit(j->tms);
    hal_gpio_deinit(j->tdi);
    hal_gpio_deinit(j->tdo);
    if (j->trst != UP_PIN_NONE) hal_gpio_deinit(j->trst);

    return UP_OK;
}

static up_status_t jtag_proto_reset(up_protocol_t *proto)
{
    jtag_priv_t *j = (jtag_priv_t *)proto->priv;

    /* Hardware reset via TRST if available */
    if (j->trst != UP_PIN_NONE) {
        hal_gpio_write(j->trst, UP_GPIO_LEVEL_LOW);
        hal_timer_delay_ms(10);
        hal_gpio_write(j->trst, UP_GPIO_LEVEL_HIGH);
        hal_timer_delay_ms(10);
    }

    jtag_goto_state(j, TAP_RESET);
    return UP_OK;
}

static up_status_t jtag_proto_transfer(up_protocol_t *proto,
                                        const uint8_t *tx, size_t tx_len,
                                        uint8_t *rx, size_t rx_len)
{
    jtag_priv_t *j = (jtag_priv_t *)proto->priv;

    /* Interpret as DR scan: tx_len = bits to shift in, rx_len ignored (same) */
    size_t bits = tx_len * 8;
    if (bits == 0) return UP_ERROR_INVALID_ARG;

    jtag_goto_state(j, TAP_DR_SHIFT);
    jtag_shift_data(j, tx, rx, bits);
    jtag_goto_state(j, TAP_IDLE);

    return UP_OK;
}

static up_status_t jtag_proto_detect(up_protocol_t *proto,
                                      uint32_t *ids, size_t max_ids, size_t *found)
{
    jtag_priv_t *j = (jtag_priv_t *)proto->priv;

    *found = 0;

    /* Read IDCODE: shift 32 zeros through DR after reset */
    jtag_goto_state(j, TAP_RESET);
    jtag_goto_state(j, TAP_DR_SHIFT);

    /* Shift out IDCODEs from the chain */
    for (size_t dev = 0; dev < max_ids; dev++) {
        uint8_t tdo[4] = {0};
        uint8_t zeros[4] = {0};

        jtag_shift_data(j, zeros, tdo, 32);

        uint32_t idcode = ((uint32_t)tdo[3] << 24) |
                          ((uint32_t)tdo[2] << 16) |
                          ((uint32_t)tdo[1] << 8)  |
                          (uint32_t)tdo[0];

        if (idcode == 0 || idcode == 0xFFFFFFFF) break;

        ids[dev] = idcode;
        (*found)++;

        /* Re-enter Shift-DR for next device */
        jtag_goto_state(j, TAP_IDLE);
        jtag_goto_state(j, TAP_DR_SHIFT);
    }

    jtag_goto_state(j, TAP_IDLE);

    return UP_OK;
}

static up_status_t jtag_proto_set_speed(up_protocol_t *proto, uint32_t speed_hz)
{
    jtag_priv_t *j = (jtag_priv_t *)proto->priv;
    if (speed_hz > 0) {
        j->delay_us = 500000 / speed_hz;
        if (j->delay_us == 0) j->delay_us = 1;
    }
    return UP_OK;
}

/* ── Public vtable ───────────────────────────────────────────────────── */

const up_protocol_ops_t jtag_protocol_ops = {
    .name      = "JTAG",
    .type      = UP_PROTO_JTAG,
    .init      = jtag_proto_init,
    .deinit    = jtag_proto_deinit,
    .reset     = jtag_proto_reset,
    .transfer  = jtag_proto_transfer,
    .detect    = jtag_proto_detect,
    .set_speed = jtag_proto_set_speed,
};
