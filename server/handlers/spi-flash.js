/**
 * SPI Flash handler — server-side logic for W25Qxx, GD25Qxx, MX25L, etc.
 * Generates protocol commands that the browser relays to RP2040 via Web Serial.
 */

// SPI Flash command set
export const SPI_CMD = {
  WRITE_ENABLE:    0x06,
  WRITE_DISABLE:   0x04,
  READ_STATUS_1:   0x05,
  READ_STATUS_2:   0x35,
  READ_DATA:       0x03,
  FAST_READ:       0x0B,
  PAGE_PROGRAM:    0x02,
  SECTOR_ERASE_4K: 0x20,
  BLOCK_ERASE_32K: 0x52,
  BLOCK_ERASE_64K: 0xD8,
  CHIP_ERASE:      0xC7,
  JEDEC_ID:        0x9F,
  READ_UNIQUE_ID:  0x4B,
  POWER_DOWN:      0xB9,
  RELEASE_PD:      0xAB,
};

export function generateReadSequence(startAddr, length, pageSize = 256) {
  const commands = [];
  let addr = startAddr;
  let remaining = length;

  while (remaining > 0) {
    const chunk = Math.min(remaining, pageSize);
    commands.push({
      cmd: SPI_CMD.READ_DATA,
      data: [(addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF],
      readLen: chunk,
      desc: `Read ${chunk}B @ 0x${addr.toString(16).padStart(6, '0')}`,
    });
    addr += chunk;
    remaining -= chunk;
  }
  return commands;
}

export function generateWriteSequence(startAddr, data, pageSize = 256) {
  const commands = [];
  let offset = 0;

  while (offset < data.length) {
    const pageOffset = (startAddr + offset) % pageSize;
    const chunk = Math.min(pageSize - pageOffset, data.length - offset);
    const addr = startAddr + offset;

    commands.push({ cmd: SPI_CMD.WRITE_ENABLE, data: [], desc: 'Write Enable' });
    commands.push({
      cmd: SPI_CMD.PAGE_PROGRAM,
      data: [(addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF, ...data.slice(offset, offset + chunk)],
      desc: `Page Program ${chunk}B @ 0x${addr.toString(16).padStart(6, '0')}`,
    });
    commands.push({ cmd: SPI_CMD.READ_STATUS_1, pollBit: 0, desc: 'Wait WIP clear' });

    offset += chunk;
  }
  return commands;
}

export function generateEraseSequence(startAddr, length) {
  const commands = [];
  if (length === 0) {
    commands.push({ cmd: SPI_CMD.WRITE_ENABLE, data: [], desc: 'Write Enable' });
    commands.push({ cmd: SPI_CMD.CHIP_ERASE, data: [], desc: 'Chip Erase' });
    commands.push({ cmd: SPI_CMD.READ_STATUS_1, pollBit: 0, timeout: 60000, desc: 'Wait erase complete' });
  } else {
    let addr = startAddr;
    let remaining = length;
    while (remaining > 0) {
      commands.push({ cmd: SPI_CMD.WRITE_ENABLE, data: [], desc: 'Write Enable' });
      const eraseCmd = remaining >= 65536 ? SPI_CMD.BLOCK_ERASE_64K : remaining >= 32768 ? SPI_CMD.BLOCK_ERASE_32K : SPI_CMD.SECTOR_ERASE_4K;
      const eraseSize = remaining >= 65536 ? 65536 : remaining >= 32768 ? 32768 : 4096;
      commands.push({
        cmd: eraseCmd,
        data: [(addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF],
        desc: `Erase ${eraseSize/1024}KB @ 0x${addr.toString(16).padStart(6, '0')}`,
      });
      commands.push({ cmd: SPI_CMD.READ_STATUS_1, pollBit: 0, desc: 'Wait erase complete' });
      addr += eraseSize;
      remaining -= eraseSize;
    }
  }
  return commands;
}

export default { SPI_CMD, generateReadSequence, generateWriteSequence, generateEraseSequence };
