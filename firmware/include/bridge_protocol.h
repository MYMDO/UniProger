#ifndef UNIPROG_BRIDGE_PROTOCOL_H
#define UNIPROG_BRIDGE_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

// SYNC bytes
#define SYNC_0 0x55
#define SYNC_1 0xAA

// Commands matching frontend lib/protocol.js
typedef enum {
    CMD_PING           = 0x01,
    CMD_PONG           = 0x02,
    CMD_VERSION        = 0x03,
    CMD_RESET          = 0x0F,

    CMD_SPI_INIT       = 0x10,
    CMD_SPI_TRANSFER   = 0x11,
    CMD_SPI_CS         = 0x12,
    CMD_SPI_JEDEC_ID   = 0x13,

    CMD_I2C_INIT       = 0x20,
    CMD_I2C_WRITE      = 0x21,
    CMD_I2C_READ       = 0x22,
    CMD_I2C_SCAN       = 0x23,

    CMD_GPIO_SET       = 0x30,
    CMD_GPIO_GET       = 0x31,
    CMD_GPIO_CONFIG    = 0x32,

    CMD_JTAG_INIT      = 0x40,
    CMD_JTAG_SHIFT_IR  = 0x41,
    CMD_JTAG_SHIFT_DR  = 0x42,
    CMD_JTAG_IDCODE    = 0x43,

    CMD_SWD_INIT       = 0x50,
    CMD_SWD_READ       = 0x51,
    CMD_SWD_WRITE      = 0x52,

    CMD_ACK            = 0xA0,
    CMD_NAK            = 0xA1,
    CMD_DATA           = 0xA2,
    CMD_ERROR          = 0xAF,
} up_cmd_t;

// Frame state machine
typedef enum {
    STATE_SYNC_0,
    STATE_SYNC_1,
    STATE_LEN_L,
    STATE_LEN_H,
    STATE_SEQ,
    STATE_CMD,
    STATE_PAYLOAD,
    STATE_CRC_L,
    STATE_CRC_H
} rx_state_t;

typedef struct {
    uint8_t seq;
    uint8_t cmd;
    uint16_t len;
    uint8_t payload[2048];
} up_frame_t;

// Initialize the protocol engine
void up_protocol_init(void);

// Feed a byte from USB CDC into the parser
void up_protocol_feed(uint8_t byte);

// Send a response frame back to the host
void up_protocol_send(uint8_t req_seq, uint8_t cmd, const uint8_t *payload, uint16_t len);

// Send a simple ACK
void up_protocol_send_ack(uint8_t req_seq);

// Send an Error
void up_protocol_send_error(uint8_t req_seq, uint8_t err_code);

#endif // UNIPROG_BRIDGE_PROTOCOL_H
