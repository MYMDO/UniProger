/**
 * @file avr_isp.c
 * @brief AVR ISP programmer — signature read, flash/EEPROM/fuse access
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/device/device.h"
#include "src/core/hal/hal.h"
#include <string.h>
#include <stdio.h>

/* ── AVR ISP commands ──────────────────────────────────────────────── */

#define ISP_PROG_ENABLE    {0xAC, 0x53, 0x00, 0x00}
#define ISP_CHIP_ERASE     {0xAC, 0x80, 0x00, 0x00}
#define ISP_READ_SIG(a)    {0x30, 0x00, (a), 0x00}
#define ISP_READ_FUSE_LOW  {0x50, 0x00, 0x00, 0x00}
#define ISP_READ_FUSE_HIGH {0x58, 0x08, 0x00, 0x00}
#define ISP_READ_FUSE_EXT  {0x50, 0x08, 0x00, 0x00}

/* ── Known AVR signatures ──────────────────────────────────────────── */

typedef struct {
    uint32_t    sig;
    const char *name;
    uint32_t    flash_size;
    uint32_t    page_size;
    uint32_t    eeprom_size;
} avr_entry_t;

static const avr_entry_t known_avrs[] = {
    { 0x1E9502, "ATmega32",   UP_KB(32),  128, 1024 },
    { 0x1E9587, "ATmega32A",  UP_KB(32),  128, 1024 },
    { 0x1E950F, "ATmega328P", UP_KB(32),  128, 1024 },
    { 0x1E9514, "ATmega328",  UP_KB(32),  128, 1024 },
    { 0x1E9406, "ATmega168",  UP_KB(16),  128,  512 },
    { 0x1E9301, "ATmega8",    UP_KB(8),    64,  512 },
    { 0x1E9205, "ATmega48",   UP_KB(4),    64,  256 },
    { 0x1E9703, "ATmega1280", UP_KB(128), 256, 4096 },
    { 0x1E9801, "ATmega2560", UP_KB(256), 256, 4096 },
    { 0x1E9108, "ATtiny25",   UP_KB(2),    32,  128 },
    { 0x1E9206, "ATtiny45",   UP_KB(4),    64,  256 },
    { 0x1E930B, "ATtiny85",   UP_KB(8),    64,  512 },
    { 0, NULL, 0, 0, 0 },
};

/* ── Private data ────────────────────────────────────────────────────── */

typedef struct {
    up_pin_t rst_pin;
    uint32_t page_size;
    uint32_t flash_size;
} avr_priv_t;

static avr_priv_t s_avr_priv;

/* ── ISP 4-byte SPI transfer ────────────────────────────────────────── */

static up_status_t avr_isp_cmd(up_protocol_t *proto,
                                const uint8_t cmd[4], uint8_t resp[4])
{
    return up_proto_transfer(proto, cmd, 4, resp, 0);
    /* Note: SPI is full-duplex; response comes in parallel.
       For AVR ISP, response byte 3 = response data.
       We use the SPI protocol engine which handles CS. */
}

static up_status_t avr_isp_xfer(up_protocol_t *proto,
                                 uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3,
                                 uint8_t *result)
{
    uint8_t tx[4] = {b0, b1, b2, b3};
    uint8_t rx[4] = {0};

    /* Full duplex SPI — response comes during transfer */
    /* We need raw SPI access for this */
    hal_spi_inst_t *spi = (hal_spi_inst_t *)proto->hw_inst;
    up_status_t st = hal_spi_transfer(spi, tx, rx, 4);
    if (result) *result = rx[3];
    return st;
}

/* ── Device ops ──────────────────────────────────────────────────────── */

static up_status_t avr_detect(up_device_t *dev)
{
    avr_priv_t *priv = &s_avr_priv;
    dev->priv = priv;

    /* Enter programming mode: RST low, then send Programming Enable */
    hal_gpio_write(priv->rst_pin, UP_GPIO_LEVEL_LOW);
    hal_timer_delay_ms(30);

    uint8_t result = 0;
    avr_isp_xfer(dev->proto, 0xAC, 0x53, 0x00, 0x00, &result);

    if (result != 0x53) {
        /* Retry with pulse */
        hal_gpio_write(priv->rst_pin, UP_GPIO_LEVEL_HIGH);
        hal_timer_delay_ms(20);
        hal_gpio_write(priv->rst_pin, UP_GPIO_LEVEL_LOW);
        hal_timer_delay_ms(30);
        avr_isp_xfer(dev->proto, 0xAC, 0x53, 0x00, 0x00, &result);
        if (result != 0x53) {
            hal_gpio_write(priv->rst_pin, UP_GPIO_LEVEL_HIGH);
            return UP_ERROR_NO_DEVICE;
        }
    }

    /* Read signature bytes */
    uint8_t sig[3];
    avr_isp_xfer(dev->proto, 0x30, 0x00, 0x00, 0x00, &sig[0]);
    avr_isp_xfer(dev->proto, 0x30, 0x00, 0x01, 0x00, &sig[1]);
    avr_isp_xfer(dev->proto, 0x30, 0x00, 0x02, 0x00, &sig[2]);

    uint32_t signature = ((uint32_t)sig[0] << 16) |
                         ((uint32_t)sig[1] << 8)  | sig[2];

    if (signature == 0 || signature == 0xFFFFFF) {
        hal_gpio_write(priv->rst_pin, UP_GPIO_LEVEL_HIGH);
        return UP_ERROR_NO_DEVICE;
    }

    dev->info.id = signature;
    dev->info.type = UP_DEVICE_TYPE_MCU_AVR;
    dev->info.caps = UP_CAP_READ | UP_CAP_WRITE | UP_CAP_ERASE |
                     UP_CAP_VERIFY | UP_CAP_DETECT |
                     UP_CAP_FUSE_READ | UP_CAP_FUSE_WRITE;

    for (int i = 0; known_avrs[i].name; i++) {
        if (known_avrs[i].sig == signature) {
            strncpy(dev->info.name, known_avrs[i].name, sizeof(dev->info.name) - 1);
            dev->info.total_size = known_avrs[i].flash_size;
            dev->info.page_size = known_avrs[i].page_size;
            priv->page_size = known_avrs[i].page_size;
            priv->flash_size = known_avrs[i].flash_size;
            return UP_OK;
        }
    }

    snprintf(dev->info.name, sizeof(dev->info.name),
             "AVR %02X:%02X:%02X", sig[0], sig[1], sig[2]);
    dev->info.total_size = 0;
    dev->info.page_size = 64;
    priv->page_size = 64;
    priv->flash_size = 0;

    return UP_OK;
}

