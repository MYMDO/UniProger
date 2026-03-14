/**
 * UNIPROG Binary Protocol — Frame encoder/decoder
 *
 * Frame format:
 *   [SYNC:2][LEN:2][SEQ:1][CMD:1][PAYLOAD:N][CRC16:2]
 *
 * SYNC  = 0x55 0xAA
 * LEN   = total payload length (little-endian)
 * SEQ   = sequence number (0-255, wrapping)
 * CMD   = command byte
 * CRC16 = CRC-CCITT over [SEQ+CMD+PAYLOAD]
 */

const SYNC = [0x55, 0xAA];

/* ── Commands ────────────────────────────────────────────────────────────── */
export const CMD = {
  // System
  PING:           0x01,
  PONG:           0x02,
  VERSION:        0x03,
  RESET:          0x0F,

  // SPI
  SPI_INIT:       0x10,
  SPI_TRANSFER:   0x11,
  SPI_CS:         0x12,
  SPI_JEDEC_ID:   0x13,

  // I2C
  I2C_INIT:       0x20,
  I2C_WRITE:      0x21,
  I2C_READ:       0x22,
  I2C_SCAN:       0x23,

  // GPIO
  GPIO_SET:       0x30,
  GPIO_GET:       0x31,
  GPIO_CONFIG:    0x32,

  // JTAG
  JTAG_INIT:      0x40,
  JTAG_SHIFT_IR:  0x41,
  JTAG_SHIFT_DR:  0x42,
  JTAG_IDCODE:    0x43,

  // SWD
  SWD_INIT:       0x50,
  SWD_READ:       0x51,
  SWD_WRITE:      0x52,

  // Response
  ACK:            0xA0,
  NAK:            0xA1,
  DATA:           0xA2,
  ERROR:          0xAF,
};

/* ── CRC-16 CCITT ────────────────────────────────────────────────────────── */
function crc16(data) {
  let crc = 0xFFFF;
  for (const byte of data) {
    crc ^= byte << 8;
    for (let i = 0; i < 8; i++) {
      crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
      crc &= 0xFFFF;
    }
  }
  return crc;
}

/* ── Encoder ─────────────────────────────────────────────────────────────── */
let _seq = 0;

export function encodeFrame(cmd, payload = []) {
  const seq = _seq++ & 0xFF;
  const body = [seq, cmd, ...payload];
  const len = payload.length;
  const crc = crc16(body);
  return new Uint8Array([
    ...SYNC,
    len & 0xFF, (len >> 8) & 0xFF,
    ...body,
    crc & 0xFF, (crc >> 8) & 0xFF,
  ]);
}

/* ── Decoder ─────────────────────────────────────────────────────────────── */
export function decodeFrame(buffer) {
  if (buffer.length < 8) return null; // minimum frame size
  if (buffer[0] !== SYNC[0] || buffer[1] !== SYNC[1]) return null;

  const len = buffer[2] | (buffer[3] << 8);
  const totalLen = 4 + 1 + 1 + len + 2; // sync + len + seq + cmd + payload + crc
  if (buffer.length < totalLen) return null;

  const seq = buffer[4];
  const cmd = buffer[5];
  const payload = buffer.slice(6, 6 + len);
  const receivedCrc = buffer[6 + len] | (buffer[7 + len] << 8);

  const body = buffer.slice(4, 6 + len);
  const calculatedCrc = crc16(body);

  if (receivedCrc !== calculatedCrc) {
    return { error: 'CRC mismatch', seq, cmd, consumed: totalLen };
  }

  return { seq, cmd, payload: Array.from(payload), consumed: totalLen };
}

export default { CMD, encodeFrame, decodeFrame };
