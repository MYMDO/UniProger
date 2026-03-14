#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "src/core/hal/hal.h"
#include "bridge_protocol.h"

// ── CDC Task ────────────────────────────────────────────────────────────────

void cdc_task(void) {
    if (tud_cdc_connected()) {
        if (tud_cdc_available()) {
            uint8_t buf[256];
            uint32_t count = tud_cdc_read(buf, sizeof(buf));
            for (uint32_t i = 0; i < count; i++) {
                up_protocol_feed(buf[i]);
            }
        }
    }
}

// Helper called by bridge_protocol.c to send data back to host
void cdc_send_bytes(const uint8_t *data, uint16_t len) {
    if (tud_cdc_connected()) {
        tud_cdc_write(data, len);
        tud_cdc_write_flush();
    }
}

// ── TinyUSB Callbacks ───────────────────────────────────────────────────────

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    (void) itf; (void) rts;
    // DTR signal used to reset engine or trigger actions on host connect/disconnect
    if (dtr) {
        up_protocol_init();
    }
}

// ── Main Entry ──────────────────────────────────────────────────────────────

int main(void) {
    board_init();
    tusb_init();

    // Initialize platform HAL (clocks, unique id, etc)
    hal_platform_init();
    
    // Initialize protocol engine
    up_protocol_init();

    printf("UNIPROG SaaS Bridge Started\n");

    while (1) {
        tud_task(); // TinyUSB device task
        cdc_task(); // Read bridge protocol from USB CDC
    }

    return 0;
}
