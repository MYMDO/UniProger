/**
 * @file rp2040_hal_uart.c
 * @brief RP2040 UART HAL implementation
 *
 * SPDX-License-Identifier: MIT
 */

#include "src/core/hal/hal_uart.h"

#ifdef UNIPROGER_PLATFORM_RP2040

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

/* ── Instance storage ────────────────────────────────────────────────── */

struct hal_uart_inst {
    uart_inst_t     *hw_uart;
    hal_uart_rx_cb_t rx_callback;
    void            *rx_user_data;
};

static struct hal_uart_inst s_uart_instances[2];

/* ── IRQ handlers ────────────────────────────────────────────────────── */

static void rp2040_uart_irq_handler_common(uint idx)
{
    struct hal_uart_inst *ui = &s_uart_instances[idx];
    if (!ui->rx_callback) return;

    uint8_t buf[32];
    size_t count = 0;

    while (uart_is_readable(ui->hw_uart) && count < sizeof(buf)) {
        buf[count++] = uart_getc(ui->hw_uart);
    }

    if (count > 0) {
        ui->rx_callback(ui, buf, count, ui->rx_user_data);
    }
}

static void rp2040_uart0_irq(void) { rp2040_uart_irq_handler_common(0); }
static void rp2040_uart1_irq(void) { rp2040_uart_irq_handler_common(1); }

/* ── UART API ────────────────────────────────────────────────────────── */

up_status_t hal_uart_init(hal_uart_inst_t **inst, const hal_uart_config_t *cfg)
{
    if (!inst || !cfg || cfg->instance > 1) return UP_ERROR_INVALID_ARG;

    uart_inst_t *hw = cfg->instance == 0 ? uart0 : uart1;
    struct hal_uart_inst *ui = &s_uart_instances[cfg->instance];

    ui->hw_uart = hw;
    ui->rx_callback = cfg->rx_callback;
    ui->rx_user_data = cfg->rx_user_data;

    uart_init(hw, cfg->baud_rate);

    /* Set format */
    uart_set_format(hw, cfg->data_bits, cfg->stop_bits,
                    cfg->parity == UP_UART_PARITY_EVEN ? UART_PARITY_EVEN :
                    cfg->parity == UP_UART_PARITY_ODD  ? UART_PARITY_ODD :
                                                          UART_PARITY_NONE);

    gpio_set_function(cfg->pin_tx, GPIO_FUNC_UART);
    gpio_set_function(cfg->pin_rx, GPIO_FUNC_UART);

    /* Enable RX interrupt if callback provided */
    if (cfg->rx_callback) {
        uint irq_num = cfg->instance == 0 ? UART0_IRQ : UART1_IRQ;
        irq_handler_t handler = cfg->instance == 0 ? rp2040_uart0_irq : rp2040_uart1_irq;

        irq_set_exclusive_handler(irq_num, handler);
        irq_set_enabled(irq_num, true);
        uart_set_irq_enables(hw, true, false);
    }

    *inst = ui;
    return UP_OK;
}

up_status_t hal_uart_deinit(hal_uart_inst_t *inst)
{
    if (!inst) return UP_ERROR_INVALID_ARG;
    uart_deinit(inst->hw_uart);
    inst->rx_callback = NULL;
    return UP_OK;
}

up_status_t hal_uart_set_baud(hal_uart_inst_t *inst, uint32_t baud_rate)
{
    if (!inst) return UP_ERROR_INVALID_ARG;
    uart_set_baudrate(inst->hw_uart, baud_rate);
    return UP_OK;
}

up_status_t hal_uart_write(hal_uart_inst_t *inst,
                            const uint8_t *data, size_t len,
                            uint32_t timeout_ms)
{
    if (!inst || !data) return UP_ERROR_INVALID_ARG;
    UP_UNUSED(timeout_ms);

    uart_write_blocking(inst->hw_uart, data, len);
    return UP_OK;
}

up_status_t hal_uart_read(hal_uart_inst_t *inst,
                           uint8_t *data, size_t len,
                           size_t *actual, uint32_t timeout_ms)
{
    if (!inst || !data || !actual) return UP_ERROR_INVALID_ARG;

    *actual = 0;
    uint64_t start = time_us_64();
    uint64_t timeout_us = (uint64_t)timeout_ms * 1000;

    while (*actual < len) {
        if (uart_is_readable(inst->hw_uart)) {
            data[*actual] = uart_getc(inst->hw_uart);
            (*actual)++;
        } else if ((time_us_64() - start) >= timeout_us) {
            return (*actual > 0) ? UP_OK : UP_ERROR_TIMEOUT;
        }
    }

    return UP_OK;
}

bool hal_uart_available(hal_uart_inst_t *inst)
{
    if (!inst) return false;
    return uart_is_readable(inst->hw_uart);
}

up_status_t hal_uart_flush(hal_uart_inst_t *inst)
{
    if (!inst) return UP_ERROR_INVALID_ARG;
    uart_tx_wait_blocking(inst->hw_uart);
    return UP_OK;
}

#endif /* UNIPROGER_PLATFORM_RP2040 */
