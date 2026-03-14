/**
 * WebSocket client for UNIPROG server communication.
 * Handles chip database queries, algorithm execution, and real-time updates.
 */

export class UniprogWebSocket {
  constructor(url = 'ws://localhost:3001/ws') {
    this.url = url;
    this.ws = null;
    this.handlers = new Map();
    this.pending = new Map();
    this._reqId = 0;
    this.onStatusChange = null;
  }

  connect() {
    return new Promise((resolve, reject) => {
      this.ws = new WebSocket(this.url);
      this.ws.onopen = () => {
        if (this.onStatusChange) this.onStatusChange('connected');
        resolve();
      };
      this.ws.onclose = () => {
        if (this.onStatusChange) this.onStatusChange('disconnected');
      };
      this.ws.onerror = (e) => reject(e);
      this.ws.onmessage = (event) => this._handleMessage(JSON.parse(event.data));
    });
  }

  _handleMessage(msg) {
    if (msg.reqId && this.pending.has(msg.reqId)) {
      const { resolve } = this.pending.get(msg.reqId);
      this.pending.delete(msg.reqId);
      resolve(msg);
    }
    if (msg.type && this.handlers.has(msg.type)) {
      this.handlers.get(msg.type)(msg);
    }
  }

  request(type, data = {}) {
    return new Promise((resolve, reject) => {
      const reqId = ++this._reqId;
      this.pending.set(reqId, { resolve, reject });
      this.ws.send(JSON.stringify({ type, reqId, ...data }));
      setTimeout(() => {
        if (this.pending.has(reqId)) {
          this.pending.delete(reqId);
          reject(new Error('Request timeout'));
        }
      }, 10000);
    });
  }

  on(type, handler) {
    this.handlers.set(type, handler);
  }

  send(type, data = {}) {
    this.ws.send(JSON.stringify({ type, ...data }));
  }

  disconnect() {
    if (this.ws) { this.ws.close(); this.ws = null; }
  }

  get isConnected() {
    return this.ws?.readyState === WebSocket.OPEN;
  }
}

export default UniprogWebSocket;
