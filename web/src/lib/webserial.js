/**
 * Web Serial API wrapper for UNIPROG bridge communication.
 * Manages connection to RP2040 via USB CDC.
 */

export class WebSerialBridge {
  constructor() {
    this.port = null;
    this.reader = null;
    this.writer = null;
    this.readBuffer = [];
    this.onData = null;
    this.onDisconnect = null;
    this._reading = false;
  }

  static isSupported() {
    return 'serial' in navigator;
  }

  async connect(baudRate = 115200) {
    if (!WebSerialBridge.isSupported()) {
      throw new Error('Web Serial API not supported. Use Chrome or Edge.');
    }
    this.port = await navigator.serial.requestPort({
      filters: [{ usbVendorId: 0x2E8A }] // Raspberry Pi RP2040
    });
    await this.port.open({ baudRate });

    const textDecoder = new TextDecoderStream();
    const readableStreamClosed = this.port.readable.pipeTo(textDecoder.writable);
    this.reader = textDecoder.readable.getReader();

    const textEncoder = new TextEncoderStream();
    const writableStreamClosed = textEncoder.readable.pipeTo(this.port.writable);
    this.writer = textEncoder.writable.getWriter();

    this._reading = true;
    this._readLoop();

    return { port: this.port.getInfo() };
  }

  async _readLoop() {
    while (this._reading) {
      try {
        const { value, done } = await this.reader.read();
        if (done) break;
        if (value && this.onData) this.onData(value);
      } catch (err) {
        if (this._reading) {
          console.error('Serial read error:', err);
          this.disconnect();
        }
        break;
      }
    }
  }

  async send(data) {
    if (!this.writer) throw new Error('Not connected');
    await this.writer.write(data);
  }

  async sendBinary(uint8array) {
    if (!this.port?.writable) throw new Error('Not connected');
    const writer = this.port.writable.getWriter();
    await writer.write(uint8array);
    writer.releaseLock();
  }

  async disconnect() {
    this._reading = false;
    try {
      if (this.reader) { await this.reader.cancel(); this.reader = null; }
      if (this.writer) { await this.writer.close(); this.writer = null; }
      if (this.port) { await this.port.close(); this.port = null; }
    } catch (e) { /* ignore cleanup errors */ }
    if (this.onDisconnect) this.onDisconnect();
  }

  get isConnected() {
    return this.port !== null;
  }
}

export default WebSerialBridge;
