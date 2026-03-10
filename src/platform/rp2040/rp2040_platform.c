/**
 * @file rp2040_platform.c
 * @brief RP2040 platform initialization (clocks, USB CDC, LED, watchdog)
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/hal/hal.h"
#include "rp2040_pins.h"

#ifdef UNIPROGER_PLATFORM_RP2040

#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include "tusb.h"

/* ── Platform init ───────────────────────────────────────────────────── */

up_status_t hal_platform_init(void)
{
    /* Initialize standard I/O (USB CDC) */
    stdio_init_all();

    /* System LED */
    gpio_init(RP2040_PIN_LED);
    gpio_set_dir(RP2040_PIN_LED, GPIO_OUT);
    gpio_put(RP2040_PIN_LED, 0);

    return UP_OK;
}

up_status_t hal_platform_deinit(void)
{
    gpio_put(RP2040_PIN_LED, 0);
    return UP_OK;
}

const char *hal_platform_name(void)
{
    return "RP2040";
}

size_t hal_platform_get_chip_id(uint8_t *id, size_t id_len)
{
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);

    size_t copy_len = UP_MIN(id_len, sizeof(board_id.id));
    for (size_t i = 0; i < copy_len; i++) {
        id[i] = board_id.id[i];
    }
    return copy_len;
}

uint32_t hal_platform_get_clock_hz(void)
{
    return clock_get_hz(clk_sys);
}

void hal_platform_reset(void)
{
    watchdog_reboot(0, 0, 0);
    while (1) { __wfi(); }
}

void hal_platform_led(bool on)
{
    gpio_put(RP2040_PIN_LED, on ? 1 : 0);
}

#endif /* UNIPROGER_PLATFORM_RP2040 */
