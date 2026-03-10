/**
 * @file rp2040_hal_timer.c
 * @brief RP2040 Timer HAL implementation
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/hal/hal_timer.h"

#ifdef UNIPROGER_PLATFORM_RP2040

#include "pico/stdlib.h"
#include "hardware/timer.h"

/* ── Periodic timer state ────────────────────────────────────────────── */

static struct {
    repeating_timer_t rt;
    hal_timer_cb_t    cb;
    void             *user_data;
    bool              active;
} s_periodic;

static bool rp2040_periodic_cb(repeating_timer_t *rt)
{
    UP_UNUSED(rt);
    if (s_periodic.cb) {
        s_periodic.cb(s_periodic.user_data);
    }
    return true; /* Keep repeating */
}

/* ── Timer API ───────────────────────────────────────────────────────── */

void hal_timer_delay_us(uint32_t us)
{
    busy_wait_us_32(us);
}

void hal_timer_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

uint64_t hal_timer_get_us(void)
{
    return time_us_64();
}

uint32_t hal_timer_get_ms(void)
{
    return to_ms_since_boot(get_absolute_time());
}

up_status_t hal_timer_start_periodic(uint32_t period_us,
                                      hal_timer_cb_t cb, void *user_data)
{
    if (s_periodic.active) {
        cancel_repeating_timer(&s_periodic.rt);
    }

    s_periodic.cb = cb;
    s_periodic.user_data = user_data;

    /* Negative period = interval between callbacks */
    if (!add_repeating_timer_us(-(int64_t)period_us, rp2040_periodic_cb,
                                 NULL, &s_periodic.rt)) {
        return UP_ERROR;
    }

    s_periodic.active = true;
    return UP_OK;
}

up_status_t hal_timer_stop_periodic(void)
{
    if (s_periodic.active) {
        cancel_repeating_timer(&s_periodic.rt);
        s_periodic.active = false;
    }
    return UP_OK;
}

#endif /* UNIPROGER_PLATFORM_RP2040 */
