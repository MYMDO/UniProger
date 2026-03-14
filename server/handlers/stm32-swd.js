/**
 * STM32 SWD handler — server-side logic for ARM Cortex-M programming via SWD.
 */

// SWD DP registers
export const DP = {
  DPIDR:      0x00,
  CTRL_STAT:  0x04,
  SELECT:     0x08,
  RDBUFF:     0x0C,
};

// AHB-AP registers
export const AP = {
  CSW:  0x00,
  TAR:  0x04,
  DRW:  0x0C,
};

// STM32F1xx Flash registers
export const FLASH = {
  ACR:    0x40022000,
  KEYR:   0x40022004,
  OPTKEYR:0x40022008,
  SR:     0x4002200C,
  CR:     0x40022010,
  AR:     0x40022014,
};

export function generateConnectSequence() {
  return [
    { type: 'swd_line_reset', desc: 'SWD line reset (50+ clocks high)' },
    { type: 'swd_jtag_to_swd', desc: 'JTAG-to-SWD switch sequence' },
    { type: 'swd_line_reset', desc: 'SWD line reset again' },
    { type: 'swd_read_dp', addr: DP.DPIDR, desc: 'Read DPIDR' },
    { type: 'swd_write_dp', addr: DP.CTRL_STAT, data: 0x50000000, desc: 'Power up debug' },
    { type: 'swd_write_dp', addr: DP.SELECT, data: 0x00, desc: 'Select AP 0' },
    { type: 'swd_write_ap', addr: AP.CSW, data: 0x23000012, desc: 'CSW: 32-bit, auto-increment' },
  ];
}

export function generateReadSequence(startAddr, length) {
  const commands = generateConnectSequence();
  const wordCount = Math.ceil(length / 4);

  commands.push({ type: 'swd_write_ap', addr: AP.TAR, data: startAddr, desc: `Set TAR to 0x${startAddr.toString(16)}` });

  for (let i = 0; i < wordCount; i++) {
    commands.push({
      type: 'swd_read_ap',
      addr: AP.DRW,
      desc: `Read word ${i}/${wordCount}`,
    });
  }
  return commands;
}

export function generateFlashUnlock() {
  return [
    { type: 'swd_write_ap_mem', addr: FLASH.KEYR, data: 0x45670123, desc: 'Flash unlock key 1' },
    { type: 'swd_write_ap_mem', addr: FLASH.KEYR, data: 0xCDEF89AB, desc: 'Flash unlock key 2' },
  ];
}

export function generateEraseSequence(startAddr, pageCount = 1, pageSize = 1024) {
  const commands = [...generateConnectSequence(), ...generateFlashUnlock()];

  for (let i = 0; i < pageCount; i++) {
    const addr = startAddr + i * pageSize;
    commands.push({ type: 'swd_write_ap_mem', addr: FLASH.CR, data: 0x02, desc: 'Set PER bit' });
    commands.push({ type: 'swd_write_ap_mem', addr: FLASH.AR, data: addr, desc: `Set erase addr 0x${addr.toString(16)}` });
    commands.push({ type: 'swd_write_ap_mem', addr: FLASH.CR, data: 0x42, desc: 'Set STRT bit' });
    commands.push({ type: 'swd_poll_flash_bsy', desc: 'Wait BSY clear' });
  }

  return commands;
}

export default { DP, AP, FLASH, generateConnectSequence, generateReadSequence, generateEraseSequence };
