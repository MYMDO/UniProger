#include "bridge_protocol.h"
#include "src/core/hal/hal.h"

static hal_i2c_inst_t *i2c_inst = NULL;

void exec_i2c(up_frame_t *f) {
    switch (f->cmd) {
        case CMD_I2C_INIT: {
            if (f->len < 4) { up_protocol_send_error(f->seq, 0x02); return; }
            uint32_t speed = f->payload[0] | (f->payload[1] << 8) | (f->payload[2] << 16) | (f->payload[3] << 24);
            hal_i2c_config_t cfg = { 
                .instance = 0,
                .freq_hz = speed,
                .pin_sda = 4,
                .pin_scl = 5
            };
            
            if (i2c_inst) hal_i2c_deinit(i2c_inst);
            
            if (hal_i2c_init(&i2c_inst, &cfg) == UP_OK) {
                up_protocol_send_ack(f->seq);
            } else {
                up_protocol_send_error(f->seq, 0x20);
            }
            break;
        }
        case CMD_I2C_WRITE: {
            if (!i2c_inst) { up_protocol_send_error(f->seq, 0x20); return; }
            if (f->len < 2) { up_protocol_send_error(f->seq, 0x02); return; }
            uint8_t addr = f->payload[0];
            uint16_t data_len = f->len - 1;
            
            if (hal_i2c_write(i2c_inst, addr, &f->payload[1], data_len, false, 100) == UP_OK) {
                up_protocol_send_ack(f->seq);
            } else {
                up_protocol_send_error(f->seq, 0x21);
            }
            break;
        }
        case CMD_I2C_READ: {
            if (!i2c_inst) { up_protocol_send_error(f->seq, 0x20); return; }
            if (f->len < 3) { up_protocol_send_error(f->seq, 0x02); return; }
            uint8_t addr = f->payload[0];
            uint16_t read_len = f->payload[1] | (f->payload[2] << 8);
            
            if (read_len > 1024) { up_protocol_send_error(f->seq, 0x02); return; }
            
            uint8_t rx_buf[1024];
            if (hal_i2c_read(i2c_inst, addr, rx_buf, read_len, false, 100) == UP_OK) {
                up_protocol_send(f->seq, CMD_DATA, rx_buf, read_len);
            } else {
                up_protocol_send_error(f->seq, 0x22);
            }
            break;
        }
        case CMD_I2C_SCAN: {
            if (!i2c_inst) { up_protocol_send_error(f->seq, 0x20); return; }
            uint8_t found_addrs[128];
            size_t count = 0;
            if (hal_i2c_scan(i2c_inst, found_addrs, sizeof(found_addrs), &count) == UP_OK) {
                up_protocol_send(f->seq, CMD_DATA, found_addrs, (uint16_t)count);
            } else {
                up_protocol_send_error(f->seq, 0x23);
            }
            break;
        }
        default:
            up_protocol_send_error(f->seq, 0x01);
            break;
    }
}
