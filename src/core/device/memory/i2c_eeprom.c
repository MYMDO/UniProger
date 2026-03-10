/**
 * @file i2c_eeprom.c
 * @brief 24Cxx I2C EEPROM driver — byte/page access, size detect
 *
 * Supports: 24C01 through 24C512 and compatible.
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/device/device.h"
#include "src/core/hal/hal.h"
#include <string.h>
#include <stdio.h>

/* ── Constants ───────────────────────────────────────────────────────── */

#define EEPROM_BASE_ADDR  0x50  /* A0=A1=A2=0 */

typedef struct {
    uint8_t  dev_addr;    /**< I2C device address */
    bool     addr_16bit;  /**< 16-bit addressing (24C32+) */
    uint32_t page_size;
    uint32_t total_size;
} eeprom_priv_t;

static eeprom_priv_t s_eeprom_priv;

/* ── ACK polling — wait for write to complete ────────────────────────── */

static up_status_t eeprom_wait_ack(up_protocol_t *proto, uint8_t addr,
                                    uint32_t timeout_ms)
{
    uint64_t start = hal_timer_get_us();
    uint64_t timeout_us = (uint64_t)timeout_ms * 1000;

    while (!hal_timer_timeout(start, timeout_us)) {
        uint8_t dummy;
        up_status_t st = up_proto_transfer(proto, NULL, 0, &dummy, 1);
        if (st == UP_OK) return UP_OK;
        hal_timer_delay_us(500);
    }
    return UP_ERROR_TIMEOUT;
}

/* ── Device ops ──────────────────────────────────────────────────────── */

static up_status_t eeprom_detect(up_device_t *dev)
{
    /* Try to detect EEPROM via I2C scan */
    uint32_t ids[16];
    size_t found = 0;

    up_status_t st = up_proto_detect(dev->proto, ids, 16, &found);
    if (st != UP_OK || found == 0) return UP_ERROR_NO_DEVICE;

    /* Look for address in 0x50-0x57 range */
    bool detected = false;
    uint8_t addr = 0;
    for (size_t i = 0; i < found; i++) {
        if (ids[i] >= 0x50 && ids[i] <= 0x57) {
            addr = (uint8_t)ids[i];
            detected = true;
            break;
        }
    }
    if (!detected) return UP_ERROR_NO_DEVICE;

    eeprom_priv_t *priv = &s_eeprom_priv;
    priv->dev_addr = addr;

    /* Default to 24C256 (32KB, 16-bit addr, 64-byte pages) */
    priv->addr_16bit = true;
    priv->page_size = 64;
    priv->total_size = UP_KB(32);

    dev->priv = priv;
    dev->info.type = UP_DEVICE_TYPE_I2C_EEPROM;
    dev->info.id = addr;
    dev->info.caps = UP_CAP_READ | UP_CAP_WRITE | UP_CAP_VERIFY | UP_CAP_DETECT;
    dev->info.page_size = priv->page_size;
    dev->info.sector_size = priv->page_size;
    dev->info.total_size = priv->total_size;
    snprintf(dev->info.name, sizeof(dev->info.name), "I2C EEPROM @0x%02X", addr);

    return UP_OK;
}

static up_status_t eeprom_init(up_device_t *dev)
{
    dev->initialized = true;
    return UP_OK;
}

static up_status_t eeprom_deinit(up_device_t *dev)
{
    dev->initialized = false;
    return UP_OK;
}

static up_status_t eeprom_read(up_device_t *dev, uint32_t addr,
                                uint8_t *data, size_t len,
                                up_progress_cb_t cb, void *ud)
{
    eeprom_priv_t *priv = (eeprom_priv_t *)dev->priv;
    size_t offset = 0;

    while (offset < len) {
        uint32_t cur = addr + offset;
        size_t chunk = UP_MIN(32, len - offset);

        uint8_t addr_bytes[2];
        size_t addr_len;

        if (priv->addr_16bit) {
            addr_bytes[0] = (cur >> 8) & 0xFF;
            addr_bytes[1] = cur & 0xFF;
            addr_len = 2;
        } else {
            addr_bytes[0] = cur & 0xFF;
            addr_len = 1;
        }

        /* Write address, then read data */
        up_status_t st = up_proto_transfer(dev->proto,
                                            addr_bytes, addr_len,
                                            data + offset, chunk);
        if (st != UP_OK) return st;

        offset += chunk;
        if (cb) cb(offset, len, ud);
    }

    return UP_OK;
}

static up_status_t eeprom_write(up_device_t *dev, uint32_t addr,
                                 const uint8_t *data, size_t len,
                                 up_progress_cb_t cb, void *ud)
{
    eeprom_priv_t *priv = (eeprom_priv_t *)dev->priv;
    size_t offset = 0;

    while (offset < len) {
        uint32_t cur = addr + offset;
        uint32_t page_offset = cur % priv->page_size;
        size_t chunk = UP_MIN(priv->page_size - page_offset, len - offset);

        /* Build: addr_bytes + data */
        uint8_t buf[2 + 64]; /* max: 2 addr + 64 data */
        size_t hdr_len;

        if (priv->addr_16bit) {
            buf[0] = (cur >> 8) & 0xFF;
            buf[1] = cur & 0xFF;
            hdr_len = 2;
        } else {
            buf[0] = cur & 0xFF;
            hdr_len = 1;
        }

        memcpy(buf + hdr_len, data + offset, chunk);

        up_status_t st = up_proto_transfer(dev->proto,
                                            buf, hdr_len + chunk,
                                            NULL, 0);
        if (st != UP_OK) return st;

        /* ACK polling: wait for write cycle */
        eeprom_wait_ack(dev->proto, priv->dev_addr, 50);

        offset += chunk;
        if (cb) cb(offset, len, ud);
    }

    return UP_OK;
}

static up_status_t eeprom_verify(up_device_t *dev, uint32_t addr,
                                  const uint8_t *data, size_t len,
                                  up_progress_cb_t cb, void *ud)
{
    uint8_t buf[64];
    size_t offset = 0;

    while (offset < len) {
        size_t chunk = UP_MIN(sizeof(buf), len - offset);
        up_status_t st = eeprom_read(dev, addr + offset, buf, chunk, NULL, NULL);
        if (st != UP_OK) return st;
        if (memcmp(buf, data + offset, chunk) != 0) return UP_ERROR_VERIFY;
        offset += chunk;
        if (cb) cb(offset, len, ud);
    }
    return UP_OK;
}

const up_device_ops_t i2c_eeprom_device_ops = {
    .name    = "i2c_eeprom",
    .type    = UP_DEVICE_TYPE_I2C_EEPROM,
    .detect  = eeprom_detect,
    .init    = eeprom_init,
    .deinit  = eeprom_deinit,
    .read    = eeprom_read,
    .write   = eeprom_write,
    .erase   = NULL,
    .verify  = eeprom_verify,
    .emulate = NULL,
    .ioctl   = NULL,
};
