/**
 * @file device_registry.c
 * @brief Plugin registry — register/unregister/lookup device drivers
 *
 * SPDX-License-Identifier: MIT
 */

#include "device.h"
#include "uniproger/config.h"
#include <string.h>

static const up_device_ops_t *s_registry[UP_MAX_DEVICES];
static size_t s_count = 0;

up_status_t up_device_registry_init(void)
{
    memset(s_registry, 0, sizeof(s_registry));
    s_count = 0;
    return UP_OK;
}

up_status_t up_device_register(const up_device_ops_t *ops)
{
    if (!ops || !ops->name) return UP_ERROR_INVALID_ARG;
    if (s_count >= UP_MAX_DEVICES) return UP_ERROR_OVERFLOW;

    /* Check for duplicate */
    for (size_t i = 0; i < s_count; i++) {
        if (strcmp(s_registry[i]->name, ops->name) == 0) {
            return UP_ERROR_BUSY; /* Already registered */
        }
    }

    s_registry[s_count++] = ops;
    return UP_OK;
}

up_status_t up_device_unregister(const char *name)
{
    if (!name) return UP_ERROR_INVALID_ARG;

    for (size_t i = 0; i < s_count; i++) {
        if (strcmp(s_registry[i]->name, name) == 0) {
            /* Shift remaining entries */
            for (size_t j = i; j < s_count - 1; j++) {
                s_registry[j] = s_registry[j + 1];
            }
            s_count--;
            return UP_OK;
        }
    }
    return UP_ERROR_NOT_FOUND;
}

const up_device_ops_t *up_device_find(const char *name)
{
    if (!name) return NULL;
    for (size_t i = 0; i < s_count; i++) {
        if (strcmp(s_registry[i]->name, name) == 0) {
            return s_registry[i];
        }
    }
    return NULL;
}

up_status_t up_device_auto_detect(up_protocol_t *proto, up_device_t *dev)
{
    if (!proto || !dev) return UP_ERROR_INVALID_ARG;

    for (size_t i = 0; i < s_count; i++) {
        dev->ops = s_registry[i];
        dev->proto = proto;
        dev->initialized = false;

        if (dev->ops->detect && dev->ops->detect(dev) == UP_OK) {
            /* Device detected — initialize it */
            if (dev->ops->init) {
                up_status_t st = dev->ops->init(dev);
                if (st == UP_OK) {
                    dev->initialized = true;
                    return UP_OK;
                }
            }
            return UP_OK;
        }
    }

    return UP_ERROR_NOT_FOUND;
}

size_t up_device_count(void)
{
    return s_count;
}

const up_device_ops_t *up_device_get(size_t index)
{
    if (index >= s_count) return NULL;
    return s_registry[index];
}
