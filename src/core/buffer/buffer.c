/**
 * @file buffer.c
 * @brief Buffer manager — ring buffer and hex dump
 *
 * SPDX-License-Identifier: MIT
 */

#include "buffer.h"
#include <stdio.h>
#include <string.h>

up_status_t up_ringbuf_init(up_ringbuf_t *rb, uint8_t *buf, size_t size)
{
    if (!rb || !buf || size == 0) return UP_ERROR_INVALID_ARG;
    rb->data = buf;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    return UP_OK;
}

size_t up_ringbuf_put(up_ringbuf_t *rb, const uint8_t *data, size_t len)
{
    size_t written = 0;
    while (written < len && rb->count < rb->size) {
        rb->data[rb->head] = data[written++];
        rb->head = (rb->head + 1) % rb->size;
        rb->count++;
    }
    return written;
}

size_t up_ringbuf_get(up_ringbuf_t *rb, uint8_t *data, size_t len)
{
    size_t read = 0;
    while (read < len && rb->count > 0) {
        data[read++] = rb->data[rb->tail];
        rb->tail = (rb->tail + 1) % rb->size;
        rb->count--;
    }
    return read;
}

size_t up_ringbuf_available(const up_ringbuf_t *rb) { return rb->count; }
size_t up_ringbuf_free(const up_ringbuf_t *rb) { return rb->size - rb->count; }
void   up_ringbuf_reset(up_ringbuf_t *rb) { rb->head = rb->tail = rb->count = 0; }

void up_hexdump(const void *data, size_t len, uint32_t base_addr)
{
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < len; i += 16) {
        printf("%08lX: ", (unsigned long)(base_addr + i));
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len)
                printf("%02X ", p[i + j]);
            else
                printf("   ");
            if (j == 7) printf(" ");
        }
        printf(" |");
        for (size_t j = 0; j < 16 && (i + j) < len; j++) {
            uint8_t c = p[i + j];
            printf("%c", (c >= 0x20 && c < 0x7F) ? c : '.');
        }
        printf("|\n");
    }
}

int32_t up_memcmp_offset(const void *a, const void *b, size_t len)
{
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    for (size_t i = 0; i < len; i++) {
        if (pa[i] != pb[i]) return (int32_t)i;
    }
    return -1;
}

void up_memfill(void *buf, size_t len, uint8_t pattern)
{
    memset(buf, pattern, len);
}
