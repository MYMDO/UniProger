/**
 * @file protocol.h
 * @brief Protocol engine interface — vtable pattern for all protocol drivers
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_PROTOCOL_H
#define UNIPROGER_PROTOCOL_H

#include "uniproger/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Protocol type identifiers */
typedef enum {
    UP_PROTO_SPI     = 0,
    UP_PROTO_I2C     = 1,
    UP_PROTO_JTAG    = 2,
    UP_PROTO_SWD     = 3,
    UP_PROTO_UART    = 4,
    UP_PROTO_ONEWIRE = 5,
    UP_PROTO_MAX
} up_protocol_type_t;

/** Forward declaration */
typedef struct up_protocol up_protocol_t;

/** Protocol operations vtable */
typedef struct {
    const char        *name;
    up_protocol_type_t type;

    up_status_t (*init)(up_protocol_t *proto, const void *config);
    up_status_t (*deinit)(up_protocol_t *proto);
    up_status_t (*reset)(up_protocol_t *proto);

    /** Generic transfer: send tx_len bytes, receive rx_len bytes */
    up_status_t (*transfer)(up_protocol_t *proto,
                            const uint8_t *tx, size_t tx_len,
                            uint8_t *rx, size_t rx_len);

    /** Detect devices on the bus (returns count, fills id buffer) */
    up_status_t (*detect)(up_protocol_t *proto,
                          uint32_t *ids, size_t max_ids, size_t *found);

    /** Set bus speed/frequency */
    up_status_t (*set_speed)(up_protocol_t *proto, uint32_t speed_hz);
} up_protocol_ops_t;

/** Protocol instance */
struct up_protocol {
    const up_protocol_ops_t *ops;
    void                    *hw_inst;   /**< HAL instance handle */
    void                    *priv;      /**< Protocol-private data */
    bool                     initialized;
};

/* ── Protocol convenience wrappers ───────────────────────────────────── */

static inline up_status_t up_proto_init(up_protocol_t *p, const void *cfg)
{
    if (!p || !p->ops || !p->ops->init) return UP_ERROR_INVALID_ARG;
    up_status_t st = p->ops->init(p, cfg);
    if (st == UP_OK) p->initialized = true;
    return st;
}

static inline up_status_t up_proto_deinit(up_protocol_t *p)
{
    if (!p || !p->ops || !p->ops->deinit) return UP_ERROR_INVALID_ARG;
    p->initialized = false;
    return p->ops->deinit(p);
}

static inline up_status_t up_proto_transfer(up_protocol_t *p,
                                             const uint8_t *tx, size_t tx_len,
                                             uint8_t *rx, size_t rx_len)
{
    if (!p || !p->initialized || !p->ops->transfer) return UP_ERROR_INVALID_ARG;
    return p->ops->transfer(p, tx, tx_len, rx, rx_len);
}

static inline up_status_t up_proto_detect(up_protocol_t *p,
                                           uint32_t *ids, size_t max, size_t *found)
{
    if (!p || !p->initialized || !p->ops->detect) return UP_ERROR_NOT_SUPPORTED;
    return p->ops->detect(p, ids, max, found);
}

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_PROTOCOL_H */
