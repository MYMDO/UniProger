/**
 * @file hal_gpio.h
 * @brief HAL GPIO interface — platform-independent
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_HAL_GPIO_H
#define UNIPROGER_HAL_GPIO_H

#include "uniproger/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a GPIO pin
 * @param pin    Pin number
 * @param dir    Direction (input/output)
 * @param pull   Pull-up/down configuration
 * @return UP_OK on success
 */
up_status_t hal_gpio_init(up_pin_t pin, up_gpio_dir_t dir, up_gpio_pull_t pull);

/**
 * @brief Deinitialize a GPIO pin (return to default/hi-z state)
 */
up_status_t hal_gpio_deinit(up_pin_t pin);

/**
 * @brief Set pin direction
 */
up_status_t hal_gpio_set_dir(up_pin_t pin, up_gpio_dir_t dir);

/**
 * @brief Write a digital level to output pin
 */
void hal_gpio_write(up_pin_t pin, up_gpio_level_t level);

/**
 * @brief Read the digital level of a pin
 */
up_gpio_level_t hal_gpio_read(up_pin_t pin);

/**
 * @brief Toggle an output pin
 */
void hal_gpio_toggle(up_pin_t pin);

/**
 * @brief Attach interrupt callback to a pin
 * @param pin      Pin number
 * @param events   Bitmask of up_gpio_irq_t
 * @param cb       Callback function
 * @param user_data Opaque user data passed to callback
 */
up_status_t hal_gpio_irq_attach(up_pin_t pin, uint32_t events,
                                 up_gpio_irq_cb_t cb, void *user_data);

/**
 * @brief Detach interrupt from a pin
 */
up_status_t hal_gpio_irq_detach(up_pin_t pin);

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_HAL_GPIO_H */
