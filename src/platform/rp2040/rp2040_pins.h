/**
 * @file rp2040_pins.h
 * @brief Default pin mapping for RP2040 (Raspberry Pi Pico)
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_RP2040_PINS_H
#define UNIPROGER_RP2040_PINS_H

/* ── System ──────────────────────────────────────────────────────────── */
#define RP2040_PIN_LED         25

/* ── SPI0 — Main programmer SPI bus ──────────────────────────────────── */
#define RP2040_PIN_SPI0_SCK    2
#define RP2040_PIN_SPI0_MOSI   3
#define RP2040_PIN_SPI0_MISO   4
#define RP2040_PIN_SPI0_CS     5

/* ── SPI1 — Secondary / auxiliary ────────────────────────────────────── */
#define RP2040_PIN_SPI1_SCK    10
#define RP2040_PIN_SPI1_MOSI   11
#define RP2040_PIN_SPI1_MISO   12
#define RP2040_PIN_SPI1_CS     13

/* ── I2C0 ────────────────────────────────────────────────────────────── */
#define RP2040_PIN_I2C0_SDA    0
#define RP2040_PIN_I2C0_SCL    1

/* ── I2C1 ────────────────────────────────────────────────────────────── */
#define RP2040_PIN_I2C1_SDA    14
#define RP2040_PIN_I2C1_SCL    15

/* ── UART0 — CLI / Debug ─────────────────────────────────────────────── */
#define RP2040_PIN_UART0_TX    16
#define RP2040_PIN_UART0_RX    17

/* ── UART1 — Target passthrough ──────────────────────────────────────── */
#define RP2040_PIN_UART1_TX    8
#define RP2040_PIN_UART1_RX    9

/* ── JTAG (PIO-driven) ──────────────────────────────────────────────── */
#define RP2040_PIN_JTAG_TCK    18
#define RP2040_PIN_JTAG_TMS    19
#define RP2040_PIN_JTAG_TDI    20
#define RP2040_PIN_JTAG_TDO    21
#define RP2040_PIN_JTAG_TRST   22

/* ── SWD (PIO-driven) ───────────────────────────────────────────────── */
#define RP2040_PIN_SWD_SWCLK   18   /* Shared with JTAG TCK */
#define RP2040_PIN_SWD_SWDIO   19   /* Shared with JTAG TMS */
#define RP2040_PIN_SWD_RESET   22   /* Shared with JTAG TRST */

/* ── AVR ISP ─────────────────────────────────────────────────────────── */
#define RP2040_PIN_AVR_RST     6
#define RP2040_PIN_AVR_SCK     RP2040_PIN_SPI0_SCK
#define RP2040_PIN_AVR_MOSI    RP2040_PIN_SPI0_MOSI
#define RP2040_PIN_AVR_MISO    RP2040_PIN_SPI0_MISO

/* ── 1-Wire ──────────────────────────────────────────────────────────── */
#define RP2040_PIN_ONEWIRE     7

/* ── Level shifter / target power control ────────────────────────────── */
#define RP2040_PIN_VCC_EN      26   /* ADC0 capable — can also sense VCC */
#define RP2040_PIN_VCC_SEL     27   /* 3.3V / 5V select */

/* ── Analog ──────────────────────────────────────────────────────────── */
#define RP2040_PIN_VSENSE      28   /* ADC2 — target voltage sense */

#endif /* UNIPROGER_RP2040_PINS_H */
