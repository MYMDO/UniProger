/**
 * @file spi_flash.c
 * @brief W25Qxx SPI Flash driver — JEDEC detect, page program, sector erase
 *
 * Supports: W25Q16, W25Q32, W25Q64, W25Q128, and compatible chips.
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/device/device.h"
#include "src/core/hal/hal.h"
#include <string.h>
#include <stdio.h>

/* ── W25Qxx command set ──────────────────────────────────────────────── */

#define CMD_WRITE_ENABLE     0x06
#define CMD_WRITE_DISABLE    0x04
#define CMD_READ_STATUS1     0x05
#define CMD_READ_STATUS2     0x35
#define CMD_WRITE_STATUS     0x01
#define CMD_READ_DATA        0x03
#define CMD_FAST_READ        0x0B
#define CMD_PAGE_PROGRAM     0x02
#define CMD_SECTOR_ERASE     0x20  /* 4KB */
#define CMD_BLOCK_ERASE_32K  0x52
#define CMD_BLOCK_ERASE_64K  0xD8
#define CMD_CHIP_ERASE       0xC7
#define CMD_READ_JEDEC_ID    0x9F
#define CMD_READ_UNIQUE_ID   0x4B
#define CMD_POWER_DOWN       0xB9
#define CMD_RELEASE_PD       0xAB

#define SR1_BUSY             0x01
#define SR1_WEL              0x02

/* ── Known device table ──────────────────────────────────────────────── */

typedef struct {
    uint32_t jedec_id;
    const char *name;
    uint32_t size;
} spi_flash_entry_t;

static const spi_flash_entry_t known_chips[] = {
    { 0xEF4015, "W25Q16",   UP_KB(2048) },
    { 0xEF4016, "W25Q32",   UP_MB(4)    },
    { 0xEF4017, "W25Q64",   UP_MB(8)    },
    { 0xEF4018, "W25Q128",  UP_MB(16)   },
    { 0xEF4019, "W25Q256",  UP_MB(32)   },
    { 0xC84017, "GD25Q64",  UP_MB(8)    },
    { 0xC84018, "GD25Q128", UP_MB(16)   },
    { 0x1F8501, "AT25SF081",UP_MB(1)    },
    { 0x202017, "MX25L6433",UP_MB(8)    },
    { 0, NULL, 0 },
};

/* ── Helper functions ────────────────────────────────────────────────── */

static up_status_t flash_cmd(up_protocol_t *proto, uint8_t cmd)
{
    return up_proto_transfer(proto, &cmd, 1, NULL, 0);
}

static up_status_t flash_wait_ready(up_protocol_t *proto, uint32_t timeout_ms)
{
    uint64_t start = hal_timer_get_us();
    uint64_t timeout_us = (uint64_t)timeout_ms * 1000;

    while (1) {
        uint8_t cmd = CMD_READ_STATUS1;
        uint8_t sr = 0xFF;
        up_status_t st = up_proto_transfer(proto, &cmd, 1, &sr, 1);
        if (st != UP_OK) return st;
        if (!(sr & SR1_BUSY)) return UP_OK;
        if (hal_timer_timeout(start, timeout_us)) return UP_ERROR_TIMEOUT;
        hal_timer_delay_us(100);
    }
}

/* ── Device ops ──────────────────────────────────────────────────────── */

static up_status_t spi_flash_detect(up_device_t *dev)
{
    uint8_t cmd = CMD_READ_JEDEC_ID;
    uint8_t resp[3] = {0};

    up_status_t st = up_proto_transfer(dev->proto, &cmd, 1, resp, 3);
    if (st != UP_OK) return st;

    uint32_t jedec = ((uint32_t)resp[0] << 16) |
                     ((uint32_t)resp[1] << 8)  | resp[2];

    if (jedec == 0 || jedec == 0xFFFFFF) return UP_ERROR_NO_DEVICE;

    dev->info.id = jedec;
    dev->info.type = UP_DEVICE_TYPE_SPI_FLASH;
    dev->info.caps = UP_CAP_READ | UP_CAP_WRITE | UP_CAP_ERASE |
                     UP_CAP_VERIFY | UP_CAP_DETECT;
    dev->info.page_size = 256;
    dev->info.sector_size = 4096;

    /* Look up in known table */
    for (int i = 0; known_chips[i].name; i++) {
        if (known_chips[i].jedec_id == jedec) {
            strncpy(dev->info.name, known_chips[i].name, sizeof(dev->info.name) - 1);
            dev->info.total_size = known_chips[i].size;
            return UP_OK;
        }
    }

    /* Unknown chip — calculate size from capacity byte */
    snprintf(dev->info.name, sizeof(dev->info.name),
             "SPI Flash %02X:%02X:%02X", resp[0], resp[1], resp[2]);
    uint8_t cap = resp[2];
    if (cap >= 0x10 && cap <= 0x20) {
        dev->info.total_size = 1U << cap;
    } else {
        dev->info.total_size = 0; /* Unknown */
    }

    return UP_OK;
}

static up_status_t spi_flash_init(up_device_t *dev)
{
    /* Release from power down */
    uint8_t cmd = CMD_RELEASE_PD;
    up_proto_transfer(dev->proto, &cmd, 1, NULL, 0);
    hal_timer_delay_ms(1);
    dev->initialized = true;
    return UP_OK;
}