static up_status_t avr_init(up_device_t *dev) {
    dev->initialized = true;
    return UP_OK;
}

static up_status_t avr_deinit(up_device_t *dev) {
    avr_priv_t *priv = (avr_priv_t *)dev->priv;
    hal_gpio_write(priv->rst_pin, UP_GPIO_LEVEL_HIGH);
    dev->initialized = false;
    return UP_OK;
}

static up_status_t avr_read(up_device_t *dev, uint32_t addr,
                             uint8_t *data, size_t len,
                             up_progress_cb_t cb, void *ud)
{
    for (size_t i = 0; i < len; i++) {
        uint16_t word_addr = (addr + i) / 2;
        bool high = (addr + i) % 2;
        uint8_t cmd = high ? 0x28 : 0x20; /* Read flash high/low byte */
        avr_isp_xfer(dev->proto, cmd,
                     (word_addr >> 8) & 0xFF, word_addr & 0xFF, 0x00,
                     &data[i]);
        if (cb && (i % 256 == 0)) cb(i, len, ud);
    }
    if (cb) cb(len, len, ud);
    return UP_OK;
}

static up_status_t avr_write(up_device_t *dev, uint32_t addr,
                              const uint8_t *data, size_t len,
                              up_progress_cb_t cb, void *ud)
{
    avr_priv_t *priv = (avr_priv_t *)dev->priv;
    size_t offset = 0;

    while (offset < len) {
        /* Load page buffer */
        size_t page_bytes = UP_MIN(priv->page_size, len - offset);
        for (size_t i = 0; i < page_bytes; i++) {
            uint16_t word_offset = i / 2;
            bool high = i % 2;
            uint8_t cmd = high ? 0x48 : 0x40;
            avr_isp_xfer(dev->proto, cmd, 0x00,
                         word_offset & 0xFF, data[offset + i], NULL);
        }

        /* Write page */
        uint16_t page_addr = (addr + offset) / 2;
        avr_isp_xfer(dev->proto, 0x4C,
                     (page_addr >> 8) & 0xFF, page_addr & 0xFF, 0x00, NULL);
        hal_timer_delay_ms(5); /* Write cycle time */

        offset += page_bytes;
        if (cb) cb(offset, len, ud);
    }

    return UP_OK;
}

static up_status_t avr_erase(up_device_t *dev, uint32_t addr, size_t len,
                              up_progress_cb_t cb, void *ud)
{
    UP_UNUSED(addr); UP_UNUSED(len);
    avr_isp_xfer(dev->proto, 0xAC, 0x80, 0x00, 0x00, NULL);
    hal_timer_delay_ms(10);
    if (cb) cb(1, 1, ud);
    return UP_OK;
}

static up_status_t avr_verify(up_device_t *dev, uint32_t addr,
                               const uint8_t *data, size_t len,
                               up_progress_cb_t cb, void *ud)
{
    uint8_t byte;
    for (size_t i = 0; i < len; i++) {
        uint16_t wa = (addr + i) / 2;
        bool high = (addr + i) % 2;
        avr_isp_xfer(dev->proto, high ? 0x28 : 0x20,
                     (wa >> 8) & 0xFF, wa & 0xFF, 0x00, &byte);
        if (byte != data[i]) return UP_ERROR_VERIFY;
        if (cb && (i % 256 == 0)) cb(i, len, ud);
    }
    if (cb) cb(len, len, ud);
    return UP_OK;
}

const up_device_ops_t avr_isp_device_ops = {
    .name    = "avr_isp",
    .type    = UP_DEVICE_TYPE_MCU_AVR,
    .detect  = avr_detect,
    .init    = avr_init,
    .deinit  = avr_deinit,
    .read    = avr_read,
    .write   = avr_write,
    .erase   = avr_erase,
    .verify  = avr_verify,
    .emulate = NULL,
    .ioctl   = NULL,
};
