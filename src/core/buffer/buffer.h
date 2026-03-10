/**
 * @file buffer.h
 * @brief Buffer manager — ring buffer and hex dump utility
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_BUFFER_H
#define UNIPROGER_BUFFER_H

#include "uniproger/types.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Ring buffer */
typedef struct {
    uint8_t *data;
    size_t   size;
    size_t   head;
    size_t   tail;
    size_t   count;
} up_ringbuf_t;

up_status_t up_ringbuf_init(up_ringbuf_t *rb, uint8_t *buf, size_t size);
size_t      up_ringbuf_put(up_ringbuf_t *rb, const uint8_t *data, size_t len);
size_t      up_ringbuf_get(up_ringbuf_t *rb, uint8_t *data, size_t len);
size_t      up_ringbuf_available(const up_ringbuf_t *rb);
size_t      up_ringbuf_free(const up_ringbuf_t *rb);
void        up_ringbuf_reset(up_ringbuf_t *rb);

/** Hex dump a buffer to printf-like output */
void up_hexdump(const void *data, size_t len, uint32_t base_addr);

/** Compare two buffers, return first mismatch offset or -1 if equal */
int32_t up_memcmp_offset(const void *a, const void *b, size_t len);

/** Fill buffer with pattern */
void up_memfill(void *buf, size_t len, uint8_t pattern);

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_BUFFER_H */
