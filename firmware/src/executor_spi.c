#include "bridge_protocol.h"
#include "src/core/hal/hal.h"
#include <string.h>

static hal_spi_inst_t *spi_inst = NULL;
static up_pin_t cs_pin = 17; // Default CS for Pico bridge

void exec_spi(up_frame_t *f) {
    switch (f->cmd) {
        case CMD_SPI_INIT: {
            if (f->len < 5) { up_protocol_send_error(f->seq, 0x02); return; }
            uint32_t speed = f->payload[0] | (f->payload[1] << 8) | (f->payload[2] << 16) | (f->payload[3] << 24);
            uint8_t mode = f->payload[4];
            
            hal_spi_config_t cfg = {
                .instance = 0,
                .freq_hz = speed,
                .mode = (up_spi_mode_t)mode,
                .bit_order = UP_SPI_BIT_ORDER_MSB_FIRST,
                .pin_sck = 18,
                .pin_mosi = 19,
                .pin_miso = 16,
                .pin_cs = UP_PIN_NONE // We use manual GPIO for CS
            };
            
            // Re-init if already open
            if (spi_inst) hal_spi_deinit(spi_inst);
            
            if (hal_spi_init(&spi_inst, &cfg) == UP_OK) {
                hal_gpio_init(cs_pin, UP_GPIO_DIR_OUTPUT, UP_GPIO_PULL_NONE);
                hal_gpio_write(cs_pin, UP_GPIO_LEVEL_HIGH);
                up_protocol_send_ack(f->seq);
            } else {
                up_protocol_send_error(f->seq, 0x10);
            }
            break;
        }
        case CMD_SPI_TRANSFER: {
            if (!spi_inst) { up_protocol_send_error(f->seq, 0x10); return; }
            if (f->len == 0 || f->len > 1024) { up_protocol_send_error(f->seq, 0x02); return; }
            
            uint8_t rx_buf[1024];
            if (hal_spi_transfer(spi_inst, f->payload, rx_buf, f->len) == UP_OK) {
                up_protocol_send(f->seq, CMD_DATA, rx_buf, f->len);
            } else {
                up_protocol_send_error(f->seq, 0x11);
            }
            break;
        }
        case CMD_SPI_CS: {
            if (f->len < 1) { up_protocol_send_error(f->seq, 0x02); return; }
            bool assert_cs = f->payload[0];
            hal_gpio_write(cs_pin, assert_cs ? UP_GPIO_LEVEL_LOW : UP_GPIO_LEVEL_HIGH);
            up_protocol_send_ack(f->seq);
            break;
        }
        case CMD_SPI_JEDEC_ID: {
            if (!spi_inst) { up_protocol_send_error(f->seq, 0x10); return; }
            uint8_t tx[4] = { 0x9F, 0x00, 0x00, 0x00 };
            uint8_t rx[4] = {0};
            hal_gpio_write(cs_pin, UP_GPIO_LEVEL_LOW);
            hal_spi_transfer(spi_inst, tx, rx, 4);
            hal_gpio_write(cs_pin, UP_GPIO_LEVEL_HIGH);
            up_protocol_send(f->seq, CMD_DATA, &rx[1], 3);
            break;
        }
        default:
            up_protocol_send_error(f->seq, 0x01);
            break;
    }
}
