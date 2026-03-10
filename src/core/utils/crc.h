/**
 * @file crc.h
 * @brief CRC8/CRC16/CRC32 with lookup tables
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_CRC_H
#define UNIPROGER_CRC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t  up_crc8(const uint8_t *data, size_t len);
uint16_t up_crc16(const uint8_t *data, size_t len);
uint32_t up_crc32(const uint8_t *data, size_t len);

uint8_t  up_crc8_update(uint8_t crc, uint8_t byte);
uint16_t up_crc16_update(uint16_t crc, uint8_t byte);
uint32_t up_crc32_update(uint32_t crc, uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_CRC_H */
