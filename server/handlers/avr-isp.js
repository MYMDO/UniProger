/**
 * AVR ISP handler — server-side logic for ATmega/ATtiny programming via SPI-based ISP.
 */

export const AVR_CMD = {
  PROGRAMMING_ENABLE:  [0xAC, 0x53, 0x00, 0x00],
  CHIP_ERASE:          [0xAC, 0x80, 0x00, 0x00],
  READ_SIGNATURE_0:    [0x30, 0x00, 0x00, 0x00],
  READ_SIGNATURE_1:    [0x30, 0x00, 0x01, 0x00],
  READ_SIGNATURE_2:    [0x30, 0x00, 0x02, 0x00],
  READ_FUSE_LOW:       [0x50, 0x00, 0x00, 0x00],
  READ_FUSE_HIGH:      [0x58, 0x08, 0x00, 0x00],
  READ_FUSE_EXT:       [0x50, 0x08, 0x00, 0x00],
};

export function generateProgramSequence(flashData, pageSize = 128) {
  const commands = [];
  commands.push({ type: 'spi_transfer', data: AVR_CMD.PROGRAMMING_ENABLE, desc: 'Enter ISP mode' });
  commands.push({ type: 'spi_transfer', data: AVR_CMD.READ_SIGNATURE_0, desc: 'Read signature byte 0' });
  commands.push({ type: 'spi_transfer', data: AVR_CMD.READ_SIGNATURE_1, desc: 'Read signature byte 1' });
  commands.push({ type: 'spi_transfer', data: AVR_CMD.READ_SIGNATURE_2, desc: 'Read signature byte 2' });
  commands.push({ type: 'spi_transfer', data: AVR_CMD.CHIP_ERASE, desc: 'Chip erase' });
  commands.push({ type: 'delay', ms: 20, desc: 'Wait for erase' });

  // Program flash pages
  for (let page = 0; page < flashData.length; page += pageSize) {
    const pageData = flashData.slice(page, page + pageSize);
    for (let i = 0; i < pageData.length; i += 2) {
      const wordAddr = (page + i) / 2;
      const low = pageData[i] || 0xFF;
      const high = pageData[i + 1] || 0xFF;
      commands.push({
        type: 'spi_transfer',
        data: [0x40, (wordAddr >> 8) & 0xFF, wordAddr & 0xFF, low],
        desc: `Load low byte @ 0x${wordAddr.toString(16)}`,
      });
      commands.push({
        type: 'spi_transfer',
        data: [0x48, (wordAddr >> 8) & 0xFF, wordAddr & 0xFF, high],
        desc: `Load high byte @ 0x${wordAddr.toString(16)}`,
      });
    }
    const pageAddr = page / 2;
    commands.push({
      type: 'spi_transfer',
      data: [0x4C, (pageAddr >> 8) & 0xFF, pageAddr & 0xFF, 0x00],
      desc: `Write page @ 0x${pageAddr.toString(16)}`,
    });
    commands.push({ type: 'delay', ms: 5, desc: 'Page write delay' });
  }

  return commands;
}

export default { AVR_CMD, generateProgramSequence };