static up_status_t spi_flash_deinit(up_device_t *dev)
{
    dev->initialized = false;
    return UP_OK;
}

static up_status_t spi_flash_read(up_device_t *dev, uint32_t addr,
                                   uint8_t *data, size_t len,
                                   up_progress_cb_t cb, void *user_data)
{
    size_t offset = 0;
    const size_t chunk_size = UP_BUFFER_SIZE;

    while (offset < len) {
        size_t remaining = len - offset;
        size_t chunk = UP_MIN(remaining, chunk_size);
        uint32_t cur_addr = addr + offset;

        uint8_t cmd[4] = {
            CMD_READ_DATA,
            (cur_addr >> 16) & 0xFF,
            (cur_addr >> 8)  & 0xFF,
            cur_addr & 0xFF
        };

        up_status_t st = up_proto_transfer(dev->proto, cmd, 4,
                                            data + offset, chunk);
        if (st != UP_OK) return st;

        offset += chunk;
        if (cb) cb((uint32_t)offset, (uint32_t)len, user_data);
    }

    return UP_OK;
}

static up_status_t spi_flash_write(up_device_t *dev, uint32_t addr,
                                    const uint8_t *data, size_t len,
                                    up_progress_cb_t cb, void *user_data)
{
    size_t offset = 0;

    while (offset < len) {
        /* Page-align: write up to end of current page */
        uint32_t cur_addr = addr + offset;
        uint32_t page_offset = cur_addr % dev->info.page_size;
        size_t chunk = UP_MIN(dev->info.page_size - page_offset, len - offset);

        /* Write Enable */
        flash_cmd(dev->proto, CMD_WRITE_ENABLE);

        /* Page Program: cmd + 3-byte address + data */
        uint8_t hdr[4] = {
            CMD_PAGE_PROGRAM,
            (cur_addr >> 16) & 0xFF,
            (cur_addr >> 8)  & 0xFF,
            cur_addr & 0xFF
        };

        /* Need combined transfer: header + data in one CS assertion */
        /* Use protocol-level transfer for header, then raw SPI for data */
        /* For simplicity, build a combined buffer */
        uint8_t buf[4 + 256];
        memcpy(buf, hdr, 4);
        memcpy(buf + 4, data + offset, chunk);
        up_status_t st = up_proto_transfer(dev->proto, buf, 4 + chunk, NULL, 0);
        if (st != UP_OK) return st;

        /* Wait for completion */
        st = flash_wait_ready(dev->proto, UP_FLASH_WRITE_TIMEOUT_MS);
        if (st != UP_OK) return st;

        offset += chunk;
        if (cb) cb((uint32_t)offset, (uint32_t)len, user_data);
    }

    return UP_OK;
}

static up_status_t spi_flash_erase(up_device_t *dev, uint32_t addr,
                                    size_t len,
                                    up_progress_cb_t cb, void *user_data)
{
    /* Chip erase */
    if (addr == 0 && len >= dev->info.total_size) {
        flash_cmd(dev->proto, CMD_WRITE_ENABLE);
        flash_cmd(dev->proto, CMD_CHIP_ERASE);
        up_status_t st = flash_wait_ready(dev->proto, UP_ERASE_TIMEOUT_MS);
        if (cb) cb(len, len, user_data);
        return st;
    }

    /* Sector erase (4KB sectors) */
    size_t offset = 0;
    while (offset < len) {
        uint32_t cur_addr = addr + offset;

        flash_cmd(dev->proto, CMD_WRITE_ENABLE);

        uint8_t cmd[4] = {
            CMD_SECTOR_ERASE,
            (cur_addr >> 16) & 0xFF,
            (cur_addr >> 8)  & 0xFF,
            cur_addr & 0xFF
        };
        up_status_t st = up_proto_transfer(dev->proto, cmd, 4, NULL, 0);
        if (st != UP_OK) return st;

        st = flash_wait_ready(dev->proto, UP_ERASE_TIMEOUT_MS);
        if (st != UP_OK) return st;

        offset += dev->info.sector_size;
        if (cb) cb(UP_MIN(offset, len), len, user_data);
    }

    return UP_OK;
}

static up_status_t spi_flash_verify(up_device_t *dev, uint32_t addr,
                                     const uint8_t *data, size_t len,
                                     up_progress_cb_t cb, void *user_data)
{
    uint8_t buf[256];
    size_t offset = 0;

    while (offset < len) {
        size_t chunk = UP_MIN(sizeof(buf), len - offset);
        up_status_t st = spi_flash_read(dev, addr + offset, buf, chunk, NULL, NULL);
        if (st != UP_OK) return st;

        if (memcmp(buf, data + offset, chunk) != 0) {
            return UP_ERROR_VERIFY;
        }

        offset += chunk;
        if (cb) cb(offset, len, user_data);
    }

    return UP_OK;
}

/* ── Public ops table ────────────────────────────────────────────────── */

const up_device_ops_t spi_flash_device_ops = {
    .name    = "spi_flash",
    .type    = UP_DEVICE_TYPE_SPI_FLASH,
    .detect  = spi_flash_detect,
    .init    = spi_flash_init,
    .deinit  = spi_flash_deinit,
    .read    = spi_flash_read,
    .write   = spi_flash_write,
    .erase   = spi_flash_erase,
    .verify  = spi_flash_verify,
    .emulate = NULL,
    .ioctl   = NULL,
};
