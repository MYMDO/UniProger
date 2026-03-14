export const PROTOCOLS = {
  SPI:      { color: "#00ffa3", icon: "◈", pins: ["MOSI","MISO","CLK","CS"] },
  I2C:      { color: "#00d4ff", icon: "◉", pins: ["SDA","SCL"] },
  JTAG:     { color: "#ffb800", icon: "◆", pins: ["TDI","TDO","TCK","TMS","TRST"] },
  SWD:      { color: "#b06aff", icon: "◇", pins: ["SWDIO","SWCLK"] },
  UART:     { color: "#ff6644", icon: "◈", pins: ["TX","RX"] },
  "1-Wire": { color: "#ff4466", icon: "○", pins: ["DQ"] },
  PARALLEL: { color: "#88ff00", icon: "▣", pins: ["D0-D7","ALE","WE","RE","CE"] },
  SWI:      { color: "#00ffdd", icon: "◐", pins: ["SWI"] },
};

export const LOG_COLORS = {
  INFO: "var(--cyan)",
  WARN: "var(--amber)",
  ERR:  "var(--red)",
  OK:   "var(--green)",
  TX:   "var(--purple)",
  RX:   "var(--text-2)",
};
