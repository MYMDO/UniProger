#include "bridge_protocol.h"
#include "src/core/hal/hal.h"
#include <string.h>

void exec_system(up_frame_t *f) {
    switch (f->cmd) {
        case CMD_PING:
            up_protocol_send(f->seq, CMD_PONG, NULL, 0);
            break;
        case CMD_VERSION: {
            const char *ver = "UNIPROG-BRIDGE-v2.5";
            up_protocol_send(f->seq, CMD_DATA, (const uint8_t*)ver, (uint16_t)strlen(ver));
            break;
        }
        case CMD_RESET:
            up_protocol_send_ack(f->seq);
            hal_platform_reset(); // Reboot RP2040
            break;
        default:
            up_protocol_send_error(f->seq, 0x01);
            break;
    }
}
