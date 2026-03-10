/**
 * @file main.c
 * @brief UniProger entry point — platform init, device registration, CLI
 *
 * SPDX-License-Identifier: MIT
 */

#include "uniproger/config.h"
#include "uniproger/version.h"
#include "src/core/hal/hal.h"
#include "src/core/device/device.h"
#include "src/cli/cli.h"
#include "src/core/utils/log.h"

#define TAG "MAIN"

/* ── External device driver ops ──────────────────────────────────────── */

extern const up_device_ops_t spi_flash_device_ops;
extern const up_device_ops_t i2c_eeprom_device_ops;
extern const up_device_ops_t avr_isp_device_ops;
extern const up_device_ops_t stm32_swd_device_ops;

/* ── External command registration ───────────────────────────────────── */

extern void commands_register_all(void);

/* ── Boot LED animation ──────────────────────────────────────────────── */

static void boot_animation(void)
{
    for (int i = 0; i < 6; i++) {
        hal_platform_led(i % 2 == 0);
        hal_timer_delay_ms(100);
    }
    hal_platform_led(false);
}

/* ── Register all built-in device drivers ────────────────────────────── */

static void register_devices(void)
{
    up_device_registry_init();

    up_device_register(&spi_flash_device_ops);
    up_device_register(&i2c_eeprom_device_ops);
    up_device_register(&avr_isp_device_ops);
    up_device_register(&stm32_swd_device_ops);

    UP_LOG_INFO(TAG, "Registered %zu device drivers", up_device_count());
}

/* ── Main ────────────────────────────────────────────────────────────── */

int main(void)
{
    /* 1. Platform hardware init */
    hal_platform_init();

    /* 2. Boot animation */
    boot_animation();

    /* 3. Log startup info */
    UP_LOG_INFO(TAG, "=== %s v%s ===",
                UNIPROGER_PROJECT_NAME, UNIPROGER_VERSION_STRING);
    UP_LOG_INFO(TAG, "Platform: %s @ %lu MHz",
                hal_platform_name(),
                (unsigned long)(hal_platform_get_clock_hz() / 1000000));

    uint8_t chip_id[8];
    size_t id_len = hal_platform_get_chip_id(chip_id, sizeof(chip_id));
    if (id_len > 0) {
        char id_str[20] = {0};
        for (size_t i = 0; i < id_len && i < 8; i++) {
            snprintf(id_str + i * 2, 3, "%02X", chip_id[i]);
        }
        UP_LOG_INFO(TAG, "Chip ID: %s", id_str);
    }

    /* 4. Register device drivers */
    register_devices();

    /* 5. Initialize CLI */
    cli_init();
    commands_register_all();

    /* 6. Run CLI main loop */
    UP_LOG_INFO(TAG, "Starting CLI...");
    cli_run();

    /* Never reached */
    return 0;
}
