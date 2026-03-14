/**
 * @file types.h
 * @brief UniProger common types and definitions
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 UniProger Contributors
 */

#ifndef UNIPROGER_TYPES_H
#define UNIPROGER_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Status codes ────────────────────────────────────────────────────── */

typedef enum {
    UP_OK                = 0,
    UP_ERROR             = -1,
    UP_ERROR_TIMEOUT     = -2,
    UP_ERROR_BUSY        = -3,
    UP_ERROR_NO_MEMORY   = -4,
    UP_ERROR_INVALID_ARG = -5,
    UP_ERROR_NOT_FOUND   = -6,
    UP_ERROR_NOT_SUPPORTED = -7,
    UP_ERROR_IO          = -8,
    UP_ERROR_CRC         = -9,
    UP_ERROR_VERIFY      = -10,
    UP_ERROR_PROTECTED   = -11,
    UP_ERROR_NO_DEVICE   = -12,
    UP_ERROR_OVERFLOW    = -13,
} up_status_t;

/* ── Pin definitions ─────────────────────────────────────────────────── */

typedef uint8_t up_pin_t;

#define UP_PIN_NONE  ((up_pin_t)0xFF)

typedef enum {
    UP_GPIO_DIR_INPUT  = 0,
    UP_GPIO_DIR_OUTPUT = 1,
} up_gpio_dir_t;

typedef enum {
    UP_GPIO_PULL_NONE = 0,
    UP_GPIO_PULL_UP   = 1,
    UP_GPIO_PULL_DOWN = 2,
} up_gpio_pull_t;

typedef enum {
    UP_GPIO_LEVEL_LOW  = 0,
    UP_GPIO_LEVEL_HIGH = 1,
} up_gpio_level_t;

typedef enum {
    UP_GPIO_IRQ_EDGE_RISE    = (1 << 0),
    UP_GPIO_IRQ_EDGE_FALL    = (1 << 1),
    UP_GPIO_IRQ_LEVEL_HIGH   = (1 << 2),
    UP_GPIO_IRQ_LEVEL_LOW    = (1 << 3),
} up_gpio_irq_t;

/* ── SPI definitions ─────────────────────────────────────────────────── */

typedef enum {
    UP_SPI_MODE_0 = 0,  /* CPOL=0, CPHA=0 */
    UP_SPI_MODE_1 = 1,  /* CPOL=0, CPHA=1 */
    UP_SPI_MODE_2 = 2,  /* CPOL=1, CPHA=0 */
    UP_SPI_MODE_3 = 3,  /* CPOL=1, CPHA=1 */
} up_spi_mode_t;

typedef enum {
    UP_SPI_BIT_ORDER_MSB_FIRST = 0,
    UP_SPI_BIT_ORDER_LSB_FIRST = 1,
} up_spi_bit_order_t;

/* ── I2C definitions ─────────────────────────────────────────────────── */

typedef enum {
    UP_I2C_SPEED_STANDARD  = 100000,   /* 100 kHz */
    UP_I2C_SPEED_FAST      = 400000,   /* 400 kHz */
    UP_I2C_SPEED_FAST_PLUS = 1000000,  /* 1 MHz   */
} up_i2c_speed_t;

typedef enum {
    UP_I2C_ADDR_7BIT  = 0,
    UP_I2C_ADDR_10BIT = 1,
} up_i2c_addr_mode_t;

/* ── UART definitions ────────────────────────────────────────────────── */

typedef enum {
    UP_UART_PARITY_NONE = 0,
    UP_UART_PARITY_EVEN = 1,
    UP_UART_PARITY_ODD  = 2,
} up_uart_parity_t;

typedef enum {
    UP_UART_STOP_BITS_1 = 1,
    UP_UART_STOP_BITS_2 = 2,
} up_uart_stop_bits_t;

typedef enum {
    UP_UART_DATA_BITS_7 = 7,
    UP_UART_DATA_BITS_8 = 8,
    UP_UART_DATA_BITS_9 = 9,
} up_uart_data_bits_t;

/* ── Device capability flags ─────────────────────────────────────────── */

typedef uint32_t up_device_caps_t;

#define UP_CAP_READ       (1U << 0)
#define UP_CAP_WRITE      (1U << 1)
#define UP_CAP_ERASE      (1U << 2)
#define UP_CAP_VERIFY     (1U << 3)
#define UP_CAP_DETECT     (1U << 4)
#define UP_CAP_EMULATE    (1U << 5)
#define UP_CAP_CLONE      (1U << 6)
#define UP_CAP_LOCK       (1U << 7)
#define UP_CAP_UNLOCK     (1U << 8)
#define UP_CAP_FUSE_READ  (1U << 9)
#define UP_CAP_FUSE_WRITE (1U << 10)
#define UP_CAP_ANALYZE    (1U << 11)

/* ── Device types ────────────────────────────────────────────────────── */

typedef enum {
    UP_DEVICE_TYPE_UNKNOWN       = 0,
    UP_DEVICE_TYPE_SPI_FLASH     = 1,
    UP_DEVICE_TYPE_I2C_EEPROM    = 2,
    UP_DEVICE_TYPE_PARALLEL_FLASH = 3,
    UP_DEVICE_TYPE_MCU_AVR       = 4,
    UP_DEVICE_TYPE_MCU_STM32     = 5,
    UP_DEVICE_TYPE_MCU_PIC       = 6,
    UP_DEVICE_TYPE_MCU_ESP       = 7,
    UP_DEVICE_TYPE_GENERIC       = 0xFF,
} up_device_type_t;

/* ── Progress callback ───────────────────────────────────────────────── */

typedef void (*up_progress_cb_t)(uint32_t current, uint32_t total, void *user_data);

/* ── Memory region descriptor ────────────────────────────────────────── */

typedef struct {
    uint32_t start;
    uint32_t size;
    uint32_t page_size;
    uint32_t sector_size;
    const char *name;
} up_mem_region_t;

/* ── GPIO callback ───────────────────────────────────────────────────── */

typedef void (*up_gpio_irq_cb_t)(up_pin_t pin, uint32_t events, void *user_data);

/* ── Utility macros ──────────────────────────────────────────────────── */

#define UP_MIN(a, b)           (((a) < (b)) ? (a) : (b))
#define UP_MAX(a, b)           (((a) > (b)) ? (a) : (b))
#define UP_ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))
#define UP_ALIGN_UP(x, align)  (((x) + (align) - 1) & ~((align) - 1))
#define UP_BIT(n)              (1U << (n))
#define UP_UNUSED(x)           ((void)(x))

#define UP_KB(n)  ((uint32_t)(n) * 1024U)
#define UP_MB(n)  ((uint32_t)(n) * 1024U * 1024U)

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_TYPES_H */
