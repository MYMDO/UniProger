/**
 * @file stm32_swd.c
 * @brief STM32 SWD programmer — CoreSight init, flash program
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/device/device.h"
#include "src/core/hal/hal.h"
#include <string.h>
#include <stdio.h>

/* ── ARM CoreSight / STM32 constants ─────────────────────────────────── */

#define DP_CTRL_STAT 0x04

#define DHCSR       0xE000EDF0
#define DEMCR       0xE000EDFC
#define AIRCR       0xE000ED0C
#define CPUID       0xE000ED00

/* STM32 Flash registers (STM32F1xx) */
#define FLASH_ACR   0x40022000
#define FLASH_KEYR  0x40022004
#define FLASH_SR    0x4002200C
#define FLASH_CR    0x40022010
#define FLASH_AR    0x40022014

#define FLASH_KEY1  0x45670123
#define FLASH_KEY2  0xCDEF89AB

#define FLASH_SR_BSY  (1 << 0)
#define FLASH_CR_PG   (1 << 0)
#define FLASH_CR_PER  (1 << 1)
#define FLASH_CR_STRT (1 << 6)
#define FLASH_CR_LOCK (1 << 7)

/* ── SWD mem access helpers (via protocol transfer encoding) ─────────── */

typedef struct {
    uint32_t cpuid;
    uint32_t flash_base;
    uint32_t flash_size;
    uint32_t page_size;
} stm32_priv_t;

static stm32_priv_t s_stm32_priv;

/* SWD DP/AP read/write wrappers using protocol transfer encoding */
static up_status_t swd_dp_read(up_protocol_t *proto, uint8_t addr, uint32_t *val)
{
    uint8_t cmd = addr & 0x0F; /* DP: bit 4 = 0 */
    uint8_t rx[4];
    up_status_t st = up_proto_transfer(proto, &cmd, 1, rx, 4);
    if (st != UP_OK) return st;
    *val = (uint32_t)rx[0] | ((uint32_t)rx[1] << 8) |
           ((uint32_t)rx[2] << 16) | ((uint32_t)rx[3] << 24);
    return UP_OK;
}

static up_status_t swd_dp_write(up_protocol_t *proto, uint8_t addr, uint32_t val)
{
    uint8_t cmd[5] = {
        addr & 0x0F,
        val & 0xFF, (val >> 8) & 0xFF,
        (val >> 16) & 0xFF, (val >> 24) & 0xFF
    };
    return up_proto_transfer(proto, cmd, 5, NULL, 0);
}

static up_status_t swd_ap_read(up_protocol_t *proto, uint8_t addr, uint32_t *val)
{
    uint8_t cmd = (addr & 0x0F) | 0x10; /* AP: bit 4 = 1 */
    uint8_t rx[4];
    up_status_t st = up_proto_transfer(proto, &cmd, 1, rx, 4);
    if (st != UP_OK) return st;
    *val = (uint32_t)rx[0] | ((uint32_t)rx[1] << 8) |
           ((uint32_t)rx[2] << 16) | ((uint32_t)rx[3] << 24);
    return UP_OK;
}

static up_status_t swd_ap_write(up_protocol_t *proto, uint8_t addr, uint32_t val)
{
    uint8_t cmd[5] = {
        (addr & 0x0F) | 0x10,
        val & 0xFF, (val >> 8) & 0xFF,
        (val >> 16) & 0xFF, (val >> 24) & 0xFF
    };
    return up_proto_transfer(proto, cmd, 5, NULL, 0);
}

/* AHB-AP memory access */
static up_status_t mem_write32(up_protocol_t *proto, uint32_t addr, uint32_t val)
{
    /* TAR = addr */
    swd_ap_write(proto, 0x04, addr);
    /* DRW = val */
    return swd_ap_write(proto, 0x0C, val);
}

static up_status_t mem_read32(up_protocol_t *proto, uint32_t addr, uint32_t *val)
{
    swd_ap_write(proto, 0x04, addr);
    return swd_ap_read(proto, 0x0C, val);
}

/* ── Device ops ──────────────────────────────────────────────────────── */

static up_status_t stm32_detect(up_device_t *dev)
{
    stm32_priv_t *priv = &s_stm32_priv;
    dev->priv = priv;

    /* Read DPIDR */
    uint32_t ids[1];
    size_t found = 0;
    up_status_t st = up_proto_detect(dev->proto, ids, 1, &found);

    /* Power up debug */
    swd_dp_write(dev->proto, DP_CTRL_STAT, 0x50000000);
    hal_timer_delay_ms(10);

    /* Init AHB-AP: CSW = 32-bit, auto-increment */
    swd_ap_write(dev->proto, 0x00, 0x23000052);
    hal_timer_delay_ms(1);

    /* Read CPUID */
    st = mem_read32(dev->proto, CPUID, &priv->cpuid);
    if (st != UP_OK) return st;

    priv->flash_base = 0x08000000;
    priv->flash_size = UP_KB(128); /* Default */
    priv->page_size = 1024;

    dev->info.id = priv->cpuid;
    dev->info.type = UP_DEVICE_TYPE_MCU_STM32;
    dev->info.caps = UP_CAP_READ | UP_CAP_WRITE | UP_CAP_ERASE |
                     UP_CAP_VERIFY | UP_CAP_DETECT;
    dev->info.total_size = priv->flash_size;
    dev->info.page_size = priv->page_size;
    dev->info.sector_size = priv->page_size;
    snprintf(dev->info.name, sizeof(dev->info.name),
             "STM32 CPUID:%08lX", (unsigned long)priv->cpuid);

    return UP_OK;
}

