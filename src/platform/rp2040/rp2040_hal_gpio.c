/**
 * @file rp2040_hal_gpio.c
 * @brief RP2040 GPIO HAL implementation using pico-sdk
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/hal/hal_gpio.h"

#ifdef UNIPROGER_PLATFORM_RP2040

#include "pico/stdlib.h"
#include "hardware/gpio.h"

/* ── IRQ callback storage ────────────────────────────────────────────── */

#define RP2040_MAX_GPIO 30

typedef struct {
    up_gpio_irq_cb_t cb;
    void            *user_data;
} gpio_irq_entry_t;

static gpio_irq_entry_t s_irq_table[RP2040_MAX_GPIO];

static void rp2040_gpio_irq_handler(uint gpio, uint32_t events)
{
    if (gpio < RP2040_MAX_GPIO && s_irq_table[gpio].cb) {
        s_irq_table[gpio].cb((up_pin_t)gpio, events, s_irq_table[gpio].user_data);
    }
}

/* ── GPIO API ────────────────────────────────────────────────────────── */

up_status_t hal_gpio_init(up_pin_t pin, up_gpio_dir_t dir, up_gpio_pull_t pull)
{
    if (pin >= RP2040_MAX_GPIO) return UP_ERROR_INVALID_ARG;

    gpio_init(pin);
    gpio_set_dir(pin, dir == UP_GPIO_DIR_OUTPUT ? GPIO_OUT : GPIO_IN);

    if (pull == UP_GPIO_PULL_UP) {
        gpio_pull_up(pin);
    } else if (pull == UP_GPIO_PULL_DOWN) {
        gpio_pull_down(pin);
    } else {
        gpio_disable_pulls(pin);
    }

    return UP_OK;
}

up_status_t hal_gpio_deinit(up_pin_t pin)
{
    if (pin >= RP2040_MAX_GPIO) return UP_ERROR_INVALID_ARG;

    gpio_set_dir(pin, GPIO_IN);
    gpio_disable_pulls(pin);
    gpio_set_function(pin, GPIO_FUNC_NULL);

    /* Clear IRQ entry */
    s_irq_table[pin].cb = NULL;
    s_irq_table[pin].user_data = NULL;

    return UP_OK;
}

up_status_t hal_gpio_set_dir(up_pin_t pin, up_gpio_dir_t dir)
{
    if (pin >= RP2040_MAX_GPIO) return UP_ERROR_INVALID_ARG;
    gpio_set_dir(pin, dir == UP_GPIO_DIR_OUTPUT ? GPIO_OUT : GPIO_IN);
    return UP_OK;
}

void hal_gpio_write(up_pin_t pin, up_gpio_level_t level)
{
    gpio_put(pin, level == UP_GPIO_LEVEL_HIGH ? 1 : 0);
}

up_gpio_level_t hal_gpio_read(up_pin_t pin)
{
    return gpio_get(pin) ? UP_GPIO_LEVEL_HIGH : UP_GPIO_LEVEL_LOW;
}

void hal_gpio_toggle(up_pin_t pin)
{
    gpio_xor_mask(1u << pin);
}

up_status_t hal_gpio_irq_attach(up_pin_t pin, uint32_t events,
                                 up_gpio_irq_cb_t cb, void *user_data)
{
    if (pin >= RP2040_MAX_GPIO) return UP_ERROR_INVALID_ARG;

    s_irq_table[pin].cb = cb;
    s_irq_table[pin].user_data = user_data;

    gpio_set_irq_enabled_with_callback(pin, events, true, rp2040_gpio_irq_handler);

    return UP_OK;
}

up_status_t hal_gpio_irq_detach(up_pin_t pin)
{
    if (pin >= RP2040_MAX_GPIO) return UP_ERROR_INVALID_ARG;

    gpio_set_irq_enabled(pin, 0xFF, false);
    s_irq_table[pin].cb = NULL;
    s_irq_table[pin].user_data = NULL;

    return UP_OK;
}

#endif /* UNIPROGER_PLATFORM_RP2040 */
