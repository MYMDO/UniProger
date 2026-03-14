/**
 * I2C EEPROM handler — server-side logic for 24Cxx series.
 * Generates I2C protocol commands relayed to RP2040.
 */

export function generateReadSequence(deviceAddr, startAddr, length, pageSize = 64, addrSize = 2) {
  const commands = [];
  let addr = startAddr;
  let remaining = length;

  while (remaining > 0) {
    const chunk = Math.min(remaining, pageSize);
    const addrBytes = addrSize === 2
      ? [(addr >> 8) & 0xFF, addr & 0xFF]
      : [addr & 0xFF];

    commands.push({
      type: 'i2c_write_read',
      devAddr: deviceAddr,
      writeData: addrBytes,
      readLen: chunk,
      desc: `Read ${chunk}B @ 0x${addr.toString(16).padStart(4, '0')}`,
    });
    addr += chunk;
    remaining -= chunk;
  }
  return commands;
}

export function generateWriteSequence(deviceAddr, startAddr, data, pageSize = 64, addrSize = 2) {
  const commands = [];
  let offset = 0;

  while (offset < data.length) {
    const pageOffset = (startAddr + offset) % pageSize;
    const chunk = Math.min(pageSize - pageOffset, data.length - offset);
    const addr = startAddr + offset;
    const addrBytes = addrSize === 2
      ? [(addr >> 8) & 0xFF, addr & 0xFF]
      : [addr & 0xFF];

    commands.push({
      type: 'i2c_write',
      devAddr: deviceAddr,
      data: [...addrBytes, ...data.slice(offset, offset + chunk)],
      desc: `Write ${chunk}B @ 0x${addr.toString(16).padStart(4, '0')}`,
    });
    commands.push({
      type: 'i2c_poll',
      devAddr: deviceAddr,
      timeout: 10,
      desc: 'ACK polling (write complete)',
    });

    offset += chunk;
  }
  return commands;
}

export default { generateReadSequence, generateWriteSequence };