static up_status_t stm32_init(up_device_t *dev)
{
    /* Halt CPU */
    mem_write32(dev->proto, DHCSR, 0xA05F0003);
    hal_timer_delay_ms(10);
    dev->initialized = true;
    return UP_OK;
}

static up_status_t stm32_deinit(up_device_t *dev)
{
    /* Resume CPU */
    mem_write32(dev->proto, DHCSR, 0xA05F0000);
    dev->initialized = false;
    return UP_OK;
}

static up_status_t stm32_read(up_device_t *dev, uint32_t addr,
                               uint8_t *data, size_t len,
                               up_progress_cb_t cb, void *ud)
{
    size_t offset = 0;
    while (offset < len) {
        uint32_t val;
        up_status_t st = mem_read32(dev->proto, addr + offset, &val);
        if (st != UP_OK) return st;

        size_t remaining = len - offset;
        size_t chunk = UP_MIN(4, remaining);
        for (size_t i = 0; i < chunk; i++) {
            data[offset + i] = (val >> (i * 8)) & 0xFF;
        }
        offset += 4;
        if (cb && (offset % 256 == 0)) cb(offset, len, ud);
    }
    if (cb) cb(len, len, ud);
    return UP_OK;
}

static up_status_t flash_unlock(up_protocol_t *proto)
{
    mem_write32(proto, FLASH_KEYR, FLASH_KEY1);
    mem_write32(proto, FLASH_KEYR, FLASH_KEY2);
    return UP_OK;
}

static up_status_t flash_wait(up_protocol_t *proto, uint32_t timeout_ms)
{
    uint64_t start = hal_timer_get_us();
    while (!hal_timer_timeout(start, (uint64_t)timeout_ms * 1000)) {
        uint32_t sr;
        mem_read32(proto, FLASH_SR, &sr);
        if (!(sr & FLASH_SR_BSY)) return UP_OK;
        hal_timer_delay_us(100);
    }
    return UP_ERROR_TIMEOUT;
}

static up_status_t stm32_write(up_device_t *dev, uint32_t addr,
                                const uint8_t *data, size_t len,
                                up_progress_cb_t cb, void *ud)
{
    flash_unlock(dev->proto);

    /* Program halfword at a time */
    for (size_t i = 0; i < len; i += 2) {
        mem_write32(dev->proto, FLASH_CR, FLASH_CR_PG);
        uint16_t hw = data[i];
        if (i + 1 < len) hw |= ((uint16_t)data[i + 1] << 8);

        /* Write halfword via AHB */
        swd_ap_write(dev->proto, 0x00, 0x23000051); /* 16-bit */
        swd_ap_write(dev->proto, 0x04, addr + i);
        swd_ap_write(dev->proto, 0x0C, hw);

        flash_wait(dev->proto, 100);
        swd_ap_write(dev->proto, 0x00, 0x23000052); /* Back to 32-bit */

        if (cb && (i % 256 == 0)) cb(i, len, ud);
    }

    mem_write32(dev->proto, FLASH_CR, 0);
    if (cb) cb(len, len, ud);
    return UP_OK;
}

static up_status_t stm32_erase(up_device_t *dev, uint32_t addr, size_t len,
                                up_progress_cb_t cb, void *ud)
{
    stm32_priv_t *priv = (stm32_priv_t *)dev->priv;
    flash_unlock(dev->proto);

    size_t offset = 0;
    while (offset < len) {
        mem_write32(dev->proto, FLASH_CR, FLASH_CR_PER);
        mem_write32(dev->proto, FLASH_AR, addr + offset);
        mem_write32(dev->proto, FLASH_CR, FLASH_CR_PER | FLASH_CR_STRT);
        flash_wait(dev->proto, 1000);

        offset += priv->page_size;
        if (cb) cb(UP_MIN(offset, len), len, ud);
    }

    mem_write32(dev->proto, FLASH_CR, 0);
    return UP_OK;
}

static up_status_t stm32_verify(up_device_t *dev, uint32_t addr,
                                 const uint8_t *data, size_t len,
                                 up_progress_cb_t cb, void *ud)
{
    uint8_t buf[256];
    size_t off = 0;
    while (off < len) {
        size_t chunk = UP_MIN(sizeof(buf), len - off);
        up_status_t st = stm32_read(dev, addr + off, buf, chunk, NULL, NULL);
        if (st != UP_OK) return st;
        if (memcmp(buf, data + off, chunk) != 0) return UP_ERROR_VERIFY;
        off += chunk;
        if (cb) cb(off, len, ud);
    }
    return UP_OK;
}

const up_device_ops_t stm32_swd_device_ops = {
    .name    = "stm32_swd",
    .type    = UP_DEVICE_TYPE_MCU_STM32,
    .detect  = stm32_detect,
    .init    = stm32_init,
    .deinit  = stm32_deinit,
    .read    = stm32_read,
    .write   = stm32_write,
    .erase   = stm32_erase,
    .verify  = stm32_verify,
    .emulate = NULL,
    .ioctl   = NULL,
};
