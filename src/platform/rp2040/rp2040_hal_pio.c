/**
 * @file rp2040_hal_pio.c
 * @brief RP2040 PIO HAL implementation for programmable I/O state machines
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/hal/hal_pio.h"

#ifdef UNIPROGER_PLATFORM_RP2040

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

/* ── Instance storage ────────────────────────────────────────────────── */

#define MAX_PIO_SM  8  /* 2 PIO blocks × 4 SM each */

struct hal_pio_sm {
    PIO      pio;
    uint     sm;
    uint     offset;
    bool     in_use;
};

static struct hal_pio_sm s_pio_sm_pool[MAX_PIO_SM];

static struct hal_pio_sm *alloc_sm(void)
{
    for (int i = 0; i < MAX_PIO_SM; i++) {
        if (!s_pio_sm_pool[i].in_use) {
            s_pio_sm_pool[i].in_use = true;
            return &s_pio_sm_pool[i];
        }
    }
    return NULL;
}

/* ── PIO API ─────────────────────────────────────────────────────────── */

bool hal_pio_available(void)
{
    return true; /* RP2040 always has PIO */
}

up_status_t hal_pio_init(hal_pio_sm_t **sm, const hal_pio_config_t *cfg)
{
    if (!sm || !cfg || !cfg->program_data) return UP_ERROR_INVALID_ARG;

    struct hal_pio_sm *s = alloc_sm();
    if (!s) return UP_ERROR_NO_MEMORY;

    const pio_program_t *prog = (const pio_program_t *)cfg->program_data;

    /* Try PIO0 first, then PIO1 */
    PIO pio_hw = pio0;
    if (!pio_can_add_program(pio_hw, prog)) {
        pio_hw = pio1;
        if (!pio_can_add_program(pio_hw, prog)) {
            s->in_use = false;
            return UP_ERROR_NO_MEMORY;
        }
    }

    s->pio = pio_hw;
    s->sm = pio_claim_unused_sm(pio_hw, false);
    if ((int)s->sm < 0) {
        s->in_use = false;
        return UP_ERROR_NO_MEMORY;
    }

    s->offset = pio_add_program(pio_hw, prog);

    /* Default SM config */
    pio_sm_config c = pio_get_default_sm_config();

    /* pio_add_program already sets the default wrap in the PIO instruction memory,
       and specific wrap target/wrap behavior depends on how the PIO was compiled.
       We skip manually setting wrap here as the generated headers (jtag.pio.h, swd.pio.h)
       provide their own init functions containing the correct wrap setup via `sm_config_set_wrap`.
       But for generic fallback, we use standard defaults. */

    if (cfg->pin_count > 0) {
        sm_config_set_out_pins(&c, cfg->pin_base, cfg->pin_count);
        sm_config_set_set_pins(&c, cfg->pin_base, cfg->pin_count);
        for (uint i = 0; i < cfg->pin_count; i++) {
            pio_gpio_init(pio_hw, cfg->pin_base + i);
        }
    }

    if (cfg->pin_in_base != UP_PIN_NONE) {
        sm_config_set_in_pins(&c, cfg->pin_in_base);
    }

    if (cfg->pin_sideset_base != UP_PIN_NONE && cfg->sideset_count > 0) {
        sm_config_set_sideset_pins(&c, cfg->pin_sideset_base);
        sm_config_set_sideset(&c, cfg->sideset_count, false, false);
    }

    if (cfg->clock_div > 0) {
        sm_config_set_clkdiv_int_frac(&c, cfg->clock_div >> 8, cfg->clock_div & 0xFF);
    }

    sm_config_set_out_shift(&c, false, true, 32);
    sm_config_set_in_shift(&c, false, true, 32);

    pio_sm_init(pio_hw, s->sm, s->offset, &c);

    *sm = s;
    return UP_OK;
}

up_status_t hal_pio_deinit(hal_pio_sm_t *sm)
{
    if (!sm || !sm->in_use) return UP_ERROR_INVALID_ARG;

    pio_sm_set_enabled(sm->pio, sm->sm, false);
    pio_sm_unclaim(sm->pio, sm->sm);
    sm->in_use = false;

    return UP_OK;
}

up_status_t hal_pio_start(hal_pio_sm_t *sm)
{
    if (!sm) return UP_ERROR_INVALID_ARG;
    pio_sm_set_enabled(sm->pio, sm->sm, true);
    return UP_OK;
}

up_status_t hal_pio_stop(hal_pio_sm_t *sm)
{
    if (!sm) return UP_ERROR_INVALID_ARG;
    pio_sm_set_enabled(sm->pio, sm->sm, false);
    return UP_OK;
}

up_status_t hal_pio_put(hal_pio_sm_t *sm, uint32_t data)
{
    if (!sm) return UP_ERROR_INVALID_ARG;
    pio_sm_put_blocking(sm->pio, sm->sm, data);
    return UP_OK;
}

up_status_t hal_pio_get(hal_pio_sm_t *sm, uint32_t *data)
{
    if (!sm || !data) return UP_ERROR_INVALID_ARG;
    *data = pio_sm_get_blocking(sm->pio, sm->sm);
    return UP_OK;
}

up_status_t hal_pio_try_put(hal_pio_sm_t *sm, uint32_t data)
{
    if (!sm) return UP_ERROR_INVALID_ARG;
    if (pio_sm_is_tx_fifo_full(sm->pio, sm->sm)) return UP_ERROR_BUSY;
    pio_sm_put(sm->pio, sm->sm, data);
    return UP_OK;
}

up_status_t hal_pio_try_get(hal_pio_sm_t *sm, uint32_t *data)
{
    if (!sm || !data) return UP_ERROR_INVALID_ARG;
    if (pio_sm_is_rx_fifo_empty(sm->pio, sm->sm)) return UP_ERROR_BUSY;
    *data = pio_sm_get(sm->pio, sm->sm);
    return UP_OK;
}

up_status_t hal_pio_set_clkdiv(hal_pio_sm_t *sm, uint32_t clkdiv)
{
    if (!sm) return UP_ERROR_INVALID_ARG;
    pio_sm_set_clkdiv_int_frac(sm->pio, sm->sm, clkdiv >> 8, clkdiv & 0xFF);
    return UP_OK;
}

up_status_t hal_pio_exec(hal_pio_sm_t *sm, uint16_t instruction)
{
    if (!sm) return UP_ERROR_INVALID_ARG;
    pio_sm_exec(sm->pio, sm->sm, instruction);
    return UP_OK;
}

#endif /* UNIPROGER_PLATFORM_RP2040 */
