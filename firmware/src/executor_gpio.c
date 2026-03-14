#include "bridge_protocol.h"
#include "src/core/hal/hal.h"

void exec_gpio(up_frame_t *f) {
    switch (f->cmd) {
        case CMD_GPIO_CONFIG: {
            if (f->len < 3) { up_protocol_send_error(f->seq, 0x02); return; }
            up_pin_t pin = f->payload[0];
            uint8_t mode = f->payload[1];
            uint8_t pull = f->payload[2];
            
            hal_gpio_init(pin, (up_gpio_dir_t)mode, (up_gpio_pull_t)pull);
            up_protocol_send_ack(f->seq);
            break;
        }
        case CMD_GPIO_SET: {
            if (f->len < 2) { up_protocol_send_error(f->seq, 0x02); return; }
            up_pin_t pin = f->payload[0];
            uint8_t level = f->payload[1];
            hal_gpio_write(pin, (up_gpio_level_t)level);
            up_protocol_send_ack(f->seq);
            break;
        }
        case CMD_GPIO_GET: {
            if (f->len < 1) { up_protocol_send_error(f->seq, 0x02); return; }
            up_pin_t pin = f->payload[0];
            uint8_t level = (uint8_t)hal_gpio_read(pin);
            up_protocol_send(f->seq, CMD_DATA, &level, 1);
            break;
        }
        default:
            up_protocol_send_error(f->seq, 0x01);
            break;
    }
}
