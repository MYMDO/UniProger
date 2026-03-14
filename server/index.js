/**
 * UNIPROG SaaS Server
 * WebSocket + REST API for chip database, protocol handling, and algorithm execution.
 */

import express from 'express';
import { createServer } from 'http';
import { WebSocketServer } from 'ws';
import cors from 'cors';
import { readFileSync } from 'fs';
import { fileURLToPath } from 'url';
import { dirname, join } from 'path';

const __dirname = dirname(fileURLToPath(import.meta.url));
const PORT = process.env.PORT || 3001;

// ── Load chip database ─────────────────────────────────────────────────────
const chipDb = JSON.parse(readFileSync(join(__dirname, 'chip-db.json'), 'utf8'));

// ── Express app ─────────────────────────────────────────────────────────────
const app = express();
app.use(cors());
app.use(express.json());

// REST endpoints
app.get('/api/chips', (req, res) => {
  const { q, proto, type } = req.query;
  let results = chipDb;
  if (q) {
    const query = q.toLowerCase();
    results = results.filter(c =>
      c.name.toLowerCase().includes(query) ||
      c.mfr.toLowerCase().includes(query) ||
      c.type.toLowerCase().includes(query)
    );
  }
  if (proto && proto !== 'ALL') results = results.filter(c => c.proto === proto);
  if (type && type !== 'ALL') results = results.filter(c => c.type.includes(type));
  res.json({ count: results.length, chips: results });
});

app.get('/api/chips/:id', (req, res) => {
  const chip = chipDb.find(c => c.id === parseInt(req.params.id));
  if (!chip) return res.status(404).json({ error: 'Chip not found' });
  res.json(chip);
});

app.get('/api/status', (_req, res) => {
  res.json({
    version: '2.4.0',
    chipCount: chipDb.length,
    protocols: ['SPI', 'I2C', 'JTAG', 'SWD', 'UART', '1-Wire', 'PARALLEL', 'SWI'],
    uptime: process.uptime(),
  });
});

// ── HTTP + WebSocket server ─────────────────────────────────────────────────
const server = createServer(app);
const wss = new WebSocketServer({ server, path: '/ws' });

wss.on('connection', (ws) => {
  console.log('[WS] Client connected');

  ws.on('message', (data) => {
    try {
      const msg = JSON.parse(data);
      handleMessage(ws, msg);
    } catch (e) {
      ws.send(JSON.stringify({ type: 'error', message: 'Invalid JSON' }));
    }
  });

  ws.on('close', () => {
    console.log('[WS] Client disconnected');
  });

  // Send initial handshake
  ws.send(JSON.stringify({
    type: 'hello',
    server: 'uniprog-saas',
    version: '2.4.0',
    chipCount: chipDb.length,
    timestamp: Date.now(),
  }));
});

// ── WebSocket message handler ───────────────────────────────────────────────
function handleMessage(ws, msg) {
  const { type, reqId } = msg;

  switch (type) {
    case 'ping':
      ws.send(JSON.stringify({ type: 'pong', reqId, timestamp: Date.now() }));
      break;

    case 'chip.search':
      const results = chipDb.filter(c => {
        const q = (msg.query || '').toLowerCase();
        return c.name.toLowerCase().includes(q) || c.mfr.toLowerCase().includes(q);
      });
      ws.send(JSON.stringify({ type: 'chip.results', reqId, count: results.length, chips: results }));
      break;

    case 'chip.get':
      const chip = chipDb.find(c => c.id === msg.chipId);
      ws.send(JSON.stringify({ type: 'chip.data', reqId, chip }));
      break;

    case 'operation.start': {
      // Simulate operation progress
      const { chipId, op, startAddr, length } = msg;
      const target = chipDb.find(c => c.id === chipId);
      ws.send(JSON.stringify({ type: 'operation.ack', reqId, op, chip: target?.name }));

      let progress = 0;
      const interval = setInterval(() => {
        progress += Math.random() * 5 + 2;
        if (progress >= 100) {
          progress = 100;
          clearInterval(interval);
          ws.send(JSON.stringify({
            type: 'operation.complete', reqId, op,
            crc32: '0xA3F4C21B',
            elapsed: (Math.random() * 10 + 5).toFixed(1),
          }));
        } else {
          ws.send(JSON.stringify({
            type: 'operation.progress', reqId, op,
            progress: Math.min(progress, 100),
            speed: Math.floor(Math.random() * 200 + 800),
          }));
        }
      }, 200);
      break;
    }

    case 'protocol.analyze':
      // Return mock protocol analysis
      ws.send(JSON.stringify({
        type: 'protocol.transactions', reqId,
        transactions: [
          { id: 1, time: '00:00.001', dir: 'TX', proto: 'SPI', cmd: 'JEDEC_ID', data: '9F', resp: 'EF 40 18' },
          { id: 2, time: '00:00.003', dir: 'TX', proto: 'SPI', cmd: 'READ_STATUS', data: '05', resp: '00' },
        ],
      }));
      break;

    default:
      ws.send(JSON.stringify({ type: 'error', reqId, message: `Unknown type: ${type}` }));
  }
}

// ── Start server ────────────────────────────────────────────────────────────
server.listen(PORT, () => {
  console.log(`\n  ⊕ UNIPROG Server v2.4.0`);
  console.log(`  ├── REST API:    http://localhost:${PORT}/api`);
  console.log(`  ├── WebSocket:   ws://localhost:${PORT}/ws`);
  console.log(`  ├── Chip DB:     ${chipDb.length} entries`);
  console.log(`  └── Status:      http://localhost:${PORT}/api/status\n`);
});
