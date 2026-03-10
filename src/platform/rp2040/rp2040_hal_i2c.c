/**
 * @file rp2040_hal_i2c.c
 * @brief RP2040 I2C HAL implementation
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/hal/hal_i2c.h"

#ifdef UNIPROGER_PLATFORM_RP2040

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

/* ── Instance storage ────────────────────────────────────────────────── */

struct hal_i2c_inst {
    i2c_inst_t *hw_i2c;
};

static struct hal_i2c_inst s_i2c_instances[2];

/* ── I2C API ─────────────────────────────────────────────────────────── */

up_status_t hal_i2c_init(hal_i2c_inst_t **inst, const hal_i2c_config_t *cfg)
{
    if (!inst || !cfg || cfg->instance > 1) return UP_ERROR_INVALID_ARG;

    i2c_inst_t *hw = cfg->instance == 0 ? i2c0 : i2c1;
    struct hal_i2c_inst *ii = &s_i2c_instances[cfg->instance];

    ii->hw_i2c = hw;

    i2c_init(hw, cfg->freq_hz);

    gpio_set_function(cfg->pin_sda, GPIO_FUNC_I2C);
    gpio_set_function(cfg->pin_scl, GPIO_FUNC_I2C);
    gpio_pull_up(cfg->pin_sda);
    gpio_pull_up(cfg->pin_scl);

    *inst = ii;
    return UP_OK;
}

up_status_t hal_i2c_deinit(hal_i2c_inst_t *inst)
{
    if (!inst) return UP_ERROR_INVALID_ARG;
    i2c_deinit(inst->hw_i2c);
    return UP_OK;
}

up_status_t hal_i2c_write(hal_i2c_inst_t *inst, uint8_t addr,
                           const uint8_t *data, size_t len,
                           bool nostop, uint32_t timeout_ms)
{
    if (!inst || !data) return UP_ERROR_INVALID_ARG;
    UP_UNUSED(timeout_ms); /* pico-sdk uses its own timeout */

    int ret = i2c_write_blocking(inst->hw_i2c, addr, data, len, nostop);
    return (ret == PICO_ERROR_GENERIC) ? UP_ERROR_IO : UP_OK;
}

up_status_t hal_i2c_read(hal_i2c_inst_t *inst, uint8_t addr,
                          uint8_t *data, size_t len,
                          bool nostop, uint32_t timeout_ms)
{
    if (!inst || !data) return UP_ERROR_INVALID_ARG;
    UP_UNUSED(timeout_ms);

    int ret = i2c_read_blocking(inst->hw_i2c, addr, data, len, nostop);
    return (ret == PICO_ERROR_GENERIC) ? UP_ERROR_IO : UP_OK;
}

up_status_t hal_i2c_write_read(hal_i2c_inst_t *inst, uint8_t addr,
                                const uint8_t *wr_data, size_t wr_len,
                                uint8_t *rd_data, size_t rd_len,
                                uint32_t timeout_ms)
{
    up_status_t st = hal_i2c_write(inst, addr, wr_data, wr_len, true, timeout_ms);
    if (st != UP_OK) return st;
    return hal_i2c_read(inst, addr, rd_data, rd_len, false, timeout_ms);
}

up_status_t hal_i2c_scan(hal_i2c_inst_t *inst,
                          uint8_t *addrs, size_t max_addrs, size_t *found)
{
    if (!inst || !addrs || !found) return UP_ERROR_INVALID_ARG;

    *found = 0;
    uint8_t dummy;

    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        int ret = i2c_read_blocking(inst->hw_i2c, addr, &dummy, 1, false);
        if (ret >= 0) {
            if (*found < max_addrs) {
                addrs[*found] = addr;
            }
            (*found)++;
        }
    }

    return UP_OK;
}

#endif /* UNIPROGER_PLATFORM_RP2040 */
