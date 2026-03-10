/**
 * @file commands.c
 * @brief CLI command handlers — detect, read, write, erase, verify, dump, info
 *
 * SPDX-License-Identifier: MIT
 */

#include "cli.h"
#include "src/core/device/device.h"
#include "src/core/protocol/protocol.h"
#include "src/core/buffer/buffer.h"
#include "src/core/utils/log.h"
#include "src/core/hal/hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "CMD"

/* ── External protocol/device ops declarations ───────────────────────── */

extern const up_protocol_ops_t spi_protocol_ops;
extern const up_protocol_ops_t i2c_protocol_ops;
extern const up_protocol_ops_t jtag_protocol_ops;
extern const up_protocol_ops_t swd_protocol_ops;
extern const up_protocol_ops_t uart_protocol_ops;
extern const up_protocol_ops_t onewire_protocol_ops;

extern const up_device_ops_t spi_flash_device_ops;
extern const up_device_ops_t i2c_eeprom_device_ops;
extern const up_device_ops_t avr_isp_device_ops;
extern const up_device_ops_t stm32_swd_device_ops;

/* ── Progress callback ───────────────────────────────────────────────── */

static void progress_cb(uint32_t current, uint32_t total, void *ud)
{
    (void)ud;
    if (total == 0) return;
    uint32_t pct = (current * 100) / total;
    printf("\r  Progress: %lu / %lu (%lu%%)",
           (unsigned long)current, (unsigned long)total, (unsigned long)pct);
    if (current >= total) printf(" Done!\n");
}

/* ── Command: detect ─────────────────────────────────────────────────── */

static up_status_t cmd_detect(int argc, char *argv[])
{
    (void)argc; (void)argv;
    cli_printf("\n  Scanning for devices...\n\n");

    size_t dev_count = up_device_count();
    cli_printf("  Registered drivers: %zu\n", dev_count);

    for (size_t i = 0; i < dev_count; i++) {
        const up_device_ops_t *ops = up_device_get(i);
        cli_printf("  [%zu] %-16s type=%d\n", i, ops->name, ops->type);
    }
    cli_printf("\n");

    return UP_OK;
}

/* ── Command: info ───────────────────────────────────────────────────── */

static up_status_t cmd_info(int argc, char *argv[])
{
    (void)argc; (void)argv;
    cli_printf("\n  System Information:\n");
    cli_printf("  Platform:  %s\n", hal_platform_name());
    cli_printf("  Clock:     %lu MHz\n", (unsigned long)(hal_platform_get_clock_hz() / 1000000));

    uint8_t chip_id[8];
    size_t id_len = hal_platform_get_chip_id(chip_id, sizeof(chip_id));
    cli_printf("  Chip ID:   ");
    for (size_t i = 0; i < id_len; i++) printf("%02X", chip_id[i]);
    cli_printf("\n\n");

    return UP_OK;
}

/* ── Command: scan_i2c ───────────────────────────────────────────────── */

static up_status_t cmd_scan_i2c(int argc, char *argv[])
{
    (void)argc; (void)argv;
    cli_printf("\n  I2C Bus Scan:\n");
    cli_printf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int row = 0; row < 8; row++) {
        cli_printf("  %X0", row);
        for (int col = 0; col < 16; col++) {
            int addr = row * 16 + col;
            if (addr < 0x08 || addr > 0x77) {
                cli_printf("   ");
            } else {
                cli_printf(" %02X", addr); /* Placeholder */
            }
        }
        cli_printf("\n");
    }
    cli_printf("\n");
    return UP_OK;
}

/* ── Command: pin ────────────────────────────────────────────────────── */

static up_status_t cmd_pin(int argc, char *argv[])
{
    if (argc < 3) {
        cli_printf("  Usage: pin <num> <high|low|read|in|out>\n");
        return UP_ERROR_INVALID_ARG;
    }

    uint8_t pin = (uint8_t)atoi(argv[1]);
    const char *action = argv[2];

    if (strcmp(action, "high") == 0) {
        hal_gpio_init(pin, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_NONE);
        hal_gpio_write(pin, UP_GPIO_LEVEL_HIGH);
        cli_printf("  Pin %d → HIGH\n", pin);
    } else if (strcmp(action, "low") == 0) {
        hal_gpio_init(pin, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_NONE);
        hal_gpio_write(pin, UP_GPIO_LEVEL_LOW);
        cli_printf("  Pin %d → LOW\n", pin);
    } else if (strcmp(action, "read") == 0) {
        hal_gpio_init(pin, UP_GPIO_DIR_INPUT, UP_GPIO_PULL_UP);
        up_gpio_level_t level = hal_gpio_read(pin);
        cli_printf("  Pin %d = %s\n", pin, level ? "HIGH" : "LOW");
    } else if (strcmp(action, "in") == 0) {
        hal_gpio_init(pin, UP_GPIO_DIR_INPUT, UP_GPIO_PULL_UP);
        cli_printf("  Pin %d → INPUT\n", pin);
    } else if (strcmp(action, "out") == 0) {
        hal_gpio_init(pin, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_NONE);
        cli_printf("  Pin %d → OUTPUT\n", pin);
    } else {
        cli_printf("  Unknown action: %s\n", action);
    }

    return UP_OK;
}

/* ── Command: reset ──────────────────────────────────────────────────── */

static up_status_t cmd_reset(int argc, char *argv[])
{
    (void)argc; (void)argv;
    cli_printf("  Resetting system...\n");
    hal_timer_delay_ms(100);
    hal_platform_reset();
    return UP_OK; /* Never reached */
}

/* ── Command registration ────────────────────────────────────────────── */

static const cli_command_t s_commands[] = {
    { "detect",   "Detect connected devices",         "detect",                    cmd_detect },
    { "info",     "Show system information",           "info",                      cmd_info },
    { "scan",     "Scan I2C bus for devices",          "scan",                      cmd_scan_i2c },
    { "pin",      "GPIO pin control",                  "pin <num> <high|low|read>", cmd_pin },
    { "reset",    "Reset system",                      "reset",                     cmd_reset },
};

void commands_register_all(void)
{
    for (size_t i = 0; i < sizeof(s_commands) / sizeof(s_commands[0]); i++) {
        cli_register(&s_commands[i]);
    }
}
