#include "bridge_protocol.h"
#include <stdio.h>
#include <string.h>

// Define this function locally or include the TinyUSB CDC header
extern void cdc_send_bytes(const uint8_t *data, uint16_t len);

// External executor dispatchers
extern void exec_system(up_frame_t *frame);
extern void exec_spi(up_frame_t *frame);
extern void exec_i2c(up_frame_t *frame);
extern void exec_gpio(up_frame_t *frame);

static rx_state_t state = STATE_SYNC_0;
static up_frame_t rx_frame;
static uint16_t payload_idx = 0;
static uint16_t rx_crc = 0;

static uint16_t crc16_ccitt(uint16_t crc, uint8_t data) {
    crc ^= (uint16_t)data << 8;
    for (int i = 0; i < 8; i++) {
        if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
        else              crc = (crc << 1);
    }
    return crc;
}

static uint16_t calc_frame_crc(up_frame_t *f) {
    uint16_t crc = 0xFFFF;
    crc = crc16_ccitt(crc, f->seq);
    crc = crc16_ccitt(crc, f->cmd);
    for (uint16_t i = 0; i < f->len; i++) {
        crc = crc16_ccitt(crc, f->payload[i]);
    }
    return crc;
}

void up_protocol_init(void) {
    state = STATE_SYNC_0;
}

static void dispatch_frame(up_frame_t *f) {
    uint8_t group = f->cmd & 0xF0;
    switch (group) {
        case 0x00: exec_system(f); break; // System cmds (PING, etc)
        case 0x10: exec_spi(f); break;    // SPI cmds
        case 0x20: exec_i2c(f); break;    // I2C cmds
        case 0x30: exec_gpio(f); break;   // GPIO cmds
        default:
            up_protocol_send_error(f->seq, 0x01); // Unknown command
            break;
    }
}

void up_protocol_feed(uint8_t byte) {
    switch (state) {
        case STATE_SYNC_0:
            if (byte == SYNC_0) state = STATE_SYNC_1;
            break;
        case STATE_SYNC_1:
            if (byte == SYNC_1) state = STATE_LEN_L;
            else if (byte != SYNC_0) state = STATE_SYNC_0;
            break;
        case STATE_LEN_L:
            rx_frame.len = byte;
            state = STATE_LEN_H;
            break;
        case STATE_LEN_H:
            rx_frame.len |= (byte << 8);
            if (rx_frame.len > sizeof(rx_frame.payload)) {
                state = STATE_SYNC_0; // Frame too large
            } else {
                state = STATE_SEQ;
            }
            break;
        case STATE_SEQ:
            rx_frame.seq = byte;
            state = STATE_CMD;
            break;
        case STATE_CMD:
            rx_frame.cmd = byte;
            payload_idx = 0;
            state = (rx_frame.len > 0) ? STATE_PAYLOAD : STATE_CRC_L;
            break;
        case STATE_PAYLOAD:
            rx_frame.payload[payload_idx++] = byte;
            if (payload_idx >= rx_frame.len) {
                state = STATE_CRC_L;
            }
            break;
        case STATE_CRC_L:
            rx_crc = byte;
            state = STATE_CRC_H;
            break;
        case STATE_CRC_H:
            rx_crc |= (byte << 8);
            if (calc_frame_crc(&rx_frame) == rx_crc) {
                dispatch_frame(&rx_frame);
            }
            state = STATE_SYNC_0;
            break;
    }
}

void up_protocol_send(uint8_t req_seq, uint8_t cmd, const uint8_t *payload, uint16_t len) {
    uint8_t header[6];
    header[0] = SYNC_0;
    header[1] = SYNC_1;
    header[2] = len & 0xFF;
    header[3] = (len >> 8) & 0xFF;
    header[4] = req_seq;
    header[5] = cmd;

    uint16_t crc = 0xFFFF;
    crc = crc16_ccitt(crc, req_seq);
    crc = crc16_ccitt(crc, cmd);
    for (uint16_t i = 0; i < len; i++) {
        crc = crc16_ccitt(crc, payload[i]);
    }

    uint8_t footer[2];
    footer[0] = crc & 0xFF;
    footer[1] = (crc >> 8) & 0xFF;

    // In a real RTOS this should be atomic/mutex protected
    cdc_send_bytes(header, 6);
    if (len > 0) {
        cdc_send_bytes(payload, len);
    }
    cdc_send_bytes(footer, 2);
}

void up_protocol_send_ack(uint8_t req_seq) {
    up_protocol_send(req_seq, CMD_ACK, NULL, 0);
}

void up_protocol_send_error(uint8_t req_seq, uint8_t err_code) {
    up_protocol_send(req_seq, CMD_ERROR, &err_code, 1);
}
