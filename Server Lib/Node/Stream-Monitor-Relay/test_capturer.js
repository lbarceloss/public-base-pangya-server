/*
 * Capturador SIMULADO - so pra testar o relay sem a DLL.
 * Conecta, faz REGISTER e manda um "frame" (JPEG 1x1 base64) a cada 500ms.
 * Tambem loga os comandos que o relay repassa (RATE / TOGGLE / CLOSE).
 *
 * uso:  node test_capturer.js [id] [nickname]
 */
'use strict';
const WebSocket = require('ws');

const URL = process.env.WEBSHOT_URL || 'ws://127.0.0.1:4573';
const id = process.argv[2] || '4484';
const nickname = process.argv[3] || 'ADM_SuperSS';

// JPEG 1x1 valido (base64). Serve pra provar que o frame chega e renderiza.
const JPEG_1x1 = '/9j/2wBDAAMCAgICAgMCAgIDAwMDBAYEBAQEBAgGBgUGCQgKCgkICQkKDA8MCgsOCwkJDRENDg8QEBEQCgwSExIQEw8QEBD/wAALCAABAAEBAREA/8QAFAABAAAAAAAAAAAAAAAAAAAAAP/EABQQAQAAAAAAAAAAAAAAAAAAAAD/2gAIAQEAAD8AfwD/2Q==';

const ws = new WebSocket(URL);

ws.on('open', () => {
    console.log(`[cap ${id}] conectado em ${URL}, registrando...`);
    ws.send(JSON.stringify({ type: 'REGISTER', id, nickname }));

    let n = 0;
    const timer = setInterval(() => {
        n++;
        ws.send(JSON.stringify({ type: 'FRAME', id, nickname, frame: JPEG_1x1 }));
        if (n % 10 === 0) console.log(`[cap ${id}] enviou ${n} frames`);
    }, 500);

    ws.on('close', () => { clearInterval(timer); console.log(`[cap ${id}] desconectado`); });
});

ws.on('message', (data) => {
    console.log(`[cap ${id}] comando do relay: ${data}`);
});

ws.on('error', (e) => console.error(`[cap ${id}] erro:`, e.message));
