/**
 * @file device.h
 * @brief Device abstraction — vtable interface for all device drivers
 *
 * Each device driver (SPI Flash, EEPROM, AVR, STM32...) implements this
 * interface and registers itself with the device registry.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_DEVICE_H
#define UNIPROGER_DEVICE_H

#include "uniproger/types.h"
#include "src/core/protocol/protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Forward declaration */
typedef struct up_device up_device_t;

/** Device information (filled by detect/init) */
typedef struct {
    char             name[64];
    up_device_type_t type;
    up_device_caps_t caps;
    uint32_t         id;           /**< JEDEC ID, signature, etc. */
    uint32_t         total_size;   /**< Total memory size in bytes */
    uint32_t         page_size;    /**< Write page size */
    uint32_t         sector_size;  /**< Erase sector size */
    uint32_t         num_regions;
    const up_mem_region_t *regions;
} up_device_info_t;

/** Device operations vtable */
typedef struct {
    const char       *name;
    up_device_type_t  type;

    /** Detect device and fill info */
    up_status_t (*detect)(up_device_t *dev);

    /** Initialize device for operations */
    up_status_t (*init)(up_device_t *dev);

    /** Deinitialize device */
    up_status_t (*deinit)(up_device_t *dev);

    /** Read data from device */
    up_status_t (*read)(up_device_t *dev, uint32_t addr,
                        uint8_t *data, size_t len,
                        up_progress_cb_t cb, void *user_data);

    /** Write data to device */
    up_status_t (*write)(up_device_t *dev, uint32_t addr,
                         const uint8_t *data, size_t len,
                         up_progress_cb_t cb, void *user_data);

    /** Erase region (or chip if addr=0, len=total_size) */
    up_status_t (*erase)(up_device_t *dev, uint32_t addr, size_t len,
                         up_progress_cb_t cb, void *user_data);

    /** Verify data against device contents */
    up_status_t (*verify)(up_device_t *dev, uint32_t addr,
                          const uint8_t *data, size_t len,
                          up_progress_cb_t cb, void *user_data);

    /** Emulate device (respond as target) */
    up_status_t (*emulate)(up_device_t *dev);

    /** Device-specific command */
    up_status_t (*ioctl)(up_device_t *dev, uint32_t cmd,
                         void *arg, size_t arg_len);
} up_device_ops_t;

/** Device instance */
struct up_device {
    const up_device_ops_t *ops;
    up_protocol_t         *proto;       /**< Protocol used to communicate */
    up_device_info_t       info;
    void                  *priv;        /**< Driver-private data */
    bool                   initialized;
};

/* ── Device convenience wrappers ─────────────────────────────────────── */

static inline up_status_t up_dev_detect(up_device_t *d)
{
    if (!d || !d->ops || !d->ops->detect) return UP_ERROR_INVALID_ARG;
    return d->ops->detect(d);
}

static inline up_status_t up_dev_read(up_device_t *d, uint32_t addr,
                                       uint8_t *data, size_t len,
                                       up_progress_cb_t cb, void *ud)
{
    if (!d || !d->initialized || !d->ops->read) return UP_ERROR_INVALID_ARG;
    return d->ops->read(d, addr, data, len, cb, ud);
}

static inline up_status_t up_dev_write(up_device_t *d, uint32_t addr,
                                        const uint8_t *data, size_t len,
                                        up_progress_cb_t cb, void *ud)
{
    if (!d || !d->initialized || !d->ops->write) return UP_ERROR_INVALID_ARG;
    return d->ops->write(d, addr, data, len, cb, ud);
}

static inline up_status_t up_dev_erase(up_device_t *d, uint32_t addr,
                                        size_t len,
                                        up_progress_cb_t cb, void *ud)
{
    if (!d || !d->initialized || !d->ops->erase) return UP_ERROR_INVALID_ARG;
    return d->ops->erase(d, addr, len, cb, ud);
}

/* ── Device registry ─────────────────────────────────────────────────── */

up_status_t up_device_registry_init(void);
up_status_t up_device_register(const up_device_ops_t *ops);
up_status_t up_device_unregister(const char *name);

/** Find device by name */
const up_device_ops_t *up_device_find(const char *name);

/** Auto-detect: try all registered drivers */
up_status_t up_device_auto_detect(up_protocol_t *proto, up_device_t *dev);

/** Get number of registered devices */
size_t up_device_count(void);

/** Get device ops by index */
const up_device_ops_t *up_device_get(size_t index);

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_DEVICE_H */
